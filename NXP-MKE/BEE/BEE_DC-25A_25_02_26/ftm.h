#ifndef   FTM_H
#define   FTM_H

#include "stdint.h"

#define MAX_PWM 500  // Adjust based on your required max PWM
#define STEP_DELAY 50 // Delay between each step (in ms)
#define STEP_SIZE 50  // PWM increment per step

#define FORWARD 0
#define REVERSE 1

void ftm_init(void);
void ramp_up(uint8_t dir,uint32_t target_speed);
void ramp_down(uint8_t dir, uint32_t target_speed);
void ramp_down(uint8_t dir, uint32_t target_speed);


#endif
