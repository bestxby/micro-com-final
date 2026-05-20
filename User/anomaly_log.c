#include "anomaly_log.h"

/**
  * @brief  Initializes the anomaly log ring buffer.
  * @param  buf: Pointer to the AnomalyLogBuffer structure.
  * @retval None
  */
void Log_Init(AnomalyLogBuffer *buf)
{
    if (buf == 0) return;

    buf->head = 0;
    buf->count = 0;
    for (uint8_t i = 0; i < MAX_ANOMALY_LOGS; i++) {
        buf->logs[i].timestamp_s = 0;
        buf->logs[i].baseline_temp = 0.0f;
        buf->logs[i].current_temp = 0.0f;
        buf->logs[i].current_press = 0.0f;
    }
}

/**
  * @brief  Adds a new anomaly log event to the circular buffer.
  * @param  buf: Pointer to the AnomalyLogBuffer structure.
  * @param  timestamp: System time in seconds.
  * @param  baseline: Calculated baseline temperature.
  * @param  current_t: Filtered trigger temperature.
  * @param  current_p: Measured pressure.
  * @retval None
  */
void Log_Add(AnomalyLogBuffer *buf, uint32_t timestamp, float baseline, float current_t, float current_p)
{
    if (buf == 0) return;

    /* Write log at current head index */
    buf->logs[buf->head].timestamp_s = timestamp;
    buf->logs[buf->head].baseline_temp = baseline;
    buf->logs[buf->head].current_temp = current_t;
    buf->logs[buf->head].current_press = current_p;

    /* Advance head pointer (ring buffer wraps around) */
    buf->head = (buf->head + 1) % MAX_ANOMALY_LOGS;

    /* Increment count up to maximum capacity */
    if (buf->count < MAX_ANOMALY_LOGS) {
        buf->count++;
    }
}
