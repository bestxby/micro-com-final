#ifndef __ESP8266_H
#define __ESP8266_H

#include "stm32f10x.h"

/* Public API */
uint8_t ESP8266_Init(void);
uint8_t ESP8266_ConnectWiFi(const char *ssid, const char *password);
uint8_t ESP8266_SyncNTP(void);
uint8_t ESP8266_GetNTPTime(char *datetime_str);
uint8_t ESP8266_GetWeather(float *out_temp, char *weather_text);
uint8_t ESP8266_SendWeChatAlert(const char *sendkey, const char *title, const char *content);

#endif /* __ESP8266_H */
