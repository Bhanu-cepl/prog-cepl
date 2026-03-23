#ifndef DISPLAY_SUPPORT_H
#define DISPLAY_SUPPORT_H

#include <stdint.h>
#include "stdbool.h"

#define CHAR_W         22       // width of one character
#define CHAR_H_PAGES    3       // each character is 3 pages tall

#define P_X1             2
#define P_Y1             1
#define HH_X1            46  //(P_X1 + 2*(CHAR_W))        // after "P:"
#define COLON_X1         76  //(HH_X1 + CHAR_W+8)       // after HH
#define MM_X1            96  //(COLON_X1 + 2*(CHAR_W-12))      // after ":"


extern uint8_t initialized;
extern uint8_t old_h;
extern uint8_t old_s;
extern uint8_t old_m;
extern uint16_t key_delay;
extern uint8_t pt_digits[4];
extern uint8_t pt_index;

extern uint8_t blink_state;
extern uint32_t last_blink_tick;

extern uint32_t cur_val;
extern uint8_t  display;


/* Public display APIs */
void show_mode(char *mode_str, uint8_t mode_id);
void show_mode_flicker(char *mode_str, uint8_t mode_id);
void update_PT_display(uint32_t totalSeconds, uint8_t mode);
void update_screen(void);
void low_lvl_indic(void);
void itrip_indic(void);
bool check_key1_abort(void);
void next_pt_digit(void);
void increment_pt_digit(void);
uint32_t get_pause_minutes(void);
void display_pause_time(void);
void load_pause_digits(uint16_t pause_time);
uint8_t pw_reset_check(void);
uint8_t fact_set_check(void);
void fault_plc_action(void);
void plc_fault_clr(void);
void exit_setting_mode(void);


/* Optional helper */
void display_reset_flicker(void);

#endif
