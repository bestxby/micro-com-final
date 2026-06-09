#include "esp8266.h"
#include "usart1.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Internal Buffers */
static char esp_rx_buf[2048];

/**
  * @brief  Helper to wait for an expected reply from ESP8266
  * @param  expected: The substring to look for (e.g. "OK")
  * @param  timeout_ms: Timeout period in milliseconds
  * @retval 1 = Success, 0 = Timeout
  */
static uint8_t ESP8266_WaitReply(const char *expected, uint32_t timeout_ms)
{
    uint16_t idx = 0;
    memset(esp_rx_buf, 0, sizeof(esp_rx_buf));
    
    uint32_t start_ticks = timeout_ms;
    while (start_ticks > 0) {
        char c;
        while (USART1_ReadChar(&c)) {
            if (idx < sizeof(esp_rx_buf) - 1) {
                esp_rx_buf[idx++] = c;
                esp_rx_buf[idx] = '\0';
                if (strstr(esp_rx_buf, expected) != NULL) {
                    return 1; /* Found expected string */
                }
            } else {
                /* Flush buffer on overflow to avoid crash */
                idx = 0;
                memset(esp_rx_buf, 0, sizeof(esp_rx_buf));
            }
        }
        /* Delay ~1ms */
        for (volatile int d = 0; d < 7200; d++);
        start_ticks--;
    }
    return 0; /* Timeout */
}

/**
  * @brief  Helper to send AT command and check expected reply
  */
static uint8_t ESP8266_SendCmd(const char *cmd, const char *reply, uint32_t timeout_ms)
{
    USART1_ClearBuffer();
    USART1_SendString(cmd);
    return ESP8266_WaitReply(reply, timeout_ms);
}

/**
  * @brief  Initialize ESP8266 at 115200 baud
  * @retval 0 = Success, 1 = Error
  */
uint8_t ESP8266_Init(void)
{
    USART1_Init(115200);
    
    /* Disconnect current TCP connections and test AT response */
    ESP8266_SendCmd("AT+CIPCLOSE\r\n", "CLOSE", 500);
    if (!ESP8266_SendCmd("AT\r\n", "OK", 1000)) {
        return 1; /* Hardware offline */
    }
    
    return 0;
}

volatile uint8_t wifi_conn_error = 0;

/**
  * @brief  Connect ESP8266 to WiFi Access Point
  * @param  ssid: Hotspot name
  * @param  password: Hotspot password
  * @retval 0 = Connected, 1 = Error
  */
uint8_t ESP8266_ConnectWiFi(const char *ssid, const char *password)
{
    char cmd[128];
    wifi_conn_error = 0;
    
    /* 1. Set station mode */
    if (!ESP8266_SendCmd("AT+CWMODE=1\r\n", "OK", 1000)) {
        /* Accept "no change" as OK */
        if (strstr(esp_rx_buf, "no change") == NULL) {
            wifi_conn_error = 10;
            return 1;
        }
    }
    
    /* 2. Connect to AP */
    sprintf(cmd, "AT+CWJAP=\"%s\",\"%s\"\r\n", ssid, password);
    if (!ESP8266_SendCmd(cmd, "OK", 12000)) {
        char *p = strstr(esp_rx_buf, "+CWJAP:");
        if (p) {
            wifi_conn_error = (uint8_t)atoi(p + 7);
        } else {
            wifi_conn_error = 9;
        }
        return 1;
    }
    
    /* 3. Configure single connection mode (required for simple HTTP requests) */
    if (!ESP8266_SendCmd("AT+CIPMUX=0\r\n", "OK", 1000)) {
        wifi_conn_error = 20;
        return 1;
    }
    
    return 0;
}

/**
  * @brief  Enable and configure NTP server clock synchronization
  * @retval 0 = Success, 1 = Error
  */
uint8_t ESP8266_SyncNTP(void)
{
    /* Enable NTP, GMT+8 timezone, CN servers */
    if (!ESP8266_SendCmd("AT+CIPSNTPCFG=1,8,\"cn.ntp.org.cn\",\"ntp.sjtu.edu.cn\",\"us.pool.ntp.org\"\r\n", "OK", 2000)) {
        return 1;
    }
    return 0;
}

/**
  * @brief  Query and parse NTP time
  * @param  datetime_str: Destination buffer (must be at least 20 bytes)
  * @retval 0 = Success, 1 = Error
  */
uint8_t ESP8266_GetNTPTime(char *datetime_str)
{
    if (!ESP8266_SendCmd("AT+CIPSNTPTIME?\r\n", "OK", 2000)) {
        return 1;
    }
    
    char *p = strstr(esp_rx_buf, "+CIPSNTPTIME:");
    if (p == NULL) {
        return 1;
    }
    
    char wdy[4], mon_str[4], day[3], time_part[9], year[5];
    /* Parse "+CIPSNTPTIME:Fri Jun 05 20:15:30 2026" */
    if (sscanf(p + 13, "%3s %3s %2s %8s %4s", wdy, mon_str, day, time_part, year) == 5) {
        const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
        int m = 1;
        for (int i = 0; i < 12; i++) {
            if (strcmp(mon_str, months[i]) == 0) {
                m = i + 1;
                break;
            }
        }
        sprintf(datetime_str, "%s-%02d-%s %s", year, m, day, time_part);
        return 0;
    }
    
    return 1;
}

extern volatile uint16_t net_year;
extern volatile uint8_t net_month;
extern volatile uint8_t net_day;
extern volatile uint8_t net_hour;
extern volatile uint8_t net_minute;
extern volatile uint8_t net_second;
extern char current_net_time[24];

char wifi_debug_msg[64] = "Init OK";

/**
  * @brief  Helper to format and set debug message
  */
static void ESP8266_SetDebugMsg(const char *prefix, const char *raw_buf)
{
    char temp[128];
    snprintf(temp, sizeof(temp), "%s:%s", prefix, raw_buf);
    for (int i = 0; temp[i] != '\0'; i++) {
        if (temp[i] == '\r' || temp[i] == '\n') {
            temp[i] = ' ';
        }
    }
    strncpy(wifi_debug_msg, temp, 63);
    wifi_debug_msg[63] = '\0';
}

/**
  * @brief  Parse GMT Date from HTTP response header and convert to GMT+8
  */
static uint8_t ParseHTTPDate(const char *http_buf, int *y, int *m, int *d, int *hh, int *mm, int *ss)
{
    const char *p = strstr(http_buf, "Date: ");
    if (p == NULL) return 1;
    
    char wdy[4], mon_str[4], time_part[9];
    int day_val, year_val;
    if (sscanf(p + 6, "%3s, %d %3s %d %8s", wdy, &day_val, mon_str, &year_val, time_part) == 5) {
        const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
        int month_val = 1;
        for (int i = 0; i < 12; i++) {
            if (strcmp(mon_str, months[i]) == 0) {
                month_val = i + 1;
                break;
            }
        }
        
        int h, min, sec;
        if (sscanf(time_part, "%d:%d:%d", &h, &min, &sec) == 3) {
            h += 8; // GMT+8
            if (h >= 24) {
                h -= 24;
                day_val++;
                const uint8_t days_in_months[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
                uint8_t dim = days_in_months[month_val - 1];
                if (month_val == 2) {
                    uint8_t is_leap = (year_val % 4 == 0 && (year_val % 100 != 0 || year_val % 400 == 0));
                    if (is_leap) dim = 29;
                }
                if (day_val > dim) {
                    day_val = 1;
                    month_val++;
                    if (month_val > 12) {
                        month_val = 1;
                        year_val++;
                    }
                }
            }
            *y = year_val;
            *m = month_val;
            *d = day_val;
            *hh = h;
            *mm = min;
            *ss = sec;
            return 0;
        }
    }
    return 1;
}

/**
  * @brief  Fetch current weather from Seniverse API (using free key)
  * @param  out_temp: Output pointer for temperature value
  * @param  weather_text: Output pointer for weather status text (e.g. "Sunny")
  * @retval 0 = Success, 1 = Error
  */
uint8_t ESP8266_GetWeather(float *out_temp, char *weather_text)
{
    /* 1. Open TCP connection to Seniverse weather server */
    if (!ESP8266_SendCmd("AT+CIPSTART=\"TCP\",\"api.seniverse.com\",80\r\n", "CONNECT", 3000)) {
        ESP8266_SetDebugMsg("TCP_ERR", esp_rx_buf);
        return 1;
    }
    
    /* 2. Format HTTP GET request (Guangzhou location, Celsius units) */
    char http_req[256];
    sprintf(http_req, "GET /v3/weather/now.json?key=SHzMnk5kmVJghlvvP&location=guangzhou&language=en&unit=c HTTP/1.1\r\nHost: api.seniverse.com\r\nConnection: close\r\n\r\n");
    
    char send_cmd[32];
    sprintf(send_cmd, "AT+CIPSEND=%d\r\n", (int)strlen(http_req));
    
    /* 3. Send command and data */
    if (!ESP8266_SendCmd(send_cmd, ">", 2000)) {
        ESP8266_SendCmd("AT+CIPCLOSE\r\n", "OK", 500);
        ESP8266_SetDebugMsg("SEND_ERR", esp_rx_buf);
        return 1;
    }
    
    if (!ESP8266_SendCmd(http_req, "+IPD", 4000)) {
        ESP8266_SetDebugMsg("IPD_ERR", esp_rx_buf);
        return 1;
    }
    
    /* Wait and read the remaining HTTP response payload to avoid PARSE_ERR */
    {
        uint16_t idx = strlen(esp_rx_buf);
        uint32_t idle_ticks = 500;
        while (idle_ticks > 0) {
            char c;
            uint8_t read_any = 0;
            while (USART1_ReadChar(&c)) {
                read_any = 1;
                if (idx < sizeof(esp_rx_buf) - 1) {
                    esp_rx_buf[idx++] = c;
                    esp_rx_buf[idx] = '\0';
                }
            }
            if (read_any) {
                idle_ticks = 200; // Reset idle timer when data arrives
            }
            if (strstr(esp_rx_buf, "CLOSED") != NULL) {
                break; // Connection closed by server, all data received
            }
            for (volatile int d = 0; d < 7200; d++); // Delay ~1ms
            idle_ticks--;
        }
    }
    
    /* 4. Parse response */
    char *text_pos = strstr(esp_rx_buf, "\"text\":\"");
    char *temp_pos = strstr(esp_rx_buf, "\"temperature\":\"");
    
    if (text_pos && temp_pos) {
        /* Extract Weather Text (e.g. "Sunny") */
        text_pos += 8;
        char *end_text = strchr(text_pos, '"');
        if (end_text) {
            int len = end_text - text_pos;
            if (len > 15) len = 15;
            strncpy(weather_text, text_pos, len);
            weather_text[len] = '\0';
        }
        
        /* Extract Temperature */
        temp_pos += 15;
        *out_temp = (float)atof(temp_pos);
        
        /* Fallback: parse current time from HTTP Date header */
        int y, m, d, hh, mm, ss;
        if (ParseHTTPDate(esp_rx_buf, &y, &m, &d, &hh, &mm, &ss) == 0) {
            net_year = y;
            net_month = m;
            net_day = d;
            net_hour = hh;
            net_minute = mm;
            net_second = ss;
            sprintf(current_net_time, "%04d-%02d-%02d %02d:%02d:%02d", y, m, d, hh, mm, ss);
            ESP8266_SetDebugMsg("OK", "Weather & Date Sync OK");
        } else {
            ESP8266_SetDebugMsg("OK", "Weather OK, Date Parse Err");
        }
        
        /* Close connection */
        ESP8266_SendCmd("AT+CIPCLOSE\r\n", "OK", 500);
        return 0;
    }
    
    ESP8266_SetDebugMsg("PARSE_ERR", esp_rx_buf);
    ESP8266_SendCmd("AT+CIPCLOSE\r\n", "OK", 500);
    return 1;
}

/**
  * @brief  Trigger a ServerChan WeChat notification alert
  * @param  sendkey: ServerChan Key (e.g. SCTxxxxxx)
  * @param  title: Message title (No spaces, use underscores instead)
  * @param  content: Message content (No spaces)
  * @retval 0 = Success, 1 = Error
  */
uint8_t ESP8266_SendWeChatAlert(const char *sendkey, const char *title, const char *content)
{
    /* 1. Open TCP connection to ServerChan */
    if (!ESP8266_SendCmd("AT+CIPSTART=\"TCP\",\"sc.ftqq.com\",80\r\n", "CONNECT", 3000)) {
        return 1;
    }
    
    /* 2. Format HTTP GET request
     * Fix: Changed sprintf -> snprintf with explicit buffer size to prevent
     * stack overflow when sendkey/title/content exceed the 384-byte buffer.
     * If the combined string would overflow, snprintf safely truncates it
     * and we close the connection rather than sending a corrupt frame. */
    char http_req[384];
    int req_len = snprintf(http_req, sizeof(http_req),
        "GET /%s.send?title=%s&desp=%s HTTP/1.1\r\nHost: sc.ftqq.com\r\nConnection: close\r\n\r\n",
        sendkey, title, content);
    
    /* Guard: if output was truncated (req_len >= sizeof), abort safely */
    if (req_len <= 0 || req_len >= (int)sizeof(http_req)) {
        ESP8266_SendCmd("AT+CIPCLOSE\r\n", "OK", 500);
        return 1;
    }
    
    char send_cmd[32];
    snprintf(send_cmd, sizeof(send_cmd), "AT+CIPSEND=%d\r\n", req_len);
    
    /* 3. Send command and data */
    if (!ESP8266_SendCmd(send_cmd, ">", 2000)) {
        ESP8266_SendCmd("AT+CIPCLOSE\r\n", "OK", 500);
        return 1;
    }
    
    if (!ESP8266_SendCmd(http_req, "SEND OK", 4000)) {
        return 1;
    }
    
    /* 4. Close and exit */
    ESP8266_SendCmd("AT+CIPCLOSE\r\n", "OK", 500);
    return 0;
}

/**
  * @brief  Check if ESP8266 is connected to an Access Point and has an IP
  * @retval 1 = Connected, 0 = Disconnected/Error
  */
uint8_t ESP8266_IsConnected(void)
{
    if (ESP8266_SendCmd("AT+CWJAP?\r\n", "+CWJAP:\"", 800)) {
        return 1;
    }
    return 0;
}

/**
  * @brief  Check if ESP8266 hardware responds to AT commands
  * @retval 1 = Online, 0 = Offline
  */
uint8_t ESP8266_IsHardwareOnline(void)
{
    return ESP8266_SendCmd("AT\r\n", "OK", 500);
}

