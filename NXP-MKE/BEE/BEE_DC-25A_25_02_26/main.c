#include "MKE02Z4.h"                    // Device header
#include "main.h"
#include "i2c.h"
#include "flash.h"
#include "string.h"
#include "stdbool.h"
#include "password.h"
#include "stdio.h"
#include "display_support.h"
#include "ftm.h"


//variable for factory setting and password controlling

uint8_t digit = 0,enter_mode=0,fact_sel_mode=0,opt_mode=0;
uint8_t digit_active = 0;   // 0 = first press, 1 = increment mode
bool Pressure_release=false,last_cycle = false,LOW_LVL_FLAG=false,TRIP_FLAG=false,update_flag=false,plc_flag=false;
bool plc_stop_flag=false,sensor_flag=false,dualMode=false,mot_time_flag=false,mot_lmt_error_flag=false;

uint8_t plc=0,pre_fact_sel=0;
//low level variable

uint8_t low_lvl_cnt=0,low_lvl_lmt=0;
uint16_t low_lvl_cycles=100;
bool low_lvl_run=false,low_lvl_pause=false;

//display variables
uint8_t last_shown_mode = 0;   // invalid value to force first update
uint8_t flicker_state = 0,disp_mode=0;
char dis_str[10];
ModeConfig mode_table[] = {
    {"",        false}, // index 0 unused
    {"SP-SA",   false},
    {"MP-SA",   true},
    {"MPL-PLC", true},
    {"PR-SA",   false},
    {"PR-PLC",  false},
    {"SL-SA",   true},
    {"SL-PLC",  true}
};


//user setting controll variables and time
uint8_t stroke=0,stroke_completed=0,set_stroke=0,set_frmt=0;
uint64_t pause_time=0;
uint64_t PauseTime=0,RemainingSeconds=0,timer=0;
uint16_t speed=0,mot_timer=0,hours=0;
uint32_t s_delay=0;

SystemMode sel_mode = RUN_TIME;
Runmode run=NORMAL;

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
	GPIOA->PDDR|=(nsleep|FAULT_LED|RUN_LED|PAUSE_LED|PLC_OU1A|PLC_OU1B);

	GPIOA->PIDR&=~(KEY1|KEY2|HOM|LOW_LVL|ESTP|STP|FLT|PLC_IN);
	PORT->PUEL |=(KEY1|KEY2|HOM|LOW_LVL|ESTP|STP|FLT|PLC_IN);
	
	(void)FLASH_Init(10000000L); 					// Initialize  Flash
	SIM->SCGC |=SIM_SCGC_FLASH_MASK;      // Enable clock for Flash
	
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
	 RTC->SC |=RTC_SC_RTIF_MASK | RTC_SC_RTIE_MASK ;
	 timer++;
	key_delay++;
	s_delay++;
	if(mot_time_flag==true){
		mot_timer++;
		if(mot_timer>13){
			mot_lmt_error_flag=true;
	}}
}


int main(void){
	
	SystemCoreClockUpdate(); //get Core Clock Frequency
  SysTick_Config(SystemCoreClock/1000); //Generate interrupt each 1 ms
  NVIC_SetPriority(SysTick_IRQn, 2); //Set Int priority and enable Intr
	
	Delay(100);
	mcu_init();
	rtc_init();
	ftm_init();
	RUN_LED_ON;
	PAUSE_LED_ON;
	FAULT_LED_ON;
	i2c_int();
	oled_init();
  Delay(2);
	oled_blank();
	Delay(100);
	nsleep_ON;
	flashReadPassword();
	RUN_LED_OFF;
	PAUSE_LED_OFF;
	FAULT_LED_OFF;
	stroke=15;
	pause_time=2;
sel_mode=RUN_TIME;
fact_sel_mode=2;	
print_string("BEE-VX",2,1,6);
sensor_check();
if(HOM_STATUS==0){
	mot_time_flag=true;
	ramp_up(REVERSE,speed);
	mot_timer=0;
	mot_lmt_error_flag=false;
	oled_blank();
	draw_icon(hom,0,0,32,4);
	 while(HOM_STATUS==0){	
    if(mot_lmt_error_flag==true && HOM_STATUS==0){
    	sel_mode=FAULT;	
			break;
		}
	 }
		mot_timer=0;
	 mot_time_flag=false;
	 oled_blank();
		ramp_down(REVERSE,speed);
}
	while(1){
	
		 switch(sel_mode){
			 
        case FACTORY_SETTING:
					if(KEY1_PRESSED==0 && KEY2_PRESSED==0){  // still working
						Delay(150);
						if(KEY1_PRESSED==0 && KEY2_PRESSED==0){
							Display_control_On();
							print_string("Dual Mod",2,1,8);
							Delay(500);
							dualMode=true;
						fact_sel_mode=1;
						pause_time=99*60;
						sel_mode=RUN_TIME;
						Display_control_On();
							stroke_completed=0;
							exit_setting_mode();
						break;
					}
				}
				if(KEY1_PRESSED==0 && KEY2_PRESSED==1){
					Delay(150);
					if(KEY1_PRESSED==0 && KEY2_PRESSED==1){
						initialized=0;
						fact_sel_mode++;
						if(fact_sel_mode>7){
							fact_sel_mode=1;
						}
					}
				}
				
				if (fact_sel_mode >= 1 && fact_sel_mode <= 7) {
					show_mode_flicker(mode_table[fact_sel_mode].mode, FACTORY_SETTING);
					Pressure_release = mode_table[fact_sel_mode].pressure_release;
				}
				
				if(KEY2_PRESSED==0 && KEY1_PRESSED==1){
					Delay(150);
					if(KEY2_PRESSED==0 && KEY1_PRESSED==1){
						Display_control_On();
						if(fact_sel_mode==1||fact_sel_mode==4||fact_sel_mode==6){
							stroke=15;
							pause_time=99*60;   //hours
						}
						if(fact_sel_mode==2){
							stroke=9;
							pause_time=99*60;  //hours
						}
						if(fact_sel_mode==3|| fact_sel_mode==5||fact_sel_mode==7){
							stroke=15;
							pause_time=5;  //minutes
							plc=1;
							initialized=0;
							sel_mode=NPN_PNP_SEL;
							break;
						}
						dualMode=false;	
						sel_mode=RUN_TIME;
						Display_control_On();
						stroke_completed=0;
						exit_setting_mode();
						break;
							}
						}
            break;
						
						
						
						
				case PASSWORD:
					  if(KEY1_PRESSED == 0){  //digit entery
							Delay(80);
							if(KEY1_PRESSED == 0){
								show_cursor = 1;
								 if (!digit_active)
        {
            // First press ? just show 0
            display_password_masked();
            digit_active = 1;
        }
        else
        {
            // Next presses ? increment
            entered[cur_index]++;
            if (entered[cur_index] > 9)
                entered[cur_index] = 0;

            display_password_masked();
        }
							}}
						if(KEY2_PRESSED == 0){  //place selection or select the number
							Delay(80);
							if(KEY2_PRESSED == 0 &&cur_index<=4){
								show_cursor = 1;       // enable cursor
								shift_place();
							}}
						if (cur_index >= PASSWORD_LENGTH){
							display_password_masked();  // <– force redraw so 4th digit becomes *
							if(KEY1_PRESSED==0 && KEY2_PRESSED==0){
								Delay(80);
								if(KEY1_PRESSED==0 && KEY2_PRESSED==0){
									PW_Validation();
									display=0;

						}}}
						else{
							display_password_masked(); // Draw with cursor at new position
							}
					break;
							
							
				case NPN_PNP_SEL:
					if(KEY1_PRESSED==0){
						Delay(150);
						if(KEY1_PRESSED==0){
							initialized=0;
							plc++;
						}}
					if(plc>2){
						plc=1;
					}
					if(plc==1){
						show_mode_flicker("NPN",FACTORY_SETTING);
					}
					if(plc==2){
						show_mode_flicker("PNP",FACTORY_SETTING);
					}
					if(KEY2_PRESSED==0){
						Delay(150);
						if(KEY2_PRESSED==0){
							if (plc == 1) {
								PLC_OU1A_ON;
								PLC_OU1B_OFF;}
							else if (plc == 2) {
								PLC_OU1A_OFF;
								PLC_OU1B_ON;
							}
						dualMode=false;	
						sel_mode=RUN_TIME;
							run=PLC;
						Display_control_On();
						stroke_completed=0;
						exit_setting_mode();
						break;
						}
						
					}
					break;					
						
							
				case RUN_TIME:
				switch(run){
					
					case NORMAL:
						RUN_LED_ON;
				   PAUSE_LED_OFF;
						update_screen();
					if(stroke_completed < stroke){
						if(Pressure_release == true && stroke_completed == (stroke - 1)){
            last_cycle = true;
						}
						if(sensor_check()){
						break;
					}

        // --- Forward Motion ---
        ramp_up(FORWARD, speed);
					mot_time_flag=true;  // motor movement timer check start point
					mot_timer=0;
					mot_lmt_error_flag=false;

        // Loop for forward motion with continuous key check
        while((last_cycle ? STP_STATUS : ESTP_STATUS) == 0){
					if(PLC_IN_STATUS==1 && plc_flag==true){
						Delay(150);
						if(PLC_IN_STATUS==1){
							run=PLC;
							plc_flag=false;
							plc_stop_flag=true;
							break;
						}
					}
					if(mot_lmt_error_flag==true && HOM_STATUS==0){   // motor timer monitoring 
						sel_mode=FAULT;
						stroke_completed=stroke;
						break;
					}

					if (pw_reset_check()) {
						break;   // exit reverse motion immediately
						}
					if (fact_set_check()) {
						break;
					}

					if(check_key1_abort()){
						s_delay=0;
						break;
					}
					if(LOW_LVL_STATUS==0){
						Delay(150);
						if(LOW_LVL_STATUS==0){
							LOW_LVL_FLAG=true;
							FAULT_LED_ON;
							low_lvl_lmt=low_lvl_cycles/stroke;
							//print_string("LOW LVL",2,1,8);
							if(low_lvl_cnt>=low_lvl_lmt){
								print_string("END LVL ",2,1,8);
								stroke_completed=stroke;
								sel_mode=FAULT;
								break;}
						}
					}
					if(FLT_STATUS==0){
						TRIP_FLAG=true;
						itrip_indic();
						stroke_completed=stroke;
						sel_mode=FAULT;
						print_string("OVR LOAD",2,1,8);	

						break;
					}					
        }

        ramp_down(FORWARD, speed);
				mot_timer=0;
				mot_time_flag=0;
        Delay(1000);

        // --- Reverse Motion if not last cycle ---
				if(stroke_completed < stroke && plc_stop_flag==false){
        if(!last_cycle){
            ramp_up(REVERSE, speed);
					mot_time_flag=true;  // motor movement timer check start point
					mot_timer=0;
					mot_lmt_error_flag=false;
					
            while(HOM_STATUS == 0){
							if(PLC_IN_STATUS==1 && plc_flag==true){
						Delay(150);
						if(PLC_IN_STATUS==1){
							plc_flag=false;
							plc_stop_flag=true;
							run=PLC;
							break;
						}
					}
							if (pw_reset_check()) {
						break;   // exit reverse motion immediately
						}
					if (fact_set_check()) {
						break;
					}

					if(check_key1_abort()){
						s_delay=0;
						break;
					}
					if(LOW_LVL_STATUS==0){
						Delay(150);
						if(LOW_LVL_STATUS==0){
							LOW_LVL_FLAG=true;
							FAULT_LED_ON;
							low_lvl_lmt=low_lvl_cycles/stroke;
							//print_string("LOW LVL",2,1,8);
							if(low_lvl_cnt>=low_lvl_lmt){
								print_string("END LVL ",2,1,8);
								stroke_completed=stroke;
								sel_mode=FAULT;
								break;}
						}
					}
					if(FLT_STATUS==0){
						TRIP_FLAG=true;
						itrip_indic();
						stroke_completed=stroke;
						sel_mode=FAULT;
						print_string("OVR LOAD",2,1,8);	

						break;
					}
						}

            ramp_down(REVERSE, speed);
						mot_timer=0;
						mot_time_flag=0;
            Delay(1000);
        }

        stroke_completed++;
    }
	}
    if(sel_mode==PASSWORD){
			fact_disp();
		}
		if(sel_mode==CYCLE_SETTING || sel_mode== MODE_SETTING){
			set_disp();
		}
    // --- After all strokes ---
    if(stroke_completed == stroke &&sel_mode==RUN_TIME){
        sel_mode = PAUSE_TIME;
			if(LOW_LVL_FLAG==true){
					low_lvl_run= true;
				}
        timer = 0;
        oled_blank();
        print_string("PT:", 2, 1, 3);
        old_m = 255;
        old_h = 65535;
        old_s = 255;
    }

    break;
					case PLC:
						RUN_LED_OFF;
				   PAUSE_LED_OFF;
						update_screen();
						if(PLC_IN_STATUS==0){
							Delay(100);
							if(PLC_IN_STATUS==0){
								run=NORMAL;
								plc_flag=true;
								plc_stop_flag=false;
								break;
							}}
						if (pw_reset_check()) {
						break;   // exit reverse motion immediately
						}
					if (fact_set_check()) {
						break;
					}
						break;
	}	
				break;
					
	
	
				case PAUSE_TIME:
					PAUSE_LED_ON;
				RUN_LED_OFF;
					PauseTime=pause_time*60;
				RemainingSeconds=PauseTime-timer;
				hours = RemainingSeconds / 3600;
				if(hours >= 100){        // 3 digits
					disp_mode = 2;      // HHH
				}
				else if(RemainingSeconds >= 3600){
					disp_mode = 1;      // HH:MM
					}
				else{
					disp_mode = 0;      // MM:SS
				}
				update_PT_display(RemainingSeconds, disp_mode);
				if (pw_reset_check()) {
						break;   // exit reverse motion immediately
						}
					if (fact_set_check()) {
						fact_disp();
						break;
					}

					if(check_key1_abort()){
						s_delay=0;
						set_disp();
						break;
					}
					if(LOW_LVL_STATUS==0){
						Delay(150);
						if(LOW_LVL_STATUS==0){
							LOW_LVL_FLAG=true;
							FAULT_LED_ON;
							low_lvl_lmt=low_lvl_cycles/stroke;
							//print_string("LOW LVL",2,1,8);
							if(low_lvl_cnt>=low_lvl_lmt){
								print_string("END LVL ",2,1,8);
								stroke_completed=stroke;
								sel_mode=FAULT;
								break;}
						}
					}
					if(FLT_STATUS==0){
						TRIP_FLAG=true;
						itrip_indic();
						stroke_completed=stroke;
						sel_mode=FAULT;
					  print_string("OVR LOAD",2,1,8);	
		
						break;
					}

				if (timer >= PauseTime){
					if(LOW_LVL_FLAG==true && low_lvl_run==true){
					low_lvl_run= false;
					low_lvl_cnt++;
						if(low_lvl_cnt>=low_lvl_lmt){
								print_string("END LVL ",2,1,8);
								stroke_completed=stroke;
								sel_mode=FAULT;
								break;}
				}
					if(last_cycle==true){
						ramp_up(REVERSE, speed);
						while (HOM_STATUS == 0) {
							
						}
						ramp_down(REVERSE, speed);
						last_cycle=false;
					}
					if(plc_flag==true){
						run=PLC;
					}
					else{
						run=NORMAL;}
					sel_mode=RUN_TIME;
					stroke_completed=0;
						display=0;
				}
				
					break;
				
				case MODE_SETTING:
					PAUSE_LED_OFF;
				  RUN_LED_OFF;
					if(KEY1_PRESSED==0){
						Delay(80);
						if(KEY1_PRESSED==0){
							s_delay=0;
							initialized=0;
							fact_sel_mode++;
							if(fact_sel_mode>2){
								fact_sel_mode=1;}
						}
					}
					if (fact_sel_mode >= 1 && fact_sel_mode <= 2) {
					show_mode_flicker(mode_table[fact_sel_mode].mode, FACTORY_SETTING);
					Pressure_release = mode_table[fact_sel_mode].pressure_release;
				}
					if(KEY2_PRESSED==0){
						Delay(80);
						if(KEY2_PRESSED==0){
							sel_mode=CYCLE_SETTING;
							print_string("CYCLE-00", 2, 1, 8);
							break;
						}
					}
					if(KEY1_PRESSED==1 && KEY2_PRESSED==1 && s_delay>3){
						 if(KEY1_PRESSED==1 && KEY2_PRESSED==1){
							 stroke_completed=0;
							 display=0;
							 sel_mode=RUN_TIME;
							 exit_setting_mode();
							 s_delay=0;
						 }
					 }
					
					break;
				
				case CYCLE_SETTING:
					PAUSE_LED_OFF;
				  RUN_LED_OFF;
					if(KEY1_PRESSED==0){
						Delay(80);
						if(KEY1_PRESSED==0){
							update_flag=true;
							set_stroke++;
							s_delay=0;
							if(set_stroke>15){
							set_stroke=15;}
						}
					}
					if(KEY2_PRESSED==0){
						Delay(80);
						if(KEY2_PRESSED==0){
							update_flag=true;
							set_stroke--;
							s_delay=0;
						}
					}
					if(set_stroke>15 || set_stroke<=0){
						set_stroke=1;
					}
					if(update_flag==true){
						oled_clear_area(85, 1, 20);
						sprintf(dis_str, "%02d", set_stroke);
						print_string(dis_str, 85, 1, strlen(dis_str));
					update_flag=false;}
					
					if(KEY1_PRESSED==1 && KEY2_PRESSED==1 && s_delay>3){
						 if(KEY1_PRESSED==1 && KEY2_PRESSED==1){
							 stroke=set_stroke;
							 s_delay=0;
							 sel_mode=PAUSE_FRMT_SEL;
							 set_frmt=1;
							 /*oled_blank();
							 load_pause_digits(pause_time);
							 display_pause_time();
							 last_blink_tick = msTicks;*/
						 }
					 }
					
					break;
				case PAUSE_FRMT_SEL:
					if(KEY1_PRESSED==0){
						Delay(150);
						if(KEY1_PRESSED==0){
							s_delay=0;
							set_frmt++;
							if(set_frmt>=3){
								set_frmt=1;
							}
						}
					}
					if(set_frmt==2){
					print_string("HHH     ",2,1,8);}
					if(set_frmt==1){
					print_string("HH :MM  ",2,1,8);}
					if(KEY2_PRESSED==0){
						Delay(150);
						if(KEY2_PRESSED==0){
							s_delay=0;
							sel_mode=PAUSE_SETTING;
							oled_blank();
							load_pause_digits(pause_time);
							 if(set_frmt == 1){
							 display_pause_time();}
						 else if(set_frmt == 2){
							 display_pause_hours();}
							last_blink_tick = msTicks;
							
						}}

					break;
					 
					 
					 case PAUSE_SETTING:
						 cur_val = msTicks;
					 if(cur_val - last_blink_tick >= 500){
						 blink_state = !blink_state;
						 last_blink_tick = cur_val;
						 if(set_frmt == 1){
							 display_pause_time();}
						 else if(set_frmt == 2){
							 display_pause_hours();}
					 }
					 if(KEY1_PRESSED == 0){
						 Delay(80);
						 if(KEY1_PRESSED == 0){
							 increment_pt_digit();
							 s_delay=0;
						 }
					 }
					 if(KEY2_PRESSED == 0){
						 Delay(80);
						 if(KEY2_PRESSED == 0){
							 next_pt_digit();
							 s_delay=0;
						 }
					 }
					 if(KEY1_PRESSED==1 && KEY2_PRESSED==1 && s_delay>3){
						 if(KEY1_PRESSED==1 && KEY2_PRESSED==1 ){
							 pause_time = get_pause_minutes();
							 old_h = 65535;
							 old_m = 255;
							 stroke_completed=0;
							 display=0;
							 sel_mode=RUN_TIME;
							 exit_setting_mode();
							 break;
						 }
					 }
					 break;
					 
					 
					 
					 case FAULT:
						 PAUSE_LED_OFF;
					 RUN_LED_OFF;
					 if(sensor_flag==true){
						 sens_flt_indic();
					 }
					 if(mot_lmt_error_flag==true && HOM_STATUS==0){
						 print_string("PLCM FLT",2,1,8);
						 mot_lmt_error_flag=false;
					 }
						 
					 
						 if(LOW_LVL_FLAG==true){
							 low_lvl_indic();
						 }
						 if(TRIP_FLAG==true){
							 itrip_indic();
						 }
						 if(KEY2_PRESSED==0 && TRIP_FLAG==true){
							 Delay(150);
							 if(KEY2_PRESSED==0){
								 nsleep_OFF;
								 Delay(200);
								 FAULT_LED_OFF;
								 plc_fault_clr();
								 nsleep_ON;
								 stroke_completed=0;
							   display=0;
							   sel_mode=RUN_TIME;
								 TRIP_FLAG=false;
								 exit_setting_mode();
							 }
						 }
						 if(KEY2_PRESSED==0 && LOW_LVL_FLAG==true){
							 Delay(150);
							 if(KEY2_PRESSED==0){
								 FAULT_LED_OFF;
								 plc_fault_clr();
								 stroke_completed=0;
							   display=0;
								 if(plc_flag==true){
									 run=PLC;
								 }
								 else{
								 run=NORMAL;}
							   sel_mode=RUN_TIME;
								 LOW_LVL_FLAG=false;
								 exit_setting_mode();
							 }
						 }
						 
						 break;
    }
	}
	
}





