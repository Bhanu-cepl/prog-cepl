#ifndef MAIN_H
#define MAIN_H
#include "stdint.h"
#include "stdbool.h"

#define nsleep      1UL<<9
#define FAULT_LED   1UL<<25
#define RUN_LED      1UL<<15
#define PAUSE_LED   1UL<<24

#define PLC_OU1A  1UL<<11
#define PLC_OU1B  1UL<<10

#define nsleep_ON (GPIOA->PSOR |= nsleep);
#define nsleep_OFF (GPIOA->PCOR |= nsleep);

#define FAULT_LED_ON (GPIOA->PSOR|=FAULT_LED)
#define FAULT_LED_OFF (GPIOA->PCOR|=FAULT_LED)

#define RUN_LED_ON (GPIOA->PSOR|=RUN_LED)
#define RUN_LED_OFF (GPIOA->PCOR|=RUN_LED)

#define PAUSE_LED_ON (GPIOA->PSOR|=PAUSE_LED)
#define PAUSE_LED_OFF (GPIOA->PCOR|=PAUSE_LED)
#define PLC_OU1A_ON  GPIOA->PSOR|=PLC_OU1A
#define PLC_OU1A_OFF  GPIOA->PCOR|=PLC_OU1A

#define PLC_OU1B_ON  GPIOA->PSOR|=PLC_OU1B
#define PLC_OU1B_OFF  GPIOA->PCOR|=PLC_OU1B

#define KEY1 1UL<<13
#define KEY2 1UL<<14

#define LOW_LVL 1UL<<0
#define HOM  1UL<<21
#define ESTP 1UL<<22
#define STP 1UL<<23
#define FLT 1UL<<19

#define PLC_IN 1UL<<16

#define KEY2_PRESSED ((GPIOA->PDIR>>13)&0X01)
#define KEY1_PRESSED ((GPIOA->PDIR>>14)&0X01)

#define LOW_LVL_STATUS ((GPIOA->PDIR>>0)&0X01)
#define HOM_STATUS ((GPIOA->PDIR>>21)&0X01)
#define ESTP_STATUS ((GPIOA->PDIR>>22)&0X01)
#define STP_STATUS ((GPIOA->PDIR>>23)&0X01)
#define FLT_STATUS ((GPIOA->PDIR>>19)&0X01)

#define PLC_IN_STATUS ((GPIOA->PDIR>>16)&0X01)

typedef enum {
    FACTORY_SETTING=1,
    PASSWORD,
	  NPN_PNP_SEL,
	  RUN_TIME,
	  PAUSE_TIME,
	  MODE_SETTING,
	  CYCLE_SETTING,
	  PAUSE_SETTING,
	  FAULT,
    NONE
} SystemMode;

typedef enum {
	NORMAL,
	PLC
}Runmode;

typedef struct {
    char *mode;
    bool pressure_release;
} ModeConfig;

extern SystemMode sel_mode;
extern Runmode run;
extern uint8_t digit;
extern uint8_t enter_mode;
extern uint8_t fact_sel_mode;
extern uint8_t stroke,stroke_completed;
extern bool LOW_LVL_FLAG,TRIP_FLAG,dualMode;
extern uint64_t pause_time;
extern uint8_t digit_active;
extern uint8_t plc;
extern uint8_t pre_fact_sel;
extern uint16_t speed;
extern uint32_t s_delay;
extern uint64_t timer;


void Delay(uint32_t dlyTicks);
void updat_password(int digit);
void shift_place(void);
void flashReadPassword(void);
void flashWritePassword(void);
void update_PT_display(uint32_t totalSeconds, uint8_t mode);

#endif

