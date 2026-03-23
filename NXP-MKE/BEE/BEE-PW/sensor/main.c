#include "MKE02Z4.h"                    // Device header

#define LOW_LVL 1UL<<0
#define HOM  1UL<<21
#define ESTP 1UL<<22
#define STP 1UL<<23
#define FLT 1UL<<19

#define LOW_LVL_STATUS ((GPIOA->PDIR>>0)&0X01)
#define HOM_STATUS ((GPIOA->PDIR>>21)&0X01)
#define ESTP_STATUS ((GPIOA->PDIR>>22)&0X01)
#define STP_STATUS ((GPIOA->PDIR>>23)&0X01)
#define FLT_STATUS ((GPIOA->PDIR>>19)&0X01)

int main(void){
	
	GPIOA->PIDR&=~(HOM|ESTP|STP);
	PORT->PUEL |=(HOM|ESTP|STP);
	
	while(1){
		if(HOM_STATUS==0){
		}
		if(ESTP_STATUS==0){
		}
		if(STP_STATUS==0){
		}
		
	}
	
	
}