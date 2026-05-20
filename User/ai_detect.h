#ifndef __AI_DETECT_H
#define __AI_DETECT_H

#include <stdint.h>

/* AI State Definitions */
#define AI_STATE_NORMAL      0
#define AI_STATE_ANOMALY     1
#define AI_STATE_LEARNING    2

/* AI Anomaly Detection Structure */
typedef struct {
    float alpha;                 /* EMA smoothing factor (e.g., 0.2) */
    float last_filtered_temp;    /* Last filtered temperature value */
    float baseline_temp;         /* Adaptive baseline temperature mean */
    uint32_t learning_samples;   /* Number of samples collected in learning phase */
    uint32_t max_learning_samples; /* Total samples required for learning phase */
    uint8_t is_learning_done;    /* 0 = Learning in progress, 1 = Learning finished */
} AI_Detector;

/* Public API */
void AI_Init(AI_Detector *detector, float alpha, uint32_t max_samples);
uint8_t AI_Process(AI_Detector *detector, float raw_temp, float *out_filtered_temp);

#endif /* __AI_DETECT_H */
