#include "MKE02Z4.h"                    // Device header
#include "main.h"
#include "i2c.h"
#include <stdio.h>
#include "string.h"
#include "stdbool.h"


#define CHAR_W         22       // width of one character
#define CYCLE_NUM_X      90     // starting X (change as needed)
#define CYCLE_NUM_WIDTH  20     // enough for 2 digits

char dis_str[10];
bool update_flag=false;
uint32_t s_delay=0,cycle=0;
volatile uint32_t msTicks = 0;
uint32_t curTicks = 0;
void SysTick_Handler(void) {
    msTicks++;
}

void Delay(uint32_t dlyTicks) {
    curTicks = msTicks;
    while ((msTicks - curTicks) < dlyTicks);
}

void mcu_init(void){

	GPIOA->PIDR&=~(KEY1|KEY2);
	PORT->PUEL |=(KEY1|KEY2);
	
}



int main(void){
	
	SystemCoreClockUpdate(); //get Core Clock Frequency
  SysTick_Config(SystemCoreClock/1000); //Generate interrupt each 1 ms
  NVIC_SetPriority(SysTick_IRQn, 2); //Set Int priority and enable Intr
	
	mcu_init();
	while(1){
		if(KEY1_PRESSED==0){
			 Delay(80);
			 if(KEY1_PRESSED==0){
				 update_flag=true;
				 s_delay=0;
				 cycle++;
				 if(cycle>50){
					 cycle=50;}
			 }
		 }
		 if(KEY2_PRESSED==0){
			 Delay(80);
			 if(KEY2_PRESSED==0){
				 update_flag=true;
				 s_delay=0;
				 cycle--;
			 }
		 }
		 if(cycle>50 || cycle<=0){
					cycle=1;
				}
		 if(update_flag==true){
		oled_clear_area(CYCLE_NUM_X, 1, CYCLE_NUM_WIDTH);
    sprintf(dis_str, "%02d", cycle);
  print_string(dis_str, CYCLE_NUM_X, 1, strlen(dis_str));
			 		 update_flag=false;}
	}
}


