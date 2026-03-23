#include "MKE02Z4.h"                    // Device header
#include "stdbool.h"

#define MAX_PWM 500  // Adjust based on your required max PWM
#define STEP_DELAY 50 // Delay between each step (in ms)
#define STEP_SIZE 50  // PWM increment per step

#define FORWARD 0
#define REVERSE 1

#define HOM  1UL<<21
#define ESTP 1UL<<22
#define STP 1UL<<23

#define HOM_STATUS ((GPIOA->PDIR>>21)&0X01)
#define ESTP_STATUS ((GPIOA->PDIR>>22)&0X01)
#define STP_STATUS ((GPIOA->PDIR>>23)&0X01)

int pwm = 0;
uint8_t stroke=0,stroke_completed=0,current_stroke=0;
bool Pressure_release=false;
uint16_t speed=0;

volatile uint32_t msTicks = 0;
uint32_t curTicks = 0;
void SysTick_Handler(void) {
    msTicks++;
}

void Delay(uint32_t dlyTicks) {
    curTicks = msTicks;
    while ((msTicks - curTicks) < dlyTicks);
}

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

        if(ESTP_STATUS == 1 || HOM_STATUS == 1) {
					  pwm=0;
            exit_flag = true;
            break;       // exit the for-loop
        }

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

int main(void){
	
	SystemCoreClockUpdate(); //get Core Clock Frequency
  SysTick_Config(SystemCoreClock/1000); //Generate interrupt each 1 ms
  NVIC_SetPriority(SysTick_IRQn, 2); //Set Int priority and enable Intr
	
	GPIOA->PIDR&=~(HOM|ESTP|STP);
	PORT->PUEL |=(HOM|ESTP|STP);
	
	ftm_init();
	Pressure_release=true;
	stroke=10;
	
	while(1){
		if (stroke_completed < stroke)
{
    bool last_cycle = false;

    if (Pressure_release == true && stroke_completed == (stroke - 1)) {
        last_cycle = true;
    }

    /* ---------- FORWARD MOTION ---------- */
    ramp_up(FORWARD, speed);

    if (last_cycle) {
        while (STP_STATUS == 0) {
            ;   // wait for pressure release stop
        }
    } else {
        while (ESTP_STATUS == 0) {
            ;   // wait for end stroke sensor
        }
    }

    ramp_down(FORWARD, speed);
    Delay(1000);

    /* ---------- REVERSE MOTION ---------- */
    if (!last_cycle) {
        ramp_up(REVERSE, speed);

        while (HOM_STATUS == 0) {
            ;   // wait for home sensor
        }

        ramp_down(REVERSE, speed);
        Delay(200);
    }

    stroke_completed++;
}

/* ---------- ALL STROKES DONE ---------- */

	}
	
}