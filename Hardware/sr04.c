#include "sr04.h"

#define SR04_TRIG_H()   (GPIOB->BSRR = GPIO_Pin_11)
#define SR04_TRIG_L()   (GPIOB->BRR  = GPIO_Pin_11)
#define SR04_ECHO_READ() ((GPIOB->IDR & GPIO_Pin_12) ? 1 : 0)

static void SR04_DelayUs(uint32_t us)
{
    volatile uint32_t count = us * 10;
    while (count--);
}

void SR04_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    (void)RCC->APB2ENR;
    
    /* 
     * Configure PB11 (Trig) as General Output Push-Pull (CNF=00, MODE=11 -> 0x3)
     * Configure PB12 (Echo) as Input with Pull-Up/Pull-Down (CNF=10, MODE=00 -> 0x8)
     */
    GPIOB->CRH &= ~0x000FF000;
    GPIOB->CRH |=  0x00083000;
    
    /* Select Pull-Down for PB12 (write 0 to ODR) */
    GPIOB->ODR &= ~GPIO_Pin_12;
    
    SR04_TRIG_L();
}

/**
  * @brief  Trigger HC-SR04 and measure the distance.
  * @param  dist_cm: Pointer to float where distance in cm is saved.
  * @retval 0 = Success, 1 = Timeout/Sensor Error.
  */
uint8_t SR04_GetDistance(float *dist_cm)
{
    uint32_t count = 0;
    
    /* 1. Send >10us High pulse to Trig pin */
    SR04_TRIG_H();
    SR04_DelayUs(12);
    SR04_TRIG_L();
    
    /* 2. Wait for Echo pin to go High */
    count = 0;
    while (SR04_ECHO_READ() == 0) {
        count++;
        if (count > 60000) {
            return 1; /* Timeout, sensor not connected or faulty */
        }
    }
    
    /* 3. Measure the duration of Echo pin being High */
    count = 0;
    while (SR04_ECHO_READ() != 0) {
        count++;
        /* Calibrate loop execution time to ~1us per loop iteration */
        for (volatile int d = 0; d < 6; d++);
        if (count > 30000) {
            break; /* Range limit (approx. 400cm) */
        }
    }
    
    /* 
     * Speed of sound is 340m/s = 0.034cm/us.
     * With loop calibrated to ~0.77us per count:
     * Distance (cm) = count * 0.77us * 0.034cm/us / 2 = count * 0.01309f.
     */
    *dist_cm = (float)count * 0.01309f;
    
    return 0;
}
