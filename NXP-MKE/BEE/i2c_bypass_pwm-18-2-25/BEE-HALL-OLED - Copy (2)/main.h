#ifndef MAIN_H
#define MAIN_H
#include "stdint.h"


#define CHAR_W         22       // width of one character
#define CHAR_H_PAGES    3       // each character is 3 pages tall

#define P_X             2
#define P_Y             1
#define HH_X            32  //(P_X + 2*(CHAR_W-7))        // after "P:"
#define COLON_X         65  //(HH_X + CHAR_W+11)       // after HH
#define MM_X            85 //(COLON_X + 2*(CHAR_W-12))      // after ":"

#define P_X1             2
#define P_Y1             1
#define HH_X1            46  //(P_X1 + 2*(CHAR_W))        // after "P:"
#define COLON_X1         76  //(HH_X1 + CHAR_W+8)       // after HH
#define MM_X1            96  //(COLON_X1 + 2*(CHAR_W-12))      // after ":"


#define CYCLE_NUM_X      90     // starting X (change as needed)
#define CYCLE_NUM_WIDTH  20     // enough for 2 digits


#define FORWARD 0
#define REVERSE 1

#define MAX_PWM 500  // Adjust based on your required max PWM
#define STEP_DELAY 50 // Delay between each step (in ms)
#define STEP_SIZE 50  // PWM increment per step

#define MOT_LED   1UL<<15
#define MOD_LED   1UL<<24
#define FAULT_LED 1UL<<25
/*#define IN1       1UL<<17
#define IN2       1UL<<18*/
#define PLC_OUT   1UL<<9
#define PLC_OUT1A 1UL<<10

#define HB_LED  1UL<<12

#define MOT_LED_ON (GPIOA->PSOR|=MOT_LED)
#define MOT_LED_OFF (GPIOA->PCOR|=MOT_LED)

/*#define IN1_ON (GPIOA->PSOR|=IN1)   //FTM0 CHANNEL 1
#define IN1_OFF (GPIOA->PCOR|=IN1)  //FTM0 CHANNEL 0

#define IN2_ON (GPIOA->PSOR|=IN2)
#define IN2_OFF (GPIOA->PCOR|=IN2)*/

#define MOD_LED_ON (GPIOA->PSOR|=MOD_LED)
#define MOD_LED_OFF (GPIOA->PCOR|=MOD_LED)

#define FAULT_LED_ON (GPIOA->PSOR|=FAULT_LED)
#define FAULT_LED_OFF (GPIOA->PCOR|=FAULT_LED)

#define PLC_OUT_ON (GPIOA->PSOR |= PLC_OUT);
#define PLC_OUT_OFF (GPIOA->PCOR |= PLC_OUT);

#define PLC_OUT1A_ON (GPIOA->PSOR |= PLC_OUT1A);
#define PLC_OUT1A_OFF (GPIOA->PCOR |= PLC_OUT1A);

#define HB_LED_ON (GPIOA->PSOR |= HB_LED);
#define HB_LED_OFF (GPIOA->PCOR |= HB_LED);

#define KEY1 1UL<<13
#define KEY2 1UL<<14

#define LOW_LVL 1UL<<0
#define HOM  1UL<<21
#define ESTP 1UL<<22
#define STP 1UL<<23
#define FLT 1UL<<19

#define KEY2_PRESSED ((GPIOA->PDIR>>13)&0X01)
#define KEY1_PRESSED ((GPIOA->PDIR>>14)&0X01)

#define LOW_LVL_STATUS ((GPIOA->PDIR>>0)&0X01)
#define HOM_STATUS ((GPIOA->PDIR>>21)&0X01)
#define ESTP_STATUS ((GPIOA->PDIR>>22)&0X01)
#define STP_STATUS ((GPIOA->PDIR>>23)&0X01)
#define FLT_STATUS ((GPIOA->PDIR>>19)&0X01)

void SysTick_Handler(void);
void Delay(uint32_t dlyTicks);
void low_lvl_indic(void);
void itrip_indic(void);
void led_blink(void);
void update_screen(void);
void flashRead(void);
void flashWrite(void);
void present_mode(void);
void exit_setting_mode(void);
void update_time_display(uint16_t counter);
void update_PT_display(uint32_t totalSeconds, uint8_t mode);
void show_mode(char* mode_str, uint8_t mode_id);
void load_pause_digits(uint16_t pause_time);
void next_pt_digit(void);
void increment_pt_digit(void);
uint32_t get_pause_minutes(void);
void display_pause_time(void);


#endif
