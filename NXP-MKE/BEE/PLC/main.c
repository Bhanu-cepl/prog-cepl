#include "MKE02Z4.h"                    // NXP::Device:Startup

#define PLC_OU1A  1UL<<11
#define PLC_OU1B  1UL<<10
#define LED1      1UL<<24
#define LED2      1UL<<25

#define PLC_OU1A_ON  GPIOA->PSOR|=PLC_OU1A
#define PLC_OU1A_OFF  GPIOA->PCOR|=PLC_OU1A

#define PLC_OU1B_ON  GPIOA->PSOR|=PLC_OU1B
#define PLC_OU1B_OFF  GPIOA->PCOR|=PLC_OU1B

#define LED1_ON   GPIOA->PSOR|=LED1
#define LED1_OFF  GPIOA->PCOR|=LED1

#define LED2_ON   GPIOA->PSOR|=LED2
#define LED2_OFF  GPIOA->PCOR|=LED2

#define KEY1 1UL<<13
#define KEY2 1UL<<14
#define PLC_IN 1UL<<16

#define KEY1_PRESSED ((GPIOA->PDIR>>13)&0x01)
#define KEY2_PRESSED ((GPIOA->PDIR>>14)&0x01)
#define PLC_IN_STS   ((GPIOA->PDIR>>16)&0x01)

volatile int32_t msticks=0;
void delay(int32_t delay){
	uint32_t currentticks=msticks;
	while((msticks-currentticks)<delay){
		
	}
	
}
void SysTick_Handler(void){
	msticks++;}

void mcu_init(void){
	GPIOA->PDDR|=(PLC_OU1A|PLC_OU1B|LED1|LED2);
	GPIOA->PIDR&=~(KEY1|KEY2|PLC_IN);
	PORT->PUEL|=(KEY1|KEY2|PLC_IN);
}

typedef enum {
	NONE=0,
	NPN,
	PNP
}TRANSISTER;

TRANSISTER PNP_NPN_SEL=NONE;

int main(void){
		SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock / 1000);
	NVIC_SetPriority(SysTick_IRQn, 2);
	
	mcu_init();
	
	while(1){
	
	switch(PNP_NPN_SEL){
		
		case NONE:
			if(KEY1_PRESSED==0 && KEY2_PRESSED==1){
				delay(150);
				if(KEY1_PRESSED==0 && KEY2_PRESSED==1){
				PNP_NPN_SEL=NPN;

			}}
			if(KEY1_PRESSED==1 && KEY2_PRESSED==0){
				delay(150);
				if(KEY1_PRESSED==1 && KEY2_PRESSED==0){
				PNP_NPN_SEL=PNP;

			}
			}
			break;
		case NPN:
		if(PLC_IN_STS==0){
			delay(150);
			if(PLC_IN_STS==0){
				PLC_OU1A_OFF;
				PLC_OU1B_ON;
					LED1_ON;
					LED2_OFF;
			}
		}
			break;
		case PNP:
			if(PLC_IN_STS==0){
			delay(150);
			if(PLC_IN_STS==0){
				PLC_OU1A_ON;
				PLC_OU1B_OFF;
				LED1_OFF;
				LED2_ON;
			}
		}
			break;
		
	}
}
	
	
	
	
}