#include "ai_detect.h"

/**
  * @brief  Initializes the AI Detector parameters.
  * @param  detector: Pointer to the AI_Detector structure.
  * @param  alpha: Exponential Moving Average (EMA) smoothing factor (0.0 < alpha <= 1.0).
  * @param  max_samples: Number of samples required during the learning phase.
  * @retval None
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
  * @brief  Filters incoming temperature and evaluates system anomaly state.
  * @param  detector: Pointer to the AI_Detector structure.
  * @param  raw_temp: Raw temperature input from the sensor.
  * @param  out_filtered_temp: Pointer to float to store the output EMA filtered value.
  * @retval AI_STATE_LEARNING, AI_STATE_NORMAL, or AI_STATE_ANOMALY.
  */
uint8_t AI_Process(AI_Detector *detector, float raw_temp, float *out_filtered_temp)
{
    float filtered = 0.0f;

    if (detector == 0 || out_filtered_temp == 0) {
        return AI_STATE_NORMAL;
    }

    /* 1. Exponential Moving Average (EMA) Filtering */
    if (detector->learning_samples == 0) {
        /* On first sample, initialize filter value to raw to prevent startup lag */
        filtered = raw_temp;
    } else {
        filtered = (detector->alpha * raw_temp) + ((1.0f - detector->alpha) * detector->last_filtered_temp);
    }
    detector->last_filtered_temp = filtered;
    *out_filtered_temp = filtered;

    /* 2. Adaptive Learning Phase */
    if (detector->is_learning_done == 0) {
        detector->learning_samples++;
        
        /* Calculate incremental mean: mean(n) = ((n-1)*mean(n-1) + x) / n */
        detector->baseline_temp = (((float)(detector->learning_samples - 1) * detector->baseline_temp) + filtered) 
                                  / (float)detector->learning_samples;

        if (detector->learning_samples >= detector->max_learning_samples) {
            detector->is_learning_done = 1;
        }
        return AI_STATE_LEARNING;
    }

    /* 3. Monitoring & Anomaly Detection Phase */
    float diff = filtered - detector->baseline_temp;
    if (diff < 0.0f) {
        diff = -diff; /* Get absolute value */
    }

    if (diff > 5.0f) {
        return AI_STATE_ANOMALY; /* Deviation exceeds 5 degrees C */
    }

    return AI_STATE_NORMAL;
}
