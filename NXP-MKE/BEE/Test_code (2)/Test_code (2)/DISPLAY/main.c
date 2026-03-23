#include "MKE02Z4.h"                    // Device header
#include "main.h"
#include "oled.h"

#define PLC_OUT   1UL<<9


#define PLC_OUT_ON (GPIOA->PSOR |= PLC_OUT);
#define PLC_OUT_OFF (GPIOA->PCOR |= PLC_OUT);

volatile uint32_t msTicks = 0;
uint32_t curTicks = 0;
void SysTick_Handler(void) {
    msTicks++;
}

void Delay(uint32_t dlyTicks) {
    curTicks = msTicks;
    while ((msTicks - curTicks) < dlyTicks);
}

int main(){
	
	SystemCoreClockUpdate(); //get Core Clock Frequency
  SysTick_Config(SystemCoreClock/1000); //Generate interrupt each 1 ms
  NVIC_SetPriority(SysTick_IRQn, 2); //Set Int priority and enable Intr
	
	  SIM->SOPT &=~(SIM_SOPT_NMIE_MASK);
	GPIOA->PDDR |=((1UL<<1)|(1UL<<0)|PLC_OUT|1UL<<12);
	PLC_OUT_ON;
	oled_init();
	oled_blank();
	Delay(2);
 //oled_scroll_left();
	while(1){
		print_string("TEST COD",0,1,8);
		Delay(1500);
    oled_blank();
		Delay(1500);
		
	}
	
}