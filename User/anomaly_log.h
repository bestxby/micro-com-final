#ifndef __ANOMALY_LOG_H
#define __ANOMALY_LOG_H

#include <stdint.h>

#define MAX_ANOMALY_LOGS    5

/* Anomaly Log Event Record Structure */
typedef struct {
    uint32_t timestamp_s;   /* System uptime in seconds when triggered */
    float baseline_temp;    /* Adaptive learning baseline temperature */
    float current_temp;     /* Filtered temperature at trigger time */
    float current_press;    /* Current barometric pressure in Pa */
} AnomalyEvent;

/* Ring Buffer Structure for Logs */
typedef struct {
    AnomalyEvent logs[MAX_ANOMALY_LOGS];
    uint8_t head;           /* Points to the index where the next log will be written */
    uint8_t count;          /* Total active log records stored (max MAX_ANOMALY_LOGS) */
} AnomalyLogBuffer;

/* Public API */
void Log_Init(AnomalyLogBuffer *buf);
void Log_Add(AnomalyLogBuffer *buf, uint32_t timestamp, float baseline, float current_t, float current_p);

#endif /* __ANOMALY_LOG_H */
