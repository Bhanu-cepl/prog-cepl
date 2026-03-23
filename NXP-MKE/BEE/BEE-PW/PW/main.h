#ifndef MAIN_H
#define MAIN_H
#include "stdint.h"
#include "stdbool.h"

#define nsleep 1UL<<9

#define nsleep_ON (GPIOA->PSOR |= nsleep);
#define nsleep_OFF (GPIOA->PCOR |= nsleep);

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

typedef enum {
    FACTORY_SETTING=1,
    PASSWORD,
	  RUN_TIME,
	  PAUSE_TIME,
	  SETTING,
	  PAUSE_SETTING,
	  FAULT,
    NONE
} SystemMode;

extern SystemMode sel_mode;
extern uint8_t digit;
extern uint8_t enter_mode;
extern uint8_t fact_sel_mode;
extern uint8_t stroke,stroke_completed;
extern bool LOW_LVL_FLAG,TRIP_FLAG;
extern uint64_t pause_time;
extern uint8_t digit_active;


void Delay(uint32_t dlyTicks);
void updat_password(int digit);
void shift_place(void);
void flashReadPassword(void);
void flashWritePassword(void);
void update_PT_display(uint32_t totalSeconds, uint8_t mode);

#endif

