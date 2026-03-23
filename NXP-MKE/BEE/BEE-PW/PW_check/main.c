#include "MKE02Z4.h"                    // Device header
#include "main.h"
#include "i2c.h"
#include "flash.h"
#include "string.h"
#include "stdbool.h"

#define PASSWORD_LENGTH 4
#define FLASH_PASSWORD_FLAG_ADDR   0xF020
#define FLASH_PASSWORD_DATA_ADDR   0xF024
#define PASSWORD_FLAG_VALUE        0xA5A5A5A5

/* Password buffers */
uint8_t entered[PASSWORD_LENGTH] = {0,0,0,0};
uint8_t password_created = 0;   // 0 = create mode, 1 = verify mode
uint8_t cur_index = 0;
uint8_t digit = 0,enter_mode=0,sel_mode=0;
int j = 0;
uint8_t blink_state = 0;
uint8_t show_cursor = 0;   // 0 = show all *, 1 = show mask + cursor
uint8_t saved_password[4];
uint8_t last_shown_mode = 0;   // invalid value to force first update

uint8_t digit_active = 0;
uint8_t fact_sel_mode=0;

bool dualMode=false;
char* mode[7]={"SPL","MP-SA","MP-PLC","PR-SA","PR-PLC","SL-SA","SL-PLC"};
uint8_t dual_mode[2];

typedef enum {   //mode variables for switching
    MODE_DEFAULT = 1,
    MODE_1,
    MODE_2,
    MODE_3,
	  PASSWORD
}SystemMode;

SystemMode current_mode = MODE_DEFAULT;


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

	GPIOA->PIDR&=~(KEY1|KEY2);
	PORT->PUEL |=(KEY1|KEY2);
	
	(void)FLASH_Init(10000000L); 					// Initialize  Flash
	SIM->SCGC |=SIM_SCGC_FLASH_MASK;      // Enable clock for Flash
	
}

void PW_Validation(void){
										if (!password_created){
										flashWritePassword();
										oled_blank();
										flashReadPassword();
										print_string("PW SAVED", 2, 1, 8);
										Delay(1000);
										password_created = 1;
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
										}
										Delay(1500);
									}
									oled_blank();
									enter_mode=0;
									cur_index = 0;
									current_mode=MODE_DEFAULT;
									/*if(reset==true){
										oled_blank();
										print_string("NEW PW", 2, 1, 6);
												sel_mode=PASSWORD;
										    password_created=0;
											}
										
									else{
									sel_mode=RUN_TIME;
									stroke_completed=0;
										run_check();
									}*/
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
    if (d > 9) d = 0;

    entered[cur_index] = (uint8_t)d;

    display_password_masked();
}


void shift_place(void)
{
    cur_index++;

    if (cur_index >= PASSWORD_LENGTH)
    {
        // All 4 digits entered ? pulse fully masked
        //cur_index = 0;

        display_password_masked();  // <– force redraw so 4th digit becomes *
        if(KEY1_PRESSED==0 && KEY2_PRESSED==0){
					Delay(150);
					if(KEY1_PRESSED==0 && KEY2_PRESSED==0){
        if (!password_created)
        {
            flashWritePassword();
            oled_blank();
            print_string("PW SAVED", 2, 1, 8);
					  current_mode=sel_mode;
            Delay(1000);
            password_created = 1;
        }
        else
        {
            if (entered[0] == saved_password[0] &&
                entered[1] == saved_password[1] &&
                entered[2] == saved_password[2] &&
                entered[3] == saved_password[3])
            {
                oled_blank();
                print_string("OK", 2, 1, 2);
							current_mode=sel_mode;
            }
            else
            {
                oled_blank();
                print_string("WRONG", 2, 1, 5);
							current_mode=sel_mode;

            }

            Delay(1500);
        }

        oled_blank();
				enter_mode=0;
				 cur_index = 0;
    }}}
		else{
   display_password_masked(); // Draw with cursor at new position
		}
}


/*void shift_place(void)
{
    cur_index++;

    if (cur_index >= PASSWORD_LENGTH)
    {
        // 4 digits complete
        cur_index = 0;

        if (!password_created)
        {
            // First time ? Save password
            flashWritePassword();
            oled_blank();
            print_string("PW SAVED", 2, 1, 1);
            Delay(1000);
            password_created = 1;
        }
        else
        {
            // Verify password
            if (entered[0] == saved_password[0] &&
                entered[1] == saved_password[1] &&
                entered[2] == saved_password[2] &&
                entered[3] == saved_password[3])
            {
                oled_blank();
                print_string("OK", 2, 1, 2);
            }
            else
            {
                oled_blank();
                print_string("WRONG", 2, 1, 5);
            }

            Delay(1500);
        }

        oled_blank();
        display_password_masked();
    }

    display_password_masked();
}*/

void show_mode(char* mode_str, uint8_t mode_id) {
    if (last_shown_mode != mode_id) {
        oled_blank();
        print_string(mode_str, 2, 1, strlen(mode_str));
        last_shown_mode = mode_id;
    }
}
int main(void){
	
	SystemCoreClockUpdate(); //get Core Clock Frequency
  SysTick_Config(SystemCoreClock/1000); //Generate interrupt each 1 ms
  NVIC_SetPriority(SysTick_IRQn, 2); //Set Int priority and enable Intr
	
	mcu_init();
	i2c_int();
	oled_init();
	//oled_vertical_clear_center(1);
 // Delay(10);
	oled_blank();
	Delay(100);
	flashReadPassword();	
	fact_sel_mode=1;
	while(1){
		
		
		 switch(current_mode)
    {
        case MODE_DEFAULT:
					
				if(KEY1_PRESSED==0 && KEY2_PRESSED==0 && dualMode==false){
					Delay(150);
					if(KEY1_PRESSED==0 &&KEY2_PRESSED==0){
						dual_mode[0]=fact_sel_mode;
						dualMode=true;
						oled_blank();
					}
				}
          if(KEY1_PRESSED==0 && KEY2_PRESSED==1){
					Delay(150);
					if(KEY1_PRESSED==0 && KEY2_PRESSED==1){
						fact_sel_mode++;
					}
				}
				if(fact_sel_mode>7){
					fact_sel_mode=1;
				}
				show_mode(mode[fact_sel_mode],fact_sel_mode);
				
				/*if(fact_sel_mode==1){
					show_mode("SPL",MODE_1);  //pr not required
				}
				if(fact_sel_mode==2){
					show_mode("MP-SA",MODE_2);  // pr required
				}
				if(fact_sel_mode==3){
					show_mode("MPL-PLC",MODE_3); //pr required
				}
				if(fact_sel_mode==4){
					show_mode("PR-SA",MODE_DEFAULT);  //pr not required
				}
				if(fact_sel_mode==5){
					show_mode("PR-PLC",MODE_1);  //pr not required
				}
				if(fact_sel_mode==6){
					show_mode("SL-SA",MODE_2);  //pr required
				}
				if(fact_sel_mode==7){
					show_mode("SL-PLC",MODE_3);  //pr required
				}*/
				
				if(KEY2_PRESSED==0 && KEY1_PRESSED==1){
					Delay(150);
					if(KEY2_PRESSED==0  && KEY1_PRESSED==1){
						sel_mode=PASSWORD;
						oled_blank();
						Delay(30);
						show_mode("PW", PASSWORD);
						oled_blank();
						Delay(30);
						digit_active=0;
						display_password_masked();   // shows ****
						enter_mode=1;
						current_mode=PASSWORD;
						if(dualMode==true){
							dual_mode[1]=fact_sel_mode;
							print_string("MODE 2", 2, 0, 6);
						}
						else{
							print_string("MODE 1", 2, 0, 6);
						}
						oled_blank();
						break;
							}
						}
            break;
        case MODE_1:
              show_mode("MODE 1", MODE_1);
				if(KEY1_PRESSED==0 && KEY2_PRESSED==0){
							Delay(150);
							if(KEY1_PRESSED==0 && KEY2_PRESSED==0){
								sel_mode=current_mode;
								current_mode=PASSWORD;
								oled_blank();
								Delay(30);
								show_mode("PW", PASSWORD);
								oled_blank();
								Delay(30);
								display_password_masked();   // shows ****
								enter_mode=1;
								break;
							}
						}
            break;

            break;

        case MODE_2:
            print_string("MODE 2", 2, 0, 6);
            break;

        case MODE_3:
            print_string("MODE 3", 2, 0, 6);
            break;
				case PASSWORD:
					  if(KEY1_PRESSED == 0){  //digit entery
							Delay(150);
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
							Delay(150);
							if(KEY2_PRESSED == 0 &&cur_index<=4){
								show_cursor = 1;       // enable cursor
								shift_place();
							}}
						if (cur_index >= PASSWORD_LENGTH){
							display_password_masked();  // <– force redraw so 4th digit becomes *
							if(KEY1_PRESSED==0 && KEY2_PRESSED==0){
								Delay(150);
								if(KEY1_PRESSED==0 && KEY2_PRESSED==0){
									PW_Validation();

						}}}
						else{
							display_password_masked(); // Draw with cursor at new position
							}
					
					break;
    }
		/*if(KEY1_PRESSED == 0){
    Delay(150);
    if(KEY1_PRESSED == 0){
        show_cursor = 1;
        digit = entered[cur_index] + 1;

        if(digit > 9) digit = 0;

        updat_password(digit);
    }
}


if(KEY2_PRESSED == 0){
    Delay(150);
    if(KEY2_PRESSED == 0 &&cur_index<=4){
        show_cursor = 1;       // enable cursor
        shift_place();
    }
}*/

		
		}
	
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
	  FLASH_Program1LongWord(FLASH_PASSWORD_DATA_ADDR, packed);
    FLASH_Program1LongWord(FLASH_PASSWORD_FLAG_ADDR, PASSWORD_FLAG_VALUE);
   
}
