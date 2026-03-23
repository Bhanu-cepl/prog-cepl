#ifndef PASSWORD_H
#define PASSWORD_H

#include <stdint.h>
#include <stdbool.h>

#define PASSWORD_LENGTH 4

/* Public variables */
extern uint8_t entered[PASSWORD_LENGTH];
extern uint8_t saved_password[PASSWORD_LENGTH];
extern uint8_t password_created;
extern uint8_t cur_index;
extern uint8_t show_cursor;
extern bool reset;

/* Public APIs */
void PW_Validation(void);
void Password_StartEntry(void);
void display_password_masked(void);
void updat_password(int d);
void shift_place(void);
void flashReadPassword(void);
void flashWritePassword(void);
void run_check(void);

#endif
