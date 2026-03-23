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
	
	  //SIM->SOPT &=~(SIM_SOPT_NMIE_MASK);
	GPIOA->PDDR |=((1UL<<25)|(1UL<<15)|PLC_OUT|1UL<<24);
	oled_init();
	while(1){
		GPIOA->PSOR|=(1UL<<24);
		GPIOA->PSOR|=(1UL<<25);
		GPIOA->PSOR|=(1UL<<15);
		Delay(1500);
		GPIOA->PCOR|=(1UL<<24);
		GPIOA->PCOR|=(1UL<<25);	
		GPIOA->PCOR|=(1UL<<15);
		Delay(1500);
	}
	
}