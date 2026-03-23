#include "MKE02Z4.h"                    // Device header
#include "ftm.h"
#include "main.h"
#include "stdbool.h"


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
