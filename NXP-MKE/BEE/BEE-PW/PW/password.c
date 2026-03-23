#include "password.h"
#include "flash.h"
#include "i2c.h"
#include "string.h"
#include "main.h"


/* Flash definitions */
#define PASSWORD_LENGTH 4
#define FLASH_PASSWORD_FLAG_ADDR   0xF020
#define FLASH_PASSWORD_DATA_ADDR   0xF024
#define PASSWORD_FLAG_VALUE        0xA5A5A5A5

/* Internal state */
uint8_t entered[PASSWORD_LENGTH] = {0,0,0,0};
uint8_t password_created = 0;   // 0 = create mode, 1 = verify mode
uint8_t cur_index = 0;
int j = 0;
uint8_t blink_state = 0;
uint8_t show_cursor = 0;   // 0 = show all *, 1 = show mask + cursor
uint8_t saved_password[4];
bool reset=false;



void Password_StartEntry(void)
{
	uint8_t i=0;
    for (i = 0; i < PASSWORD_LENGTH; i++)
        entered[i] = 0;

    cur_index   = 0;
    show_cursor = 0;
}


/* ----------- Internal helpers ----------- */

void PW_Validation(void){
										if (!password_created){
										flashWritePassword();
										oled_blank();
										flashReadPassword();
										print_string("PW SAVED", 2, 1, 8);
										sel_mode=RUN_TIME;
										Delay(1000);
										password_created = 1;
											reset=false;
									}
									else{
										if (entered[0] == saved_password[0] &&entered[1] == saved_password[1] &&
										entered[2] == saved_password[2] &&entered[3] == saved_password[3]){
											oled_blank();
											print_string("OK", 2, 1, 2);
										}
										else{
											oled_blank();
											print_string("WRONG", 2, 1, 5);
											reset=false;
										}
										Delay(1500);
									}
									oled_blank();
									enter_mode=0;
									cur_index = 0;
									if(reset==true){
										oled_blank();
										print_string("NEW PW", 2, 1, 6);
												sel_mode=PASSWORD;
										    password_created=0;
											}
										
									else{
									sel_mode=RUN_TIME;
									stroke_completed=0;
									}
}

void display_password_masked(void)
{
    uint8_t x = 2;
    uint8_t y = 1;

    for (j = 0; j < PASSWORD_LENGTH; j++)
    {

        if (show_cursor && j == cur_index)
        {
            // Show actual digit at cursor
            print_char(entered[j] + '0', x, y);
        }
        else
        {
					if(!enter_mode){
            // Mask others
            print_char('*', x, y);}
        }

        x += 21;   // spacing to next digit
    }
		enter_mode=0;
}



void updat_password(int d)
{
    if (d >= 9) 
			d = 0;

    entered[cur_index] = (uint8_t)d;

    display_password_masked();
}


void shift_place(void)
{
    cur_index++;
}


void flashReadPassword(void)
{
    uint32_t flag  = *(uint32_t*)FLASH_PASSWORD_FLAG_ADDR;
    uint32_t value = *(uint32_t*)FLASH_PASSWORD_DATA_ADDR;

    if (flag != PASSWORD_FLAG_VALUE)
    {
        password_created = 0; // no password
        return;
    }

    password_created = 1; // password stored

    saved_password[0] = (value >> 24) & 0xFF;
    saved_password[1] = (value >> 16) & 0xFF;
    saved_password[2] = (value >> 8 ) & 0xFF;
    saved_password[3] =  value        & 0xFF;
}

void flashWritePassword(void)
{
    uint32_t packed = (entered[0] << 24) |
                      (entered[1] << 16) |
                      (entered[2] << 8 ) |
                       entered[3];
	  FLASH_EraseSector(FLASH_PASSWORD_DATA_ADDR);
	  FLASH_Program1LongWord(FLASH_PASSWORD_DATA_ADDR, packed);
    FLASH_Program1LongWord(FLASH_PASSWORD_FLAG_ADDR, PASSWORD_FLAG_VALUE);
   
}
