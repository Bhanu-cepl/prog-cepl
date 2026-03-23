#include "MKE02Z4.h"                    // Device header
#include "oled.h"
#include "main.h"
#include "stdbool.h"
#include "flash.h"
#include "ke02_config.h"
#include <stdio.h>
#include "string.h"


uint8_t pt_digits[4] = {0,0,0,0};
uint8_t pt_index = 0;

uint8_t blink_state = 1;
uint32_t last_blink_tick = 0;

uint32_t cur_val = 0;

uint32_t s_delay=0,timer=0,pause_time=0,PauseTime=0,m_timer=0,RemainingSeconds=0,called=0;
uint8_t disp_mode=0;
uint8_t last_shown_mode = 0;   // invalid value to force first update
uint16_t mode=0,run_mode=0,month=0,weeks=0;
uint64_t s_cycle=0,p_cycle=0,s_pausetime=0,p_pausetime=0;
uint16_t speed = 0,duty_cycle=0,cycle=0,sel_ml=0;
float total_minute=0,possible_cycle=0,b_vlt=0,fractionPart=0;
uint8_t set_mode=0,set_ml=0;
uint8_t direction = 0,cycle_completed=0,current_cycle=0; 
uint8_t blink_timer=0;
int pwm = 0;
uint32_t check=0,check_timer=0;

static uint8_t old_h = 255;
static uint8_t old_m = 255;
static uint8_t colon_drawn = 0;
static uint8_t old_s = 255;
static uint8_t last_mode = 255;


double every_min=0;
float digit=0,dig=0;
char dis_str[10];
bool LOW_LVL_FLAG=false,TRIP_FLAG=false,blink_called=false,last_cycle=false,update_flag=false;

char rtime_str[10];
struct timervalue{
uint32_t Seconds;
uint32_t Minutes;
uint32_t Hours;
};

typedef enum OP_MODE{ /* Operation Modes*/
RUN_TIME=1,
PAUSE_TIME,
MANUAL,	
SETTING,
CYCLE_SETTING,
PAUSETIME_SETTING,
ML,SET_MONTHS,
SET_WEEKS,RPM,
FAULT,
}opt_mode;

typedef enum RUN_MODES{
	SINGLE_LINE=1,
	PROGRESSIVE,
	MONTH,WEEK
}opt_run_mode;

struct flashdata{
	uint32_t s_cycle;
	uint32_t p_cycle;
	uint32_t s_pausetime;
	uint32_t p_pausetime;
	uint32_t prev_mode;
};
struct flashdata memory;

struct timervalue runtime,pausetime,elapsedsetting;

volatile uint32_t msTicks = 0;
uint32_t curTicks = 0;
void SysTick_Handler(void) {
    msTicks++;
	if(mode==PAUSETIME_SETTING && check==1){
		check_timer++;
	}
	
}

void Delay(uint32_t dlyTicks) {
    curTicks = msTicks;
    while ((msTicks - curTicks) < dlyTicks);
}
void rtc_init(){
	/* RTC INITIALIZATION */
	 SIM->SCGC |= SIM_SCGC_RTC_MASK;
    RTC->SC &= 0x00000000;
    RTC->MOD = 16000; /* RTC MOD for 1 Second @ 24MHz Bus Clock */
    RTC->SC = RTC_SC_RTCLKS(0x03) | RTC_SC_RTCPS_MASK | RTC_SC_RTIF_MASK | RTC_SC_RTIE_MASK;
    NVIC_EnableIRQ(RTC_IRQn);
    NVIC_SetPriority(RTC_IRQn, 1);
}

void RTC_IRQHandler(void){
	s_delay++;
	 RTC->SC |=RTC_SC_RTIF_MASK | RTC_SC_RTIE_MASK ;
	 timer++;
	
	 if (elapsedsetting.Seconds > 0) {
    elapsedsetting.Seconds--;
} else {
    if (elapsedsetting.Minutes > 0) {
			 elapsedsetting.Seconds = 59;
        elapsedsetting.Minutes--;
    } else if (elapsedsetting.Hours > 0) {
        elapsedsetting.Minutes = 59;
        elapsedsetting.Hours--;
    }
} 

blink_timer++;
if(blink_timer>=20){
	blink_called=false;
	blink_timer=0;
}
if(run_mode==MONTH && mode!=PAUSE_TIME){
	m_timer++;
}
	

 if(elapsedsetting.Hours >= 1) {
	 
	      rtime_str[2] = ':';
        rtime_str[3]=(elapsedsetting.Hours%100)/10+ '0';
        rtime_str[4]= elapsedsetting.Hours%10+ '0';
        rtime_str[5] = ':';
        rtime_str[6] = (elapsedsetting.Minutes) / 10 + '0';
        rtime_str[7] = (elapsedsetting.Minutes) % 10 + '0';
        rtime_str[8] = '\0';
    } 
 else{ //(elapsedsetting.Seconds < 60){
	      rtime_str[2] = ':';
	      rtime_str[3] = (elapsedsetting.Minutes) / 10 + '0';
        rtime_str[4] = (elapsedsetting.Minutes) % 10 + '0';
        rtime_str[5] = ':';
        rtime_str[6] = (elapsedsetting.Seconds) / 10 + '0';
        rtime_str[7] = (elapsedsetting.Seconds) % 10 + '0';
        rtime_str[8] = '\0';
 }
	}	

void mcu_init(void){
	//GPIOA->PDDR|=(1UL<<PIN5|1UL<<PIN6|1UL<<PIN7|1UL<<PIN8|1UL<<PINE|1UL<<PINRS);
	GPIOA->PDDR|=(MOT_LED|MOD_LED|FAULT_LED|PLC_OUT|PLC_OUT1A);
	GPIOA->PIDR&=~(KEY1|KEY2|HOM|LOW_LVL|ESTP|STP|FLT);
	PORT->PUEL |=(KEY1|KEY2|HOM|LOW_LVL|ESTP|STP|FLT);
//GPIOA->PCOR|=(1UL<<3|1UL<<4);
	(void)FLASH_Init(10000000L); 					// Initialize  Flash
	SIM->SCGC |=SIM_SCGC_FLASH_MASK;      // Enable clock for Flash
	
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

void motor_break(void){
	direction ^= 1;
		FTM2->CONTROLS[2].CnV = 240;
			FTM2->CONTROLS[1].CnV = 240;
			GPIOA->PSOR|=(1ul<<1);
			GPIOA->PSOR|=(1ul<<0);
			Delay(200);
}


int main(void){
	
	SystemCoreClockUpdate(); //get Core Clock Frequency
  SysTick_Config(SystemCoreClock/1000); //Generate interrupt each 1 ms
  NVIC_SetPriority(SysTick_IRQn, 2); //Set Int priority and enable Intr
	
	mcu_init();
	ftm_init();
	rtc_init();
	PLC_OUT_ON;
 while(1){
	 ramp_up(0,500);
	 Delay(10000);
	 ramp_down(0,500);
	 Delay(1000);
	 ramp_up(1,500);
	 Delay(10000);
	 ramp_down(1,500);
	 Delay(1000);
 }
 }