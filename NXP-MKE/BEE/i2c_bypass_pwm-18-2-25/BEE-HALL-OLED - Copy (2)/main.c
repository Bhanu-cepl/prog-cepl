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
if(blink_timer>=10){
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
	
	SIM->SOPT &=~(SIM_SOPT_NMIE_MASK);
	//GPIOA->PDDR|=(1UL<<PIN5|1UL<<PIN6|1UL<<PIN7|1UL<<PIN8|1UL<<PINE|1UL<<PINRS);
	GPIOA->PDDR|=(MOT_LED|MOD_LED|FAULT_LED|PLC_OUT|PLC_OUT1A|HB_LED);
	GPIOA->PIDR&=~(KEY1|KEY2|HOM|LOW_LVL|ESTP|STP|FLT);
	PORT->PUEL |=(KEY1|KEY2|HOM|LOW_LVL|ESTP|STP|FLT);
	
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


/*void ramp_down(uint8_t dir,uint32_t target_speed) {
	FTM2->CONTROLS[2].CnV = 0;     // Disable reverse
  FTM2->CONTROLS[1].CnV = 0;   // Enable forward
	if(target_speed==0){
		target_speed=MAX_PWM;
	}
    for (pwm = target_speed; pwm >= 0; pwm -= STEP_SIZE) {
        if (dir) {
            FTM2->CONTROLS[2].CnV = 0;
            FTM2->CONTROLS[1].CnV = pwm;
        } else {
            FTM2->CONTROLS[1].CnV = 0;
            FTM2->CONTROLS[2].CnV = pwm;
        }
        Delay(STEP_DELAY);
    }
    // Ensure motor stops completely
    FTM2->CONTROLS[2].CnV = 0;
    FTM2->CONTROLS[1].CnV = 0;
}*/

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
	
	Delay(100);
	mcu_init();
	ftm_init();
	rtc_init();
  oled_init();
  Delay(2);
	MOT_LED_ON;
	Delay(150);
	MOD_LED_ON;
	Delay(150);
	FAULT_LED_ON;
	Delay(150);
	MOT_LED_OFF;
	MOD_LED_OFF;
	FAULT_LED_OFF;
	PLC_OUT_ON;
	HB_LED_ON;
  oled_blank();
  print_string("BEE-V3",2,1,6);	
	Delay(1000);
	flashRead();
	present_mode();
	Delay(500);
	if(HOM_STATUS==0){
		ramp_up(REVERSE,speed);
		MOT_LED_ON;
		oled_blank();
		draw_icon(hom,0,0,32,4);
		while (HOM_STATUS == 0){

					}
						oled_blank();
		ramp_down(REVERSE,speed);
		MOT_LED_OFF;
		//motor_break();
	}	
mode=RUN_TIME;
	if(run_mode>=3){
		run_mode=2;}

 while(1){
	 switch(mode){
	 case RUN_TIME:
		 MOD_LED_OFF;
				MOT_LED_ON;
			
	 if(!blink_called){
		 led_blink();
		 blink_called=true;
	 }
				update_screen();
			switch(run_mode){
					case PROGRESSIVE:
						if(cycle_completed<cycle){
							ramp_up(FORWARD,speed);
							while(ESTP_STATUS==0){
										if(KEY1_PRESSED==0){
										Delay(150);
										if(KEY1_PRESSED==0){
												cycle_completed=cycle;
											print_string("SNG LINE",2,1,8);	
											while(KEY1_PRESSED==0){
												if(ESTP_STATUS==1){
													break;}}
											s_delay=0;
											set_mode=1;
											mode=SETTING;
											break;
										}
									}

									if(LOW_LVL_STATUS==0){
										Delay(150);
										if(LOW_LVL_STATUS==0){
											LOW_LVL_FLAG=true;
											MOT_LED_OFF;
											low_lvl_indic();
											cycle_completed=cycle;
											mode=FAULT;
											break;
										}
									}
									if(FLT_STATUS==0){
										TRIP_FLAG=true;
										MOT_LED_OFF;
										itrip_indic();
										cycle_completed=cycle;
										mode=FAULT;
										break;
									}
									if(!blink_called){
										led_blink();
										blink_called=true;
									}

							}								
							ramp_down(FORWARD,speed);
							Delay(1000);  //100
							//motor_break();
							if(cycle_completed<cycle){
							ramp_up(REVERSE,speed);
							while(HOM_STATUS==0){
									 if(KEY1_PRESSED==0){
										Delay(150);
										if(KEY1_PRESSED==0){
												cycle_completed=cycle;
											print_string("SNG LINE",2,1,8);	
											while(KEY1_PRESSED==0){
												if(HOM_STATUS==1){
													break;
												}
											}
											s_delay=0;
											set_mode=1;
											mode=SETTING;
											break;
										}
									}

									if(LOW_LVL_STATUS==0){
										Delay(150);
										if(LOW_LVL_STATUS==0){
											LOW_LVL_FLAG=true;
											low_lvl_indic();
											cycle_completed=cycle;
											mode=FAULT;
											break;
										}
									}
									if(FLT_STATUS==0){
										TRIP_FLAG=true;
										MOT_LED_OFF;
										itrip_indic();
										cycle_completed=cycle;
										mode=FAULT;
										break;
									}
									if(!blink_called){
										led_blink();
										blink_called=true;
									}

							}
							ramp_down(REVERSE,speed);
							Delay(200);
							//motor_break();
						}
							cycle_completed++;
						}
						if(cycle_completed==cycle){
						mode=PAUSE_TIME;
							oled_blank();
							print_string("PT:", P_X1, P_Y1, 3);
						timer=0;
					}
						break;
					case SINGLE_LINE:
						if (cycle_completed < cycle) {
							ramp_up(FORWARD,speed);
							current_cycle=cycle-1;
							if (cycle_completed<current_cycle) {
								while (ESTP_STATUS == 0){
									if(KEY1_PRESSED==0){
										Delay(150);
										if(KEY1_PRESSED==0){
												cycle_completed=cycle;
											print_string("SNG LINE",2,1,8);	
											while(KEY1_PRESSED==0){
												if(ESTP_STATUS==1){
											break;}}
											s_delay=0;
											set_mode=1;
											mode=SETTING;
											break;
										}
									}

									if(LOW_LVL_STATUS==0){
										Delay(150);
										if(LOW_LVL_STATUS==0){
											LOW_LVL_FLAG=true;
											low_lvl_indic();
											cycle_completed=cycle;
											mode=FAULT;
											break;
										}
									}
									if(FLT_STATUS==0){
										TRIP_FLAG=true;
										MOT_LED_OFF;
										itrip_indic();
										cycle_completed=cycle;
										mode=FAULT;
										break;
									}
									if(!blink_called){
										led_blink();
										blink_called=true;
									}

									}
							} else {
								last_cycle=true;
								while (STP_STATUS == 0){
									if(KEY1_PRESSED==0){
										Delay(150);
										if(KEY1_PRESSED==0){
												cycle_completed=cycle;
											print_string("SNG LINE",2,1,8);	
											while(KEY1_PRESSED==0){
												if(STP_STATUS==1){
													break;}}
											s_delay=0;
											set_mode=1;
											mode=SETTING;
											break;
										}
									}
									
									if(LOW_LVL_STATUS==0){
										Delay(150);
										if(LOW_LVL_STATUS==0){
											LOW_LVL_FLAG=true;
											low_lvl_indic();
											cycle_completed=cycle;
											mode=FAULT;
											break;
										}
									}
									if(FLT_STATUS==0){
										TRIP_FLAG=true;
										MOT_LED_OFF;
										itrip_indic();
										cycle_completed=cycle;
										mode=FAULT;
										break;
									}
									if(!blink_called){
										led_blink();
										blink_called=true;
									}

								}
							}
							ramp_down(FORWARD,speed);
							Delay(1000);
							//motor_break();
							if(cycle_completed<current_cycle){
								ramp_up(REVERSE,speed);
								while (HOM_STATUS == 0){
									if(KEY1_PRESSED==0){
										Delay(150);
										if(KEY1_PRESSED==0){
												cycle_completed=cycle;
											print_string("SNG LINE",2,1,8);	
											while(KEY1_PRESSED==0){
												if(HOM_STATUS==1){
													break;
												}
											}
											s_delay=0;
											set_mode=1;
											mode=SETTING;
											break;
										}
									}

									if(LOW_LVL_STATUS==0){
										Delay(150);
										if(LOW_LVL_STATUS==0){
											LOW_LVL_FLAG=true;
											low_lvl_indic();
											cycle_completed=cycle;
											mode=FAULT;
											break;
										}
									}
									if(FLT_STATUS==0){
										TRIP_FLAG=true;
										MOT_LED_OFF;
										itrip_indic();
										cycle_completed=cycle;
										mode=FAULT;
										break;
									}
									if(!blink_called){
										led_blink();
										blink_called=true;
									}

								}
								ramp_down(REVERSE,speed);
								Delay(200);
								//motor_break();
							}
								cycle_completed++;
						}
						
					if(cycle_completed==cycle){
						mode=PAUSE_TIME;
						timer=0;
						oled_blank();
						print_string("PT:", P_X, P_Y, 3);
						m_timer=0;
					}
					break;

				}
				
				break;
					case MANUAL:
					while(KEY2_PRESSED==0){
						MOT_LED_ON;
						MOD_LED_ON;
						print_string("MANUAL  ",2,1,8);	
						ramp_up(FORWARD,speed);
						while(ESTP_STATUS==0);
						ramp_down(FORWARD,speed);
						motor_break();
						ramp_up(REVERSE,speed);
						while(HOM_STATUS==0);
						ramp_down(REVERSE,speed);
						motor_break();
					}
					mode=PAUSE_TIME;
					oled_blank();
					print_string("PT:", P_X1, P_Y1, 3);
					MOT_LED_OFF;
					MOD_LED_OFF;
					timer=0;
          s_delay=0;					
						break;
	 case PAUSE_TIME:
      MOT_LED_OFF;
	 MOD_LED_ON;
			if(KEY1_PRESSED ==1 && KEY2_PRESSED==1){
				if(set_mode==3||set_mode==4){
					PauseTime=(every_min*60)-m_timer;
				}
				else{
					PauseTime=pause_time*60;
				}
				RemainingSeconds=PauseTime-timer;
				 if (RemainingSeconds >= 3600)
        disp_mode = 1;   // show HH:MM
    else
        disp_mode = 0;   // show MM:SS

    update_PT_display(RemainingSeconds, disp_mode);

				
			}
			if(!blink_called){
				led_blink();
				blink_called=true;
			}
			if(KEY1_PRESSED==0){
				Delay(200);
				if(KEY1_PRESSED==0){
					s_delay=0;
					set_mode=1;
					print_string("SNG LINE",2,1,8);
					while(KEY1_PRESSED==0);
					s_delay=0;
					mode=SETTING;
					break;
				}
			}
			

			if (timer >= PauseTime && LOW_LVL_FLAG == false) {
        if (last_cycle==true && run_mode == SINGLE_LINE) {
					ramp_up(REVERSE,speed);
					oled_blank();
						draw_icon(hom,0,0,32,4);
					while (HOM_STATUS == 0){
						
					}
					oled_blank();
					ramp_down(REVERSE,speed);
					Delay(200);
					//motor_break();
					last_cycle=false;
				}
	        old_h = 255;
          old_m = 255;
          colon_drawn = 0;
          old_s = 255;
         last_mode = 255;
				cycle_completed=0;
        mode=RUN_TIME;
				m_timer=0;
         }	
				break;
	 case SETTING:
		 MOT_LED_OFF;
		 if(KEY2_PRESSED==0){
			 Delay(150);
			if(KEY2_PRESSED==0){
				set_mode++;
				s_delay=0;
			}
		}
		 if(set_mode>2){  // mode selection is limitted for 2 mode  
			 set_mode=1;
		 }
		 if(set_mode==1){
			 show_mode("SNG LINE", set_mode);
		 }
		 if(set_mode==2){
			 show_mode("PROGSIVE", set_mode);}
		  
		/* if(set_mode==1){
			 print_string("SNG LINE",2,1,8);
		 }
		 if(set_mode==2){
			 print_string("PROGSIVE",2,1,8);}*/
		 
			if(KEY1_PRESSED==0){
				Delay(150);
				if(KEY1_PRESSED==0){
					s_delay=0;
					update_flag=true;
					 if (set_mode == 1){
						 run_mode=SINGLE_LINE;
						 oled_blank();
						 print_string("CYCLE-", 2, 1, 6);   // stays fixed
						 mode=CYCLE_SETTING;
					  cycle = s_cycle;
						 pause_time=s_pausetime;
					 }
					 else if(set_mode == 2){
						  run_mode=PROGRESSIVE;
						 oled_blank();
						 print_string("CYCLE-", 2, 1, 6);   // stays fixed
						 mode=CYCLE_SETTING;
						 cycle = p_cycle;
					   pause_time=p_pausetime;}

         mode = CYCLE_SETTING;
					 }
			 }
			if(KEY1_PRESSED==1 && KEY2_PRESSED==1 && s_delay>3){
				if(KEY1_PRESSED==1 && KEY2_PRESSED==1){
					set_mode=1;
					update_flag=true;
					exit_setting_mode();
					cycle_completed=0;
					mode=RUN_TIME;
					s_delay=0;
				}
			}
		 break;
	 case CYCLE_SETTING:
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
		if(KEY1_PRESSED==1 && KEY2_PRESSED==1 && s_delay>2){
		 if(KEY1_PRESSED==1 && KEY2_PRESSED==1){
			 s_delay=0;
			 timer=0;
			 mode=PAUSETIME_SETTING;
			 oled_blank();
    load_pause_digits(pause_time);
    display_pause_time();
    last_blink_tick = msTicks;

			 
		 }
	 }
		 break;
	 case PAUSETIME_SETTING:
		 	cur_val = msTicks;

    // Toggle blink every 500 ms
    if(cur_val - last_blink_tick >= 500)
    {
        blink_state = !blink_state;
        last_blink_tick = cur_val;
        display_pause_time();
    }
    // KEY1 ? Increase digit
    if(KEY1_PRESSED == 0)
    {
        Delay(150);
        if(KEY1_PRESSED == 0)
        {
            increment_pt_digit();
					s_delay=0;
        }
    }

    // KEY2 ? Move cursor to next digit
    if(KEY2_PRESSED == 0)
    {
        Delay(150);
        if(KEY2_PRESSED == 0)
        {
            next_pt_digit();
					s_delay=0;
        }
    }
		if(KEY1_PRESSED==1 && KEY2_PRESSED==1 && s_delay>3){
					if(KEY1_PRESSED==1 && KEY2_PRESSED==1){
						pause_time = get_pause_minutes();
						exit_setting_mode();
						if(run_mode==PROGRESSIVE){
							p_cycle=cycle;
							p_pausetime=pause_time;
							
						}
						if(run_mode==SINGLE_LINE){
							s_cycle=cycle;
							s_pausetime=pause_time;
							
						}
						old_h = 255;
						old_m = 255;
						colon_drawn = 0;
						cycle_completed=0;
						flashWrite();
						mode=RUN_TIME;
					}}
		 
		 break;
	 /*case PAUSETIME_SETTING:
	
		 if(KEY1_PRESSED==0){
			 Delay(50);
			 if(KEY1_PRESSED==0){
				 s_delay=0;
				 pause_time++;
				 if(pause_time>=5999){
							pause_time=1;
						}
					}
				}
		 if(KEY2_PRESSED==0){
			 Delay(50);
			 if(KEY2_PRESSED==0){
				 s_delay=0;
				 pause_time--;
				 if(pause_time<=0){
							pause_time=5999;
						}
					}
				}

				update_time_display(pause_time);
				if(KEY1_PRESSED==1 && KEY2_PRESSED==1 && s_delay>3){
					if(KEY1_PRESSED==1 && KEY2_PRESSED==1){
						exit_setting_mode();
						if(run_mode==PROGRESSIVE){
							p_cycle=cycle;
							p_pausetime=pause_time;
							
						}
						if(run_mode==SINGLE_LINE){
							s_cycle=cycle;
							s_pausetime=pause_time;
							
						}
						old_h = 255;
						old_m = 255;
						colon_drawn = 0;
						cycle_completed=0;
						flashWrite();
						mode=RUN_TIME;
					}
				}		 
		 break;
	 case ML:
		 if(KEY2_PRESSED==0){
			 Delay(150);
			 if(KEY2_PRESSED==0){
				 s_delay=0;
				 set_ml++;
			 }
		 }
		 if(set_ml>3){
			 set_ml=1;}
		 
		 if(set_ml==1){
			 print_string("ML-125  ",2,1,8);}
		 if(set_ml==2){
			 print_string("ML-250  ",2,1,8);}
		 if(set_ml==3){
			 print_string("ML-380  ",2,1,8);}
		 
		 if(KEY1_PRESSED==0){
			 Delay(150);
			 if(KEY1_PRESSED==0){
				 s_delay=0;
				 if(set_ml==1){
					 sel_ml=125;}
				 if(set_ml==2){
					 sel_ml=250;}
				 if(set_ml==3){
					 sel_ml=380;}
				 mode =(set_mode==3)? SET_MONTHS:SET_WEEKS;
			 }
		 }
		 if(KEY1_PRESSED==1 && KEY2_PRESSED==1 && s_delay>2){
			 if(KEY1_PRESSED==1 && KEY2_PRESSED==1){
				 s_delay=0;
				 timer=0;
				 possible_cycle=sel_ml/0.2;
				 mode =(set_mode==3)? SET_MONTHS:SET_WEEKS;
			 }
		 }
		 break;
	 case SET_MONTHS:
		 if(KEY1_PRESSED==0){
			 Delay(150);
			 if(KEY1_PRESSED==0){
				 s_delay=0;
				 month++;
				 if(month>36){
				 month=36;}
			 }
		 }
		 if(KEY2_PRESSED==0){
			 Delay(150);
			 if(KEY2_PRESSED==0){
				 s_delay=0;
				 month--;
			 }
		 }
		 if(month>0xFFFF || month==0){
					month=01;}
		 
		 sprintf(dis_str, "MONTH-%02d", month);
				print_string(dis_str, 2, 1, 8);
			if(KEY1_PRESSED==1 && KEY2_PRESSED==1 && s_delay>2){
					if(KEY1_PRESSED==1 && KEY2_PRESSED==1){
						total_minute=(month*30)*1440;
						every_min=total_minute/possible_cycle;
						every_min=(int)(every_min+0.5);
						m_timer=0;
						exit_setting_mode();
						cycle=1;
						cycle_completed=0;
						flashWrite();
						mode=RUN_TIME;
					}
				}		
		 
		 break;
	 case SET_WEEKS:
		 if(KEY1_PRESSED==0){
			 Delay(150);
			 if(KEY1_PRESSED==0){
				 s_delay=0;
				 weeks++;
				 if(weeks>48){
							weeks=48;}
			 }
		 }
		 if(KEY2_PRESSED==0){
			 Delay(150);
			 if(KEY2_PRESSED==0){
				 s_delay=0;
				 weeks--;
			 }
		 }
		 if(weeks>0xFFFF || weeks==0){
					weeks=01;}
		 
		 sprintf(dis_str, "WEEK-%02d ", weeks);
					print_string(dis_str, 2, 1, 8);
					
				if(KEY1_PRESSED==1 && KEY2_PRESSED==1 && s_delay>2){
					if(KEY1_PRESSED==1 && KEY2_PRESSED==1){
						total_minute=(weeks*7)*1440;
						every_min=total_minute/possible_cycle;
						every_min=(int)(every_min+0.5);
						m_timer=0;
						exit_setting_mode();
						cycle=1;
						cycle_completed=0;
						flashWrite();
						mode=RUN_TIME;
						run_mode=PROGRESSIVE;
					}
				}	
		 
		 break;
	 case RPM:
		 if(KEY1_PRESSED==0){
					Delay(150);
					if(KEY1_PRESSED==0){
						s_delay=0;
						duty_cycle=duty_cycle+5;
						if(duty_cycle>=100){
							duty_cycle=100;
						}
					}
				}
				if(KEY2_PRESSED==0){
					Delay(150);
					if(KEY2_PRESSED==0){
						s_delay=0;
						duty_cycle=duty_cycle-5;
						if(duty_cycle<5){
							duty_cycle=0;
						}
					}
				}
		
					sprintf(dis_str, "RPM:%03d%%", duty_cycle);
           print_string(dis_str, 2, 1, 8);

				
				if(duty_cycle>0xFF || duty_cycle==0){
				duty_cycle=00;
				}
				if(KEY1_PRESSED==1 && KEY2_PRESSED==1 && s_delay>2){
					if(KEY1_PRESSED==1 && KEY2_PRESSED==1){
						exit_setting_mode();
						dig=((float)duty_cycle/100);
						speed=dig*240;
						flashWrite();
						mode=RUN_TIME;
						cycle_completed=0;
						break;
					}
				}
		 break;*/
	 case FAULT:
		 MOT_LED_OFF;
			if(LOW_LVL_FLAG==true){
				low_lvl_indic();
			}
			if(TRIP_FLAG==true){
				itrip_indic();
			}
			if(KEY2_PRESSED==0 && TRIP_FLAG==true){
				Delay(150);
				if(KEY2_PRESSED==0){
					PLC_OUT_OFF;
					Delay(200);
					FAULT_LED_OFF;
					PLC_OUT_ON;
					cycle_completed=0;
					exit_setting_mode();
					mode=RUN_TIME;
					TRIP_FLAG=false;
				}
			}
     if(KEY2_PRESSED==0 && LOW_LVL_FLAG==true){
			 Delay(150);
			 if(KEY2_PRESSED==0){
				 FAULT_LED_OFF;
				 cycle_completed=0;
				 exit_setting_mode();
				 mode=RUN_TIME;
				 LOW_LVL_FLAG=false;
			 }
		 }
		 Delay(1500);
		 break;
	 
	 
 }
 }
}



void low_lvl_indic(void){
	LOW_LVL_FLAG=true;
	FAULT_LED_ON;
print_string("LOW LVL",2,1,8);	
	}

void itrip_indic(void){
	TRIP_FLAG=true;
	FAULT_LED_ON;
	print_string("OVR LOAD",2,1,8);	
	//LED3_OFF;
}
void led_blink(void){
	HB_LED_OFF;
	Delay(200);
	HB_LED_ON;
	Delay(200);
	HB_LED_OFF;
	Delay(200);
	HB_LED_ON;
	/*if(set_mode==SINGLE_LINE){
		MOD_LED_ON;
		Delay(200);
		MOD_LED_OFF;
		Delay(200);
	}
	if(set_mode==PROGRESSIVE){
		MOD_LED_ON;
		Delay(200);
		MOD_LED_OFF;
		Delay(200);
		MOD_LED_ON;
		Delay(200);
		MOD_LED_OFF;
		Delay(200);
	}
	if(set_mode==MONTH){
		MOD_LED_ON;
		Delay(200);
		MOD_LED_OFF;
		Delay(200);
		MOD_LED_ON;
		Delay(200);
		MOD_LED_OFF;
		Delay(200);
		MOD_LED_ON;
		Delay(200);
		MOD_LED_OFF;
		Delay(200);
	}
	if(set_mode==WEEK){
		MOD_LED_ON;
		Delay(200);
		MOD_LED_OFF;
		Delay(200);
		MOD_LED_ON;
		Delay(200);
		MOD_LED_OFF;
		Delay(200);
		MOD_LED_ON;
		Delay(200);
		MOD_LED_OFF;
		Delay(200);
		MOD_LED_ON;
		Delay(200);
		MOD_LED_OFF;
		Delay(200);
	}*/
		
}
void update_screen(void){
					RemainingSeconds = cycle - cycle_completed;
    // Format string safely
    sprintf(dis_str, "CYCLE-%02d", RemainingSeconds);
    print_string(dis_str, 2, 1, 8);
}


void exit_setting_mode(void) {
    s_delay = 0;
    timer = 0;

    if (HOM_STATUS == 0) {
        ramp_up(REVERSE, speed);
        oled_blank();
        draw_icon(hom, 0, 0, 32, 4);
        while (HOM_STATUS == 0) {
            // wait for homing
        }
        oled_blank();
        ramp_down(REVERSE, speed);
				Delay(200);
        //motor_break();
    }

    if (cycle > 50 || cycle == 0) {
        cycle = 1;
    }
	}

void present_mode(void){
		if(set_mode==SINGLE_LINE){
			print_string("SNG LINE",2,1,8);
		}
		if(set_mode==PROGRESSIVE){
			print_string("PROGSIVE",2,1,8);
		}
		if(set_mode==MONTH){
			print_string("MONTH",2,1,8);
		}
		if(set_mode==WEEK){
			print_string("WEEK    ",2,1,8);
		}
	}
void flashWrite(void){ 

    memory.s_cycle    	= s_cycle;
    memory.s_pausetime 	= s_pausetime;
	  memory.p_cycle      = p_cycle;
	  memory.p_pausetime  = p_pausetime;
	  memory.prev_mode    =set_mode;

	 // memory.count=count;
  
    FLASH_EraseSector(0XF000);
	
    FLASH_Program1LongWord(0XF000,memory.s_cycle);
		FLASH_Program1LongWord(0XF004,memory.s_pausetime);
		FLASH_Program1LongWord(0XF00c,memory.p_cycle);
		FLASH_Program1LongWord(0XF010,memory.p_pausetime);
		FLASH_Program1LongWord(0XF018,memory.prev_mode);
	
}	

void flashRead(void){ 
	uint32_t* pt1    = (uint32_t*)0XF000;
	uint32_t* pt2    = (uint32_t*)0XF004;
	uint32_t* pt3    = (uint32_t*)0XF00C;
  uint32_t* pt4    = (uint32_t*)0XF010;
	uint32_t* pt5    = (uint32_t*)0XF018;
	
	memory.s_cycle       	= *pt1;   //cycle
	memory.s_pausetime 	  = *pt2;   // pause_time
	memory.p_cycle        = *pt3; //
  memory.p_pausetime    = *pt4;
	memory.prev_mode      =*pt5;

	if (memory.s_cycle >= 51 || memory.s_cycle<=0)
	{
		memory.s_cycle = 1;
	}
	if (memory.p_cycle >= 51 || memory.p_cycle<=0)
	{
		memory.p_cycle = 1;
	}
	if (memory.s_pausetime >= 6039 || memory.s_pausetime<=0)
	{
		memory.s_pausetime = 1;
	}
	if (memory.p_pausetime >= 6039 || memory.p_pausetime<=0)
	{
		memory.p_pausetime = 1;
	}
	
	if(memory.prev_mode>=5 || memory.prev_mode<=0){
		  memory.prev_mode=2;}
	
			if(memory.prev_mode==SINGLE_LINE){
     cycle     = memory.s_cycle;
     pause_time = memory.s_pausetime;}
			
			if(memory.prev_mode==PROGRESSIVE){
		 cycle     = memory.p_cycle;
     pause_time = memory.p_pausetime;}
			
		 s_cycle = memory.s_cycle;
		 p_cycle = memory.p_cycle;
		 
		 s_pausetime=memory.s_pausetime;
		 p_pausetime=memory.p_pausetime;
		 
	   run_mode  = memory.prev_mode;
	   set_mode  = memory.prev_mode;
}


/*void update_time_display(uint16_t counter)
{
	
 
	 char buf[4];

    uint8_t hours   = counter / 60;
    uint8_t minutes = counter % 60;
     called++;
    // ----- HH -----
    if (hours != old_h)
    {
        oled_clear_area(HH_X, P_Y, 32);      // clear 2 chars (16px*2)
        sprintf(buf, "%02d", hours);
        print_string(buf, HH_X, P_Y, 2);
        old_h = hours;
    }

    // ----- MM -----
    if (minutes != old_m)
    {
        oled_clear_area(MM_X, P_Y, 32);      // clear 2 chars
			  print_string(":", COLON_X, P_Y, 1);
        sprintf(buf, "%02d", minutes);
        print_string(buf, MM_X, P_Y, 2);
        old_m = minutes;
    }

    // Draw colon only once
    /*if (!colon_drawn)
    {
        print_string(":", COLON_X, P_Y, 1);
        colon_drawn = 1;
    }
}*/


void update_PT_display(uint32_t totalSeconds, uint8_t mode)
{

    uint8_t hours   = totalSeconds / 3600;
    uint8_t minutes = (totalSeconds % 3600) / 60;
    uint8_t seconds = totalSeconds % 60;

    char buf[4];

    
    if (mode != last_mode)
    {
        oled_clear_area(HH_X1, P_Y1, 50);   // clear HH : MM or MM : SS
        colon_drawn = 0;
        old_h = old_m = old_s = 255;
        last_mode = mode;
    }

    if (mode == 1)
    {
        // Hours changed?
        if (hours != old_h)
        {
            oled_clear_area(HH_X1, P_Y1, 2 * CHAR_W);
				//	print_string(":", COLON_X1, P_Y1, 1);
            sprintf(buf, "%02d", hours);
            print_string(buf, HH_X1, P_Y1, 2);
            old_h = hours;
        }

        // Minutes changed?
        if (minutes != old_m)
        {
            oled_clear_area(MM_X1, P_Y1, 2 * CHAR_W);
					print_string(":", COLON_X1, P_Y1, 1);
            sprintf(buf, "%02d", minutes);
            print_string(buf, MM_X1, P_Y1, 2);
            old_m = minutes;
        }
    }

    else
    {
        // Minutes changed?
        if (minutes != old_m)
        {
            oled_clear_area(HH_X1, P_Y1, 2 * CHAR_W);
				//	print_string(":", COLON_X1, P_Y1, 1);
            sprintf(buf, "%02d", minutes);
            print_string(buf, HH_X1, P_Y1, 2);
            old_m = minutes;
        }

        // Seconds changed?
        if (seconds != old_s)
        {
            oled_clear_area(MM_X1, P_Y1, 2 * CHAR_W);
					print_string(":", COLON_X1, P_Y1, 1);
            sprintf(buf, "%02d", seconds);
            print_string(buf, MM_X1, P_Y1, 2);
            old_s = seconds;
        }
    }
    /*if (!colon_drawn)
    {
        print_string(":", COLON_X1, P_Y1, 1);
        colon_drawn = 1;
    }*/
}

void show_mode(char* mode_str, uint8_t mode_id) {
    if (last_shown_mode != mode_id) {
        oled_blank();
        print_string(mode_str, 2, 1, strlen(mode_str));
        last_shown_mode = mode_id;
    }
}

void load_pause_digits(uint16_t pause_time)
{
    uint16_t hrs = pause_time / 60;
    uint16_t mins = pause_time % 60;
		print_char('P', 2, 1);
    print_char(':', 16, 1);
    print_char(':', 65, 1);
	
    pt_digits[0] = hrs / 10;
    pt_digits[1] = hrs % 10;
    pt_digits[2] = mins / 10;
    pt_digits[3] = mins % 10;

    pt_index = 0;   // start at first digit
}
void display_pause_time(void)
{
    uint8_t y = 1;
    uint8_t x_positions[4] = {27,46,82,103};  
    int i;


    for(i = 0; i < 4; i++)
    {
        uint8_t xpos = x_positions[i];

        if(i == pt_index)   // selected digit
        {
            oled_clear_area(y, xpos, CHAR_W);

            if(blink_state)
            {
                print_char(pt_digits[i] + '0', xpos, y);
            }
        }
        else                // normal digits (always visible)
        {
            print_char(pt_digits[i] + '0', xpos, y);
        }
    }
}

uint32_t get_pause_minutes(void)
{
	   uint32_t hrs = (pt_digits[0] * 10) + pt_digits[1];
    uint32_t mins = (pt_digits[2] * 10) + pt_digits[3];

    return (hrs * 60) + mins;
}



void increment_pt_digit(void)
{
    pt_digits[pt_index]++;
    if(pt_digits[pt_index] > 9)
        pt_digits[pt_index] = 0;

    display_pause_time();
}
void next_pt_digit(void)
{
    pt_index++;
    if(pt_index > 3){
			pause_time = get_pause_minutes();
        pt_index = 0;
		}

    display_pause_time();
}

