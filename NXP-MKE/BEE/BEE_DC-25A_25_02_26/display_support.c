#include "display_support.h"
#include "i2c.h"
#include "string.h"
#include "stdio.h"
#include "MKE02Z4.h"
#include "main.h"
#include "password.h"
#include "ftm.h"

/* -------- Display state -------- */
static uint8_t last_shown_mode = 0xFF;
uint8_t initialized = 0;
uint8_t display=0;

uint8_t pt_digits[4] = {0,0,0,0};
uint8_t pt_index = 0;

uint8_t blink_state1 = 1;
uint32_t last_blink_tick = 0;

uint32_t cur_val = 0;

uint16_t key_delay=0;
uint16_t old_h = 65535;
uint8_t old_m = 255;
uint8_t old_s = 255;
uint8_t last_mode = 255;

/* External time base */
extern volatile uint32_t msTicks;

/* -------- Display functions -------- */

void show_mode(char *mode_str, uint8_t mode_id)
{
    if (last_shown_mode != mode_id)
    {
        oled_blank();
        print_string(mode_str, 2, 1, strlen(mode_str));
        last_shown_mode = mode_id;
    }
}

void show_mode_flicker(char *mode_str, uint8_t mode_id)
{
      static uint32_t last_tick = 0;
    static uint8_t flicker_state = 1; // 1 = ON, 0 = OFF
	uint32_t interval=0;

    /* Draw once */
    if (!initialized)
    {
        oled_blank();
        print_string(mode_str, 2, 1, strlen(mode_str));
        Display_control_On();
        flicker_state = 1;
        initialized = 1;
        last_tick = msTicks;
    }

    /* Flicker timing */
    interval = flicker_state ? 700 : 300;

    if (msTicks - last_tick >= interval)
    {
        last_tick = msTicks;
        flicker_state ^= 1;

        if (flicker_state)
            Display_control_On();
        else
            Display_control_Off();
    }
}

/* Call this when leaving FACTORY_SETTING */
void display_reset_flicker(void)
{
    initialized = 0;
    Display_control_On();
}

/* -------- Pause / Runtime display -------- */
void update_PT_display(uint32_t totalSeconds, uint8_t mode)
{
    uint16_t hours   = totalSeconds / 3600;
    uint8_t minutes  = (totalSeconds % 3600) / 60;
    uint8_t seconds  = totalSeconds % 60;

    char buf[5];

    if (mode != last_mode)
    {
        oled_clear_area(HH_X1, P_Y1, 60);
        old_h = 65535;
			  old_m = old_s = 255;
        last_mode = mode;
    }

    if (mode == 2)   /* HHH */
    {
        if (hours != old_h)
        {
            oled_clear_area(HH_X1, P_Y1, 3 * CHAR_W);
            sprintf(buf, "%03d H", hours);
            print_string(buf, HH_X1, P_Y1, 5);
            old_h = hours;
        }
    }

    else if (mode == 1)   /* HH:MM */
    {
        if (hours != old_h)
        {
            oled_clear_area(HH_X1, P_Y1, 2 * CHAR_W);
            sprintf(buf, "%02d", hours);
            print_string(buf, HH_X1, P_Y1, 2);
            old_h = hours;
        }

        if (minutes != old_m)
        {
            oled_clear_area(MM_X1, P_Y1, 2 * CHAR_W);
            print_string(":", COLON_X1, P_Y1, 1);
            sprintf(buf, "%02d", minutes);
            print_string(buf, MM_X1, P_Y1, 2);
            old_m = minutes;
        }
    }

    else            /* MM:SS */
    {
        if (minutes != old_m)
        {
            oled_clear_area(HH_X1, P_Y1, 2 * CHAR_W);
            sprintf(buf, "%02d", minutes);
            print_string(buf, HH_X1, P_Y1, 2);
            old_m = minutes;
        }

        if (seconds != old_s)
        {
            oled_clear_area(MM_X1, P_Y1, 2 * CHAR_W);
            print_string(":", COLON_X1, P_Y1, 1);
            sprintf(buf, "%02d", seconds);
            print_string(buf, MM_X1, P_Y1, 2);
            old_s = seconds;
        }
    }
}
/*void update_PT_display(uint32_t totalSeconds, uint8_t mode)
{
    uint8_t hours   = totalSeconds / 3600;
    uint8_t minutes = (totalSeconds % 3600) / 60;
    uint8_t seconds = totalSeconds % 60;

    char buf[4];

    if (mode != last_mode)
    {
        oled_clear_area(HH_X1, P_Y1, 50);
        old_h = old_m = old_s = 255;
        last_mode = mode;
    }

    if (mode == 1)   // HH:MM 
    {
        if (hours != old_h)
        {
            oled_clear_area(HH_X1, P_Y1, 2 * CHAR_W);
            sprintf(buf, "%02d", hours);
            print_string(buf, HH_X1, P_Y1, 2);
            old_h = hours;
        }

        if (minutes != old_m)
        {
            oled_clear_area(MM_X1, P_Y1, 2 * CHAR_W);
            print_string(":", COLON_X1, P_Y1, 1);
            sprintf(buf, "%02d", minutes);
            print_string(buf, MM_X1, P_Y1, 2);
            old_m = minutes;
        }
    }
    else            // MM:SS 
    {
        if (minutes != old_m)
        {
            oled_clear_area(HH_X1, P_Y1, 2 * CHAR_W);
            sprintf(buf, "%02d", minutes);
            print_string(buf, HH_X1, P_Y1, 2);
            old_m = minutes;
        }

        if (seconds != old_s)
        {
            oled_clear_area(MM_X1, P_Y1, 2 * CHAR_W);
            print_string(":", COLON_X1, P_Y1, 1);
            sprintf(buf, "%02d", seconds);
            print_string(buf, MM_X1, P_Y1, 2);
            old_s = seconds;
        }
    }
}*/

void update_screen(void){
	char dis_str[10];
		uint8_t	RemainingStrokes = stroke - stroke_completed;
	if(display==0){
		oled_blank();
	  print_string("CYCLE-", 2, 1, 6);
		display=1;
	}
    sprintf(dis_str, "%02d", RemainingStrokes);
    print_string(dis_str, 85, 1, 2);
				if(stroke==stroke_completed+1){
				display=0;
			}

}
void low_lvl_indic(void){
	LOW_LVL_FLAG=true;
  FAULT_LED_ON;
	fault_plc_action();
  Delay(500);
  FAULT_LED_OFF;
	PLC_OU1A_OFF;
	PLC_OU1B_OFF;
	Delay(500);
	}
void sens_flt_indic(void){
  FAULT_LED_ON;
	fault_plc_action();
  Delay(150);
  FAULT_LED_OFF;
	PLC_OU1A_OFF;
	PLC_OU1B_OFF;
	Delay(150);
	}
	
void itrip_indic(void){
	TRIP_FLAG=true;
	FAULT_LED_ON;
	fault_plc_action();
}

void fault_plc_action(void)
{
    if (plc == 1) {
        PLC_OU1A_OFF;
        PLC_OU1B_ON;
    }
    else if (plc == 2) {
        PLC_OU1A_ON;
        PLC_OU1B_OFF;
    }
}

void plc_fault_clr(void)
{
    if (plc == 1) {
        PLC_OU1A_ON;
        PLC_OU1B_OFF;
    }
    else if (plc == 2) {
        PLC_OU1A_OFF;
        PLC_OU1B_ON;
    }
}



void display_current_values(void)
{
    char dis_str[20];
	  uint32_t h = pause_time / 60;
    uint32_t m = (pause_time % 60) ;
	print_string(mode_table[fact_sel_mode].mode, 2, 1, 6);
	Delay(100);
	sprintf(dis_str, "CYCLE-%02d", stroke);
	print_string(dis_str, 2, 1, 8);
	Delay(500);
	if(set_frmt == 1){   // HH:MM
		sprintf(dis_str, "PT:%02ld:%02ld", h, m);}
	else if(set_frmt == 2){  // HHH
		sprintf(dis_str, "PT:%03ld", h);}
	print_string(dis_str, 2, 1, 8);
		Delay(500);
		if(sel_mode == PAUSE_TIME){
    print_string("PT:", 2, 1, 3);

    old_h = 65535;
    old_m = 255;
    old_s = 255;
}
}

bool check_key1_abort(void)
{
    if (KEY2_PRESSED == 0 && KEY1_PRESSED!=0)
    {
        Delay(150);
        if (KEY2_PRESSED == 0) {
					key_delay=0;
					 while (KEY2_PRESSED == 0 && key_delay<4) {
                if (ESTP_STATUS==1 ||STP_STATUS==1 || (HOM_STATUS == 1 && sel_mode!=PAUSE_TIME)) {
                    break;
                }
            }
					if(key_delay>=2 && fact_sel_mode != 3 && fact_sel_mode != 5 && fact_sel_mode != 7){
						stroke_completed = stroke;
						if(dualMode==true){
							sel_mode=MODE_SETTING;
							fact_sel_mode=1;
							initialized=0;
							s_delay=0;
						}
						else{
							sel_mode = CYCLE_SETTING;
							s_delay=0;
						}
            return true;
						
					}
					else{
						if(sel_mode!=RUN_TIME){
						oled_blank();
						display_current_values();
						Delay(200);
						display=0;
						//update_screen();
						}
						return false;

					}

        }
    }
    return false;
}
uint8_t sensor_check(void){
	if ((HOM_STATUS && STP_STATUS) ||
    (HOM_STATUS && ESTP_STATUS) ||
    (STP_STATUS && ESTP_STATUS)) {
			sensor_flag=true;
			print_string("SENS FLT",2,1,8);
    sel_mode = FAULT;
			
			return 1;
}
		return 0;
}

uint8_t pw_reset_check(void)
{
    if (KEY1_PRESSED == 0 && KEY2_PRESSED == 0) {

        Delay(1000);

        if (KEY1_PRESSED == 0 && KEY2_PRESSED == 0) {

            stroke_completed = stroke;
            sel_mode = PASSWORD;
            reset = true;

            return 1;   // PW reset triggered
        }
    }

    return 0;   // No action
}

uint8_t fact_set_check(void)
{
    if (KEY1_PRESSED == 0 && KEY2_PRESSED != 0) {

        Delay(2000);

        if (KEY1_PRESSED == 0 && KEY2_PRESSED != 0) {

            stroke_completed = stroke;
            sel_mode = PASSWORD;
            pre_fact_sel=fact_sel_mode;
            return 1;   // Factory setting triggered
        }
    }

    return 0;   // No action
}

void fact_disp(void){
	
	if(reset==false){
				initialized = 0;
			oled_blank();
			show_mode("FACT SET", FACTORY_SETTING);
			Delay(200);
	    oled_blank();
			show_mode("PW", PASSWORD);
			Delay(200);
			oled_blank();}
			
	else{
	    oled_blank();
	    show_mode("PW RESET", NONE);
      Delay(200);
      oled_blank();
	    show_mode("OLD PW", PASSWORD);
      Delay(200);
	    oled_blank();}
	
      digit_active = 0;
      Password_StartEntry();
      display_password_masked();
			enter_mode = 1;
}
	
void set_disp(void){
	RUN_LED_OFF;
	PAUSE_LED_OFF;
	oled_blank();
	print_string("SETTING",2,1,7);
	Delay(300);
	if(dualMode==true){
		sel_mode=MODE_SETTING;
		fact_sel_mode=1;
		initialized=0;
		s_delay=0;
	}
	else{
		print_string("CYCLE-00", 2, 1, 8);
		sel_mode = CYCLE_SETTING;
		s_delay=0;
	}
}


void load_pause_digits(uint16_t pause_time)
{
    uint16_t hrs = pause_time / 60;
    uint16_t mins = pause_time % 60;
	  if(set_frmt==1){
		print_char('P', 2, 1);
    print_char(':', 16, 1);
    print_char(':', 65, 1);}
		if(set_frmt==2){
			print_char('P', 2, 1);
    print_char(':', 20, 1);
		}
	
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

            if(blink_state1)
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

void display_pause_hours(void)
{
    uint8_t y = 1;
    uint8_t x_positions[3] = {40,60,80};  
    int i;

    for(i = 0; i < 3; i++)
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
        else
        {
            print_char(pt_digits[i] + '0', xpos, y);
        }
    }
}

uint32_t get_pause_minutes(void)
{
	uint32_t hrs = 0;
	uint32_t mins = 0;
	  if(set_frmt == 1) // HH:MM
    {
        hrs  = (pt_digits[0] * 10) + pt_digits[1];
        mins = (pt_digits[2] * 10) + pt_digits[3];
    }
    else if(set_frmt == 2) // HHH
    {
        hrs = (pt_digits[0] * 100) + (pt_digits[1] * 10) + pt_digits[2];
        mins = 0;
    }

    return (hrs * 60) + mins;
}



void increment_pt_digit(void)
{
    pt_digits[pt_index]++;
    if(pt_digits[pt_index] > 9)
        pt_digits[pt_index] = 0;

    if(set_frmt == 1){
		display_pause_time();}
		else if(set_frmt == 2){
		display_pause_hours();}
}
void next_pt_digit(void)
{
    pt_index++;
    if(pt_index > 3){
			pause_time = get_pause_minutes();
        pt_index = 0;
		}

    if(set_frmt == 1){
		display_pause_time();}
		else if(set_frmt == 2){
		display_pause_hours();}
}


void exit_setting_mode(void) {
    s_delay = 0;
    timer = 0;

    if (HOM_STATUS == 0) {
        mot_time_flag=true;
			ramp_up(REVERSE,speed);
			mot_timer=0;
			mot_lmt_error_flag=false;
			oled_blank();
			draw_icon(hom, 0, 0, 32, 4);
			while (HOM_STATUS == 0) {
				 if(mot_lmt_error_flag==true && HOM_STATUS==0){
    	sel_mode=FAULT;	
			break;
		}
			}
			mot_timer=0;
			mot_time_flag=false;
			oled_blank();
			ramp_down(REVERSE, speed);
			Delay(200);
    }

    if (stroke > 50 || stroke == 0) {
        stroke = 1;
    }
	}

