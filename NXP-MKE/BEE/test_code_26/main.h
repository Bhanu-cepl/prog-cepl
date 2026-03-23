#ifndef MAIN_H
#define MAIN_H


typedef enum{
	LED,
	KEY,
	SENSOR,
	MOTOR,
	ADC_CHECK
}run_case;

#define RUN_LED 1UL<<21
#define PAUSE_LED 1UL<<24
#define FAULT_LED 1UL<<25
#define N_sleep 1UL<<7

#define N_sleep_on  (GPIOA->PSOR|=N_sleep)
#define N_sleep_off (GPIOA->PCOR|=N_sleep)

#define RUN_LED_ON  (GPIOA->PSOR|=RUN_LED)
#define RUN_LED_OFF (GPIOA->PCOR|=RUN_LED)

#define PAUSE_LED_ON  (GPIOA->PSOR|=PAUSE_LED)
#define PAUSE_LED_OFF (GPIOA->PCOR|=PAUSE_LED)

#define FAULT_LED_ON  (GPIOA->PSOR|=FAULT_LED)
#define FAULT_LED_OFF (GPIOA->PCOR|=FAULT_LED)

#define KEY1 1UL<<2
#define KEY2 1UL<<13

#define LOW_LVL 1UL<<0
#define HOM  1UL<<3
#define ESTP 1UL<<26
#define STP 1UL<<27
#define FLT 1UL<<19


#define KEY2_PRESSED ((GPIOA->PDIR>>2)&0X01)
#define KEY1_PRESSED ((GPIOA->PDIR>>13)&0X01)

#define LOW_LVL_STATUS ((GPIOA->PDIR>>0)&0X01)
#define HOM_STATUS ((GPIOA->PDIR>>3)&0X01)
#define ESTP_STATUS ((GPIOA->PDIR>>26)&0X01)
#define STP_STATUS ((GPIOA->PDIR>>27)&0X01)
#define FLT_STATUS ((GPIOA->PDIR>>19)&0X01)

#endif