#ifndef __ANOMALY_LOG_H
#define __ANOMALY_LOG_H

#include <stdint.h>

#define MAX_ANOMALY_LOGS    5   /* 环形历史记录的最大容量 */

/* 异常历史日志数据结构 */
typedef struct {
    uint32_t timestamp_s;   /* 异常发生时的系统上电运行时间 (秒) */
    float baseline_temp;    /* 发生异常时的自适应环境温度基准 */
    float current_temp;     /* 发生异常时的实时滤波温度 */
    float current_press;    /* 发生异常时的实时大气压强 (Pa) */
} AnomalyEvent;

/* 日志存储环形队列结构体 */
typedef struct {
    AnomalyEvent logs[MAX_ANOMALY_LOGS];
    uint8_t head;           /* 写入指针，指向下一个写入的数组位置 */
    uint8_t count;          /* 队列当前有效记录个数 (最大为 MAX_ANOMALY_LOGS) */
} AnomalyLogBuffer;

/* 外部公开接口 */
void Log_Init(AnomalyLogBuffer *buf);
void Log_Add(AnomalyLogBuffer *buf, uint32_t timestamp, float baseline, float current_t, float current_p);

#endif /* __ANOMALY_LOG_H */
