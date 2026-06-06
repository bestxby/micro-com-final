#include "anomaly_log.h"

/**
  * @brief  初始化异常日志环形缓冲区。
  * @param  buf: 指向 AnomalyLogBuffer 结构体的指针。
  * @retval 无
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
  * @brief  向环形队列缓冲区追加一条新的异常报警记录。
  * @param  buf: 指向 AnomalyLogBuffer 结构体的指针。
  * @param  timestamp: 报警触发的系统秒时间戳。
  * @param  baseline: 瞬时系统自适应基准温度值。
  * @param  current_t: 瞬时测量滤波后温度值。
  * @param  current_p: 瞬时大气压强值。
  * @retval 无
  */
void Log_Add(AnomalyLogBuffer *buf, uint32_t timestamp, float baseline, float current_t, float current_p)
{
    if (buf == 0) return;

    /* 将报警快照存入 head 指针对应的索引 */
    buf->logs[buf->head].timestamp_s = timestamp;
    buf->logs[buf->head].baseline_temp = baseline;
    buf->logs[buf->head].current_temp = current_t;
    buf->logs[buf->head].current_press = current_p;

    /* 更新 head 写入位置 (环形循环回绕) */
    buf->head = (buf->head + 1) % MAX_ANOMALY_LOGS;

    /* 递增有效记录总数，达到满容量后保持为 MAX_ANOMALY_LOGS，以覆盖最旧的数据 */
    if (buf->count < MAX_ANOMALY_LOGS) {
        buf->count++;
    }
}
