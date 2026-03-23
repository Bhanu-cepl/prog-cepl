#include "MKE02Z4.h"                    // Device header
#include "main.h"
#include "stdbool.h"
#include "stdio.h"
#include "string.h"

#define HOM  1UL<<21
#define HOM_STATUS ((GPIOA->PDIR>>21)&0X01)


#define CYCLE_NUM_X      90     // starting X (change as needed)
#define CYCLE_NUM_WIDTH  20     // enough for 2 digits


#define NUM_X   86
#define NUM_WIDTH  30   // enough for 4-5 digits

#define TIME_X      25      // start of HH:MM
#define TIME_WIDTH  30      // enough to clear 5 chars


#define CHAR_W         22       // width of one character
#define CHAR_H_PAGES    3       // each character is 3 pages tall

#define P_X1             2
#define P_Y1             1
#define HH_X1    (P_X1 + 2*(CHAR_W))        // after "P:"
#define COLON_X1 (HH_X1 + CHAR_W+10)       // after HH
#define MM_X1    (COLON_X1 + 2*(CHAR_W-12))      // after ":"

char rtime_str[10];
  uint8_t disp_mode;
uint32_t s_delay=0,timer=0,pause_time=5,PauseTime=0,m_timer=0,RemainingSeconds=0;
struct timervalue{
uint32_t Seconds;
uint32_t Minutes;
uint32_t Hours;
};

struct timervalue runtime,pausetime,elapsedsetting;

int i=0;
int counter = 0,cycle=0;  // Variable to increment
char buf[16];

volatile uint32_t msTicks = 0;
uint32_t curTicks = 0;
void SysTick_Handler(void) {
    msTicks++;
}

void Delay(uint32_t dlyTicks) {
    curTicks = msTicks;
    while ((msTicks - curTicks) < dlyTicks);
}

void i2c_int(void)
{
	SIM->SCGC |=	(1<<17);									/*	turn	on	clock	to	I2C1*/
  SIM->PINSEL &= ~(1<<5);
	I2C0->C1 |= 0;
	I2C0->S	=	0x02;																/*	Clear	interrupt	flag	*/
  I2C0->F	=	0X0F;//0x1C;													/*	set	clock	to	97.09KHz	@13.981MHz	bus	clock	*/
  I2C0->C1	=	0x80;		
}

void i2c_start(void){
	int retry = 1000; 
    while (I2C0->S & 0x20) {    /* wait until bus is available */
        if (--retry <= 0)
        Delay(100);
    }
    /* send start */
    I2C0->C1 |= 0x10;    /* Tx on */
    I2C0->C1 |= 0x20;   

}

void i2c_start1(void){
	int retry = 1000; 
    while (I2C0->S & 0x20) {    /* wait until bus is available */
        if (--retry <= 0){
					break;}
				}
    /* send start */
    I2C0->C1 |= 0x10;    /* Tx on */
    I2C0->C1 |= 0x20;   

}

void i2c_add(char address, char RW) {
	    int retry = 2000;
	I2C0->D = (address << 1) | RW;         // Send the slave address with read/write bit

    //while (!(I2C0->S & 0x02));          // Wait for transfer complete
	while (!(I2C0->S & 0x02)) {
        if (--retry <= 0) {
            break; // timeout
        }
    }
    I2C0->S |= (1 << 1);               // Clear interrupt flag
    if (I2C0->S & 0x10) {             // Check for arbitration lost
        I2C0->S |= 0x10;             // Clear ARBL
    }
    if (I2C0->S & 0x01) {          // Check for NACK from slave
        // Handle NACK if needed
    }
}


void i2c_data(char data) {
		int retry = 200; 
    I2C0->D = data;  	// Send data
    while (I2C0->S & 0x20) {    /* wait until bus is available */
        if (--retry <= 0){
					break;}
				}
   // while (!(I2C0->S & 0x02));     // Wait for transfer complete
    I2C0->S |= 0x02;              // Clear interrupt flag
    if (I2C0->S & 0x01) {         // Check for NACK from slave

    }
}

void i2c_stop(void){
I2C0->C1 &= ~0x30;
}


void i2c_write(char address,char data[]){
uint16_t i=0;
i2c_start1();
i2c_add(address,0); 
	while(data[i]){
			i2c_data(data[i]);
			i++;
		}
	i2c_stop();
}

void oled_cmd(uint8_t cmd)
{
    i2c_start1();
    i2c_add(0x3C, 0);        // OLED address 0x3C write
    i2c_data(0x00);          // Control byte = command
    i2c_data(cmd);
    i2c_stop();
}
void oled_data(uint8_t data)
{
    i2c_start1();
    i2c_add(0x3C, 0);
    i2c_data(0x40);          // Control byte = data
    i2c_data(data);
    i2c_stop();
}
void oled_init(void)
{
	i2c_int();
    Delay(100);

    oled_cmd(0xAE);   // Display OFF
    oled_cmd(0x20);   // Memory addressing mode
    oled_cmd(0x00);   // Horizontal addressing

    oled_cmd(0x40);   // Start line = 0
    oled_cmd(0xB0);   // Page 0

    oled_cmd(0xC8);   // COM scan direction
    oled_cmd(0xA1);   // Segment remap

    oled_cmd(0xA8);
    oled_cmd(0x1F);   // Multiplex ratio for 128x32

    oled_cmd(0xD3);
    oled_cmd(0x00);   // Display offset

    oled_cmd(0xDA);
    oled_cmd(0x02);   // COMPINS for 32px

    oled_cmd(0x81);
    oled_cmd(0x7F);   // Contrast

    oled_cmd(0xA4);   // Display follows RAM
    oled_cmd(0xA6);   // Normal display

    oled_cmd(0xD5);
    oled_cmd(0x80);   // Clock div

    oled_cmd(0x8D);
    oled_cmd(0x14);   // Enable charge pump

    oled_cmd(0xAF);   // Display ON
}
void oled_write_page(uint8_t page, uint8_t *buffer)
{
	int i=0;
    oled_cmd(0xB0 + page);   // Set page
    oled_cmd(0x00);          // Lower column
    oled_cmd(0x10);          // Higher column

    i2c_start1();
    i2c_add(0x3C, 0);
    i2c_data(0x40);          // Data stream

    for (i = 0; i < 128; i++) {
        i2c_data(buffer[i]);
    }

    i2c_stop();
}
void oled_clear(void) { 
uint8_t buf[128] = {0}; 
int page=0; 
for (page = 0; page < 4; page++)
oled_write_page(page, buf); }
void oled_clear_area(uint8_t page, uint8_t col, uint8_t width)
{
	uint8_t p=0,j=0;
    for(p = 0; p < VER_PAGE_REQ; p++)
    {
        oled_cmd(0xB0 + page + p);
        oled_cmd(0x00 + (col & 0x0F));
        oled_cmd(0x10 + ((col >> 4) & 0x0F));

        i2c_start1();
        i2c_add(0x3C, 0);
        i2c_data(0x40);

        for(j = 0; j < width; j++)
            i2c_data(0x00);

        i2c_stop();
    }
}

void oled_pos(uint8_t page, uint8_t col)
{
    oled_cmd(0xB0 + page);          // Set page
    oled_cmd(0x00 + (col & 0x0F));  // Set lower column
    oled_cmd(0x10 + ((col >> 4) & 0x0F)); // Set higher column
}

void print_char(char ch, uint8_t x, uint8_t y) {
	uint8_t i=0,j=0;
	uint16_t index = (ch - 32) * NO_0F_BYTES_CHAR + 1;
	if(ch < 32 || ch > 127) 
		return; 
	for(j = 0; j < HOR_COL_REQ; j++) { 
for(i = 0; i < VER_PAGE_REQ; i++) {
oled_pos(y + i, x + j);
oled_data(ASCII_16[index + j * VER_PAGE_REQ + i]); 
} } }

// --------------------- Print string ---------------------
void print_string(char *s, uint8_t x, uint8_t y, int length)
{
    uint8_t x_pos = x; // Running horizontal position

    for(i = 0; i < length; i++)
    {
        char ch = s[i];
        print_char(ch, x_pos, y);

        // Update x_pos for next character
        if(ch == 'W' || ch == 'w')
            x_pos += HOR_COL_REQ + 3; // Extra spacing for wide chars
        else
            x_pos += HOR_COL_REQ - 6; // Normal spacing
    }
}

/*void update_time_display(uint16_t counter)
{
    static uint8_t old_h = 255;
    static uint8_t old_m = 255;
	char buf[4];

    uint8_t hours  = counter / 60;
    uint8_t minutes = counter % 60;

    // HH changed?
    if (hours != old_h)
    {
        // Clear HH (2 chars = 12px)
        oled_clear_area(TIME_X, 1, 12);

        
        sprintf(buf, "%02d", hours);
        print_string(buf, TIME_X, 1, 2);

        old_h = hours;
    }

    // MM changed?
    if (minutes != old_m)
    {
        // Clear MM (2 chars = 12px)
        oled_clear_area(TIME_X + 28, 1, 12);

        sprintf(buf, "%02d", minutes);
        print_string(buf, TIME_X + 28, 1, 2);

        old_m = minutes;
    }
}*/
void update_time_display(uint16_t counter)
{
    static uint8_t old_h = 255;
    static uint8_t old_m = 255;
    char buf[4];
	static uint8_t colon_drawn = 0;

    uint8_t hours  = counter / 60;
    uint8_t minutes = counter % 60;

    // ----- HH -----
    if (hours != old_h)
    {
        oled_clear_area(HH_X1, P_Y1, 2 * CHAR_W);   // clear 44px
        sprintf(buf, "%02d", hours);
        print_string(buf, HH_X1, P_Y1, 2);
        old_h = hours;
    }

    // ----- MM -----
    if (minutes != old_m)
    {
        oled_clear_area(MM_X1, P_Y1, 2 * CHAR_W);   // clear 44px
			 print_string(":", COLON_X1, P_Y1, 1);
        sprintf(buf, "%02d", minutes);
        print_string(buf, MM_X1, P_Y1, 2);
        old_m = minutes;
    }

    // draw colon only once
    
    if (!colon_drawn)
    {
        print_string(":", COLON_X1, P_Y1, 1);
        colon_drawn = 1;
    }
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
// mode: 0 = MM:SS, 1 = HH:MM
void update_PT_display(uint32_t totalSeconds, uint8_t mode)
{
    static uint8_t old_h = 255;
    static uint8_t old_m = 255;
    static uint8_t old_s = 255;
	static uint8_t last_mode = 255;
    static uint8_t colon_drawn = 0;

    uint8_t hours   = totalSeconds / 3600;
    uint8_t minutes = (totalSeconds % 3600) / 60;
    uint8_t seconds = totalSeconds % 60;

    char buf[4];

    // Draw PT only once


    // If user changes mode, reset old values
    
    if (mode != last_mode)
    {
        oled_clear_area(HH_X1, P_Y1, 50);   // clear HH : MM or MM : SS
        colon_drawn = 0;
        old_h = old_m = old_s = 255;
        last_mode = mode;
    }

    // --------------------------------
    // MODE = 1 ? HH:MM
    // --------------------------------
    if (mode == 1)
    {
        // Hours changed?
        if (hours != old_h)
        {
            oled_clear_area(HH_X1, P_Y1, 2 * CHAR_W);
					print_string(":", COLON_X1, P_Y1, 1);
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

    // --------------------------------
    // MODE = 0 ? MM:SS
    // --------------------------------
    else
    {
        // Minutes changed?
        if (minutes != old_m)
        {
            oled_clear_area(HH_X1, P_Y1, 2 * CHAR_W);
					print_string(":", COLON_X1, P_Y1, 1);
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

    // Draw colon only once
    if (!colon_drawn)
    {
        print_string(":", COLON_X1, P_Y1, 1);
        colon_drawn = 1;
    }
}


int main(void){
	

	SystemCoreClockUpdate(); //get Core Clock Frequency
  SysTick_Config(SystemCoreClock/1000); //Generate interrupt each 1 ms
  NVIC_SetPriority(SysTick_IRQn, 2); //Set Int priority and enable Intr
	
	GPIOA->PDDR |=(1UL<<12);
	GPIOA->PIDR&=~(KEY1|KEY2|HOM);
	PORT->PUEL |=(KEY1|KEY2|HOM);
	rtc_init();
	 oled_init();
    oled_clear();
GPIOA->PCOR |=(1UL<<12);
//print_string("CYCLE-", 2, 1, 6);   // stays fixed
	print_string("PT:", P_X1, P_Y1, 3);
	while(1){
		
		if(KEY1_PRESSED == 1 && KEY2_PRESSED == 1)
{
		
    // -------- calculate PauseTime --------
        PauseTime = pause_time * 60;

    // -------- calculate remaining time --------
    RemainingSeconds = PauseTime - timer;

    // -------- call the NEW display function --------
    // mode 1 = HH:MM, mode 0 = MM:SS
  

    if (RemainingSeconds >= 3600)
        disp_mode = 1;   // show HH:MM
    else
        disp_mode = 0;   // show MM:SS

    update_PT_display(RemainingSeconds, disp_mode);
}

		/*if(HOM_STATUS==0){
			Delay(150);
			if(HOM_STATUS==0){
			GPIOA->PCOR |=(1UL<<12);
		}}
	if(HOM_STATUS==1){
			Delay(150);
			if(HOM_STATUS==1){
		GPIOA->PSOR |=(1UL<<12);}}
						 Delay(500);*/

		
/*if(KEY1_PRESSED==0){
			 Delay(50);
			 if(KEY1_PRESSED==0){
				 cycle++;
				 if(cycle>50){
					 cycle=50;}
			 }
		 }
		 if(KEY2_PRESSED==0){
			 Delay(50);
			 if(KEY2_PRESSED==0){
				 cycle--;
			 }
		 }
		 if(cycle>50 || cycle<=0){
					cycle=1;
				}

  oled_clear_area(CYCLE_NUM_X, 1, CYCLE_NUM_WIDTH);
   sprintf(buf, "%02d", cycle);
  print_string(buf, CYCLE_NUM_X, 1, strlen(buf));*/
	}
	
	return 0;
}
