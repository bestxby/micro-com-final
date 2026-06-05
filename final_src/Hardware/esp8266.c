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

/**
  * @brief  Connect ESP8266 to WiFi Access Point
  * @param  ssid: Hotspot name
  * @param  password: Hotspot password
  * @retval 0 = Connected, 1 = Error
  */
uint8_t ESP8266_ConnectWiFi(const char *ssid, const char *password)
{
    char cmd[128];
    
    /* 1. Set station mode permanently */
    if (!ESP8266_SendCmd("AT+CWMODE_DEF=1\r\n", "OK", 1000)) {
        /* Accept "no change" as OK */
        if (strstr(esp_rx_buf, "no change") == NULL) {
            return 1;
        }
    }
    
    /* 2. Connect to AP (saved in flash for auto-reconnect) */
    sprintf(cmd, "AT+CWJAP_DEF=\"%s\",\"%s\"\r\n", ssid, password);
    if (!ESP8266_SendCmd(cmd, "OK", 10000)) {
        return 1;
    }
    
    /* 3. Configure single connection mode (required for simple HTTP requests) */
    if (!ESP8266_SendCmd("AT+CIPMUX=0\r\n", "OK", 1000)) {
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
        return 1;
    }
    
    /* 2. Format HTTP GET request (auto IP location, Celsius units) */
    char http_req[256];
    sprintf(http_req, "GET /v3/weather/now.json?key=S8P3F8pYv9fEsz4_k&location=ip&language=en&unit=c HTTP/1.1\r\nHost: api.seniverse.com\r\nConnection: close\r\n\r\n");
    
    char send_cmd[32];
    sprintf(send_cmd, "AT+CIPSEND=%d\r\n", (int)strlen(http_req));
    
    /* 3. Send command and data */
    if (!ESP8266_SendCmd(send_cmd, ">", 2000)) {
        ESP8266_SendCmd("AT+CIPCLOSE\r\n", "OK", 500);
        return 1;
    }
    
    if (!ESP8266_SendCmd(http_req, "+IPD", 4000)) {
        return 1;
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
        
        /* Close connection */
        ESP8266_SendCmd("AT+CIPCLOSE\r\n", "OK", 500);
        return 0;
    }
    
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
    
    /* 2. Format HTTP GET request */
    char http_req[384];
    sprintf(http_req, "GET /%s.send?title=%s&desp=%s HTTP/1.1\r\nHost: sc.ftqq.com\r\nConnection: close\r\n\r\n", sendkey, title, content);
    
    char send_cmd[32];
    sprintf(send_cmd, "AT+CIPSEND=%d\r\n", (int)strlen(http_req));
    
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
