#include "MKE02Z4.h"                    // Device header
#include "main.h"
#include "i2c.h"
#include "string.h"

#define CHAR_W         22       // width of one character


uint8_t pt_digits[6] = {0,0,0,0};
// Fixed positions for H1,H2,M1,M2
uint8_t pt_index = 0; 

uint8_t prev_digits[4] = {255,255,255,255}; // impossible initial value
uint8_t prev_blink_state = 2;               // impossible initial value


uint8_t blink_state = 1;      // 1 = visible, 0 = invisible
uint32_t last_blink_tick = 0; // store last tick when blink toggled

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
	
	/*(void)FLASH_Init(10000000L); 					// Initialize  Flash
	SIM->SCGC |=SIM_SCGC_FLASH_MASK;      // Enable clock for Flash*/
	
}

/*void display_pause_time(void)
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
}*/
void display_pause_time(void)
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
			 uint16_t total_min = get_pause_minutes();
        pt_index = 0;
		}

    display_pause_time();
}


int main(void){
	
	SystemCoreClockUpdate(); //get Core Clock Frequency
  SysTick_Config(SystemCoreClock/1000); //Generate interrupt each 1 ms
  NVIC_SetPriority(SysTick_IRQn, 2); //Set Int priority and enable Intr
	
	mcu_init();
	i2c_int();
	oled_init();
	Delay(150);
	oled_blank();
	Delay(100);

    print_char('P', 2, 1);
    print_char(':', 16, 1);
   // print_char(':', 65, 1);
	
	 display_pause_time();

    // Start counting from now so first toggle happens ~500ms later
    last_blink_tick = msTicks;
	while(1)
{
	uint32_t now = msTicks;

    // Toggle blink every 500 ms
    if(now - last_blink_tick >= 500)
    {
        blink_state = !blink_state;
        last_blink_tick = now;
        display_pause_time();
    }
    // KEY1 ? Increase digit
    if(KEY1_PRESSED == 0)
    {
        Delay(150);
        if(KEY1_PRESSED == 0)
        {
            increment_pt_digit();
        }
    }

    // KEY2 ? Move cursor to next digit
    if(KEY2_PRESSED == 0)
    {
        Delay(150);
        if(KEY2_PRESSED == 0)
        {
            next_pt_digit();
        }
    }
}

	
}


