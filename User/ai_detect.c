#include "ai_detect.h"

/**
  * @brief  初始化 AI 温度自适应监测器参数。
  * @param  detector: 指向 AI_Detector 结构体的指针。
  * @param  alpha: 指数移动平均 (EMA) 滤波器平滑因子。
  * @param  max_samples: 算法自适应学习期所需的采样总数。
  * @retval 无
  */
void AI_Init(AI_Detector *detector, float alpha, uint32_t max_samples)
{
    if (detector == 0) return;
    
    detector->alpha = alpha;
    detector->last_filtered_temp = 0.0f;
    detector->baseline_temp = 0.0f;
    detector->learning_samples = 0;
    detector->max_learning_samples = max_samples;
    detector->is_learning_done = 0;
}

/**
  * @brief  对输入温度执行 EMA 滤波并进行自适应异常诊断。
  * @param  detector: 指向 AI_Detector 结构体的指针。
  * @param  raw_temp: 传感器的原始温度输入。
  * @param  out_filtered_temp: 指向输出滤波后温度的浮点指针。
  * @retval 系统检测状态返回值 (AI_STATE_LEARNING, AI_STATE_NORMAL, 或 AI_STATE_ANOMALY)。
  */
uint8_t AI_Process(AI_Detector *detector, float raw_temp, float *out_filtered_temp)
{
    float filtered = 0.0f;

    if (detector == 0 || out_filtered_temp == 0) {
        return AI_STATE_NORMAL;
    }

    /* 1. EMA (指数移动平均) 数字滤波算法 */
    if (detector->learning_samples == 0) {
        /* 上电首个样本，直接将滤波值初始化为原始输入，防止 EMA 算法初始零值漂移 */
        filtered = raw_temp;
    } else {
        filtered = (detector->alpha * raw_temp) + ((1.0f - detector->alpha) * detector->last_filtered_temp);
    }
    detector->last_filtered_temp = filtered;
    *out_filtered_temp = filtered;

    /* 2. 环境温度基准自适应学习阶段 */
    if (detector->is_learning_done == 0) {
        detector->learning_samples++;
        
        /* 递推计算样本的累加平均值: mean(n) = ((n-1)*mean(n-1) + x) / n */
        detector->baseline_temp = (((float)(detector->learning_samples - 1) * detector->baseline_temp) + filtered) 
                                  / (float)detector->learning_samples;

        if (detector->learning_samples >= detector->max_learning_samples) {
            detector->is_learning_done = 1;
        }
        return AI_STATE_LEARNING;
    }

    /* 3. 实时异常状态监控诊断 */
    float diff = filtered - detector->baseline_temp;
    if (diff < 0.0f) {
        diff = -diff; /* 取绝对值 */
    }

    if (diff > 5.0f) {
        return AI_STATE_ANOMALY; /* 温度偏离历史学习基准超过 5°C，触发异常报警 */
    }

    return AI_STATE_NORMAL;
}
