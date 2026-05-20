#ifndef __AI_DETECT_H
#define __AI_DETECT_H

#include <stdint.h>

/* AI 算法状态宏定义 */
#define AI_STATE_NORMAL      0   /* 系统正常状态 */
#define AI_STATE_ANOMALY     1   /* 温度偏差异常报警状态 */
#define AI_STATE_LEARNING    2   /* 上电基准温度自适应学习期 */

/* AI 自适应温度监测器数据结构 */
typedef struct {
    float alpha;                 /* 指数移动平均 (EMA) 滤波器平滑因子 (0.0 < alpha <= 1.0) */
    float last_filtered_temp;    /* 上一次滤波后的温度数据值 */
    float baseline_temp;         /* 学习期结束后锁定的环境温度基准值 */
    uint32_t learning_samples;   /* 学习期内当前已采集的样本计数 */
    uint32_t max_learning_samples; /* 学习期所要求的总采样样本次数 */
    uint8_t is_learning_done;    /* 学习阶段完成标志位: 0 = 学习中, 1 = 学习结束，进入监测 */
} AI_Detector;

/* 外部公开接口 */
void AI_Init(AI_Detector *detector, float alpha, uint32_t max_samples);
uint8_t AI_Process(AI_Detector *detector, float raw_temp, float *out_filtered_temp);

#endif /* __AI_DETECT_H */
