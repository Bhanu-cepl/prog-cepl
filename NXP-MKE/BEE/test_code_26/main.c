#include "MKE02Z4.h"                    // Device header
#include "main.h"
#include "i2c.h"
#include "stdbool.h"
#include "stdio.h"



#define MAX_PWM 500  // Adjust based on your required max PWM
#define STEP_DELAY 50 // Delay between each step (in ms)
#define STEP_SIZE 50  // PWM increment per step

#define FORWARD 0
#define REVERSE 1

#define BIT  1UL<<1

run_case test_case=ADC_CHECK;
uint8_t flt_count=0;

uint16_t adc_val;
float voltage;
char buffer[20];

volatile bool Hom_sens_trig = 0,ESTP_sens_trig=0,STP_sens_trig=0;
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
	
	GPIOA->PDDR|=(RUN_LED|PAUSE_LED|FAULT_LED|N_sleep);
	
	GPIOA->PIDR&=~(KEY1|KEY2|LOW_LVL|FLT);
	PORT->PUEL |=(KEY1|KEY2|LOW_LVL|FLT);
}

void adc_init(void){
	SIM->SCGC|=SIM_SCGC_ADC_MASK;
	ADC->SC1=0;
	ADC->SC2 &= ~0X40;
	ADC->SC3|=0X00|0X10|0X08;
	ADC->APCTL1 |=(BIT);
}

uint16_t adc_read(void)
{
    ADC->SC1 = 1;                 // Select channel ADC0_SE1

    while(!(ADC->SC1 & 0x80));    // Wait for conversion complete

    return ADC->R;                // Return ADC result
}
float adc_to_voltage(uint16_t adc_value)
{
    float voltage;

    voltage = ((float)adc_value * 5) / 4095.0;

    return voltage;
}
int pwm = 0;


void ftm_init(void) {
    SIM->SCGC |= SIM_SCGC_FTM2_MASK; // Enable clock for FTM0
	  SIM->PINSEL &= ~ (SIM_PINSEL_FTM2PS2_MASK|SIM_PINSEL_FTM2PS1_MASK);//pin sel for ftm channel
    FTM2->MOD = 500;                   // Set the modulo value for PWM frequency need to inc to dec the freq
    FTM2->CONTROLS[2].CnSC |= 0X38;    // Config channel 1 for PWM
    FTM2->CONTROLS[2].CnV = 0;          // Init duty cycle to 0
    FTM2->SC = 0X03 | 0X08;             // Set prescaler and enable FTM
  
    FTM2->MOD = 500;                   // Set the modulo value for PWM frequency need to inc to dec the freq
    FTM2->CONTROLS[1].CnSC |= 0X38;    // Config channel 1 for PWM
    FTM2->CONTROLS[1].CnV = 0;          // Init duty cycle to 0
    FTM2->SC = 0X03 | 0X08;            // Set prescaler to 8 and clock to system clock and enable FTM 40Mhz/8=5Mhz,5Mhz/241=20khz
}


void ramp_up(uint8_t dir,uint32_t target_speed) {
	/*FTM2->CONTROLS[2].CnV = 0;     // Disable reverse
  FTM2->CONTROLS[1].CnV = 0;   // Enable forward*/
	if(target_speed==0 || target_speed>MAX_PWM){
		target_speed=MAX_PWM;
	}
    for (pwm = 0; pwm <= target_speed; pwm += STEP_SIZE) {
        if (dir) {
            FTM2->CONTROLS[2].CnV = 0;     // Disable reverse
            FTM2->CONTROLS[1].CnV = pwm;   // Enable forward
        } else {
            FTM2->CONTROLS[1].CnV = 0;     // Disable forward
            FTM2->CONTROLS[2].CnV = pwm;   // Enable reverse
        }
        Delay(STEP_DELAY);
    }
}

void ramp_down(uint8_t dir, uint32_t target_speed) {

    bool exit_flag = false;

   if(target_speed==0 || target_speed>MAX_PWM){
		target_speed=MAX_PWM;
	}

    for(pwm = target_speed; pwm >= 0; pwm -= STEP_SIZE) {

       /* if(ESTP_STATUS == 1 || HOM_STATUS == 1) {
					  pwm=0;
            exit_flag = true;
            break;       // exit the for-loop
        }*/

        if (dir) {
            FTM2->CONTROLS[2].CnV = 0;
            FTM2->CONTROLS[1].CnV = pwm;
        } else {
            FTM2->CONTROLS[1].CnV = 0;
            FTM2->CONTROLS[2].CnV = pwm;
        }

        Delay(STEP_DELAY);
    }

    // AFTER BREAK — STOP THE MOTOR
    FTM2->CONTROLS[1].CnV = 0;
    FTM2->CONTROLS[2].CnV = 0;

    if(exit_flag) {
        return;
    }
}
void kbi_init(void){
	
 SIM->SCGC |= (SIM_SCGC_KBI0_MASK|SIM_SCGC_KBI1_MASK);      // Enable KBI0 clock
	KBI0->SC &= ~KBI_SC_KBIE_MASK;       // Disable interrupt during config
	KBI0->ES |= (1<<3);       // Falling edge on P3 (ACTIVE LOW)
	PORT->PUEL|=HOM;
	KBI0->PE |= (1<<3);        // Enable P3
	KBI0->SC |= 0X02;        // Enable interrupt
	
	KBI1->SC &= ~KBI_SC_KBIE_MASK;       // Disable interrupt during config
	KBI1->ES |= (1 << 2) | (1 << 3);;       // Falling edge on P3 (ACTIVE LOW)
	PORT->PUEL|=(ESTP|STP);
	KBI1->PE |= (1 << 2) | (1 << 3);        // Enable P3
	KBI1->SC |= 0X02;        // Enable interrupt*/
	
	NVIC_SetPriority(KBI0_IRQn, 0);
	NVIC_SetPriority(KBI1_IRQn, 0);
  NVIC_EnableIRQ(KBI0_IRQn); 
  NVIC_EnableIRQ(KBI1_IRQn);
	
}



void KBI0_IRQHandler(void)
{
    if (KBI0->SC & KBI_SC_KBF_MASK)    // KBF flag
    {
        /* Read actual pin state */
        if (HOM_STATUS==1)   // Active LOW
        {
            Hom_sens_trig = 1;
        }

        /* Clear interrupt flag */
				KBI0->SC |= KBI_SC_KBACK_MASK;
    }

}

void KBI1_IRQHandler(void)
{
  if (KBI1->SC & KBI_SC_KBF_MASK)   // KBF flag
    {
        /* Read actual pin state */
        if (ESTP_STATUS==1)   // Active LOW
        {
            ESTP_sens_trig = 1;
        }
				if (STP_STATUS==1)   // Active LOW
        {
            STP_sens_trig = 1;
        }

        /* Clear interrupt flag */
				KBI1->SC |= KBI_SC_KBACK_MASK;
    }

}
int main(){
	
	SystemCoreClockUpdate(); //get Core Clock Frequency
  SysTick_Config(SystemCoreClock/1000); //Generate interrupt each 1 ms
  NVIC_SetPriority(SysTick_IRQn, 2); //Set Int priority and enable Intr
	
	Delay(100);
  mcu_init();	
	ftm_init();
	kbi_init();
	adc_init();
	i2c_int();
	oled_init();
	oled_blank();
	N_sleep_on;
	test_case=ADC_CHECK;
	
	while(1){
		switch(test_case){
			case LED:
			RUN_LED_ON;
			Delay(1000);
			PAUSE_LED_ON;
			Delay(1000);
			FAULT_LED_ON;
			Delay(1000);
			RUN_LED_OFF;
			PAUSE_LED_OFF;
			FAULT_LED_OFF;
			Delay(1000);
				break;
			case KEY:
				if(KEY1_PRESSED==0){
					Delay(150);
					if(KEY1_PRESSED==0){
						oled_blank();
						print_string("key1",2,1,4);
						
					}
				}
				if(KEY2_PRESSED==0){
					Delay(150);
					if(KEY2_PRESSED==0){
						oled_blank();
						print_string("key2",2,1,4);
					}
				}
				break;
			case SENSOR:
				if (HOM_STATUS==0)
        {
					oled_blank();
					print_string("HOME",2,1,4);
            Hom_sens_trig = 0;
					flt_count=0;
				}
				if (ESTP_STATUS==0)
        {
					oled_blank();
					print_string("ESTP",2,1,4);
            ESTP_sens_trig = 0;
					flt_count=0;
				}
				if (STP_STATUS==0)
        {
					oled_blank();
					print_string("STP",2,1,4);
            STP_sens_trig = 0;
					flt_count=0;
				}
				flt_count=Hom_sens_trig+ESTP_sens_trig+STP_sens_trig;
			   if(flt_count>1){
					 print_string("FAULT",2,1,5);
					 flt_count=0;
				 }
				break;
			case MOTOR:
				ramp_up(FORWARD,500);
			  Delay(1000);
			  ramp_down(FORWARD,500);
			  Delay(10000);
			  ramp_up(REVERSE,500);
			  Delay(1000);
			  ramp_down(REVERSE,500);
			  Delay(10000);
				break;
			
			case ADC_CHECK:
				    
    adc_val = adc_read();

    voltage = adc_to_voltage(adc_val);

    sprintf(buffer,"VDC: %.2f V",voltage);

    print_string(buffer,2,1,10);
				break;
			
		}
	}
}