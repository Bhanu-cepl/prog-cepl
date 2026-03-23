#include "r_smc_entry.h"
#include "stdint.h"

#include "Config_IICA0.h"

#define OLED_ADDR   (0x3C << 1)   // 0x78

extern volatile uint8_t g_iica0_status;

#define DELAY_LOOP 8000

#define ENABLE_PULLUP(pureg, bitmask)   ((pureg) |= (bitmask))

/* ---------- GPIO direction ---------- */
#define GPIO_INPUT(REG, BIT)     ((REG) |=  (1U << (BIT)))
#define GPIO_OUTPUT(REG, BIT)    ((REG) &= ~(1U << (BIT)))

/* ---------- GPIO output control ---------- */
#define GPIO_SET(PORT, BIT)      ((PORT) |=  (1U << (BIT)))
#define GPIO_CLR(PORT, BIT)      ((PORT) &= ~(1U << (BIT)))

/* ---------- GPIO input read ---------- */
#define GPIO_READ(PORT, BIT)     (((PORT) >> (BIT)) & 0x01)

/* ---------- GPIO pull-up control ---------- */
#define GPIO_PULLUP_ENABLE(PU, BIT)   ((PU) |=  (1U << (BIT)))
#define GPIO_PULLUP_DISABLE(PU, BIT)  ((PU) &= ~(1U << (BIT)))

#define PIN_P1_3     3
#define PIN_P1_2     2
#define PIN_P4_3     3

#define SW4          GPIO_READ(P1, PIN_P1_3)
#define SW2          GPIO_READ(P1, PIN_P1_2)

#define LED_ON()     GPIO_SET(P4, PIN_P4_3)
#define LED_OFF()    GPIO_CLR(P4, PIN_P4_3)



void gpio_init(void)
{

	GPIO_INPUT(PM1, PIN_P1_3);
	GPIO_INPUT(PM1, PIN_P1_2);

	/* Enable internal pull-ups */
	GPIO_PULLUP_ENABLE(PU1, PIN_P1_3);
	GPIO_PULLUP_ENABLE(PU1, PIN_P1_2);

	GPIO_OUTPUT(PM4, PIN_P4_3);

}
void delay_ms(uint16_t ms)
{
    volatile uint32_t count;
    for (uint16_t i = 0; i < ms; i++)
    {
        for (count = 0; count < DELAY_LOOP; count++)
        {
            __nop();
        }
    }
}

void SH1106_Command(uint8_t cmd)
{
	 uint8_t status;
    uint8_t buf[2];
    buf[0] = 0x00;
    buf[1] = cmd;

    status = R_Config_IICA0_Master_Send(OLED_ADDR, buf, 2, 0);
    if (status != SUCCESS) return;


        R_Config_IICA0_Wait_Comend(1);

}

void SH1106_Data(uint8_t data)
{
    uint8_t status;
    uint8_t buf[2] = {0x40, data};

    while (g_iica0_status & ON_COMMU);

    status = R_Config_IICA0_Master_Send(OLED_ADDR, buf, 2, 0);
    if (status != SUCCESS) return;

    R_Config_IICA0_Wait_Comend(1);
}



void SH1106_Init(void)
{
    R_Config_IICA0_Create();
    delay_ms(100);

    SH1106_Command(0xAE);

    SH1106_Command(0x81);   // Contrast control
    SH1106_Command(0xFF);   // 🔥 MAX contrast

  //  SH1106_Command(0x02);   // Column address low (SH1106 offset)
    //SH1106_Command(0x10);   // Column address high

    SH1106_Command(0x40);   // Display start line

    SH1106_Command(0xA1);   // Segment remap
    SH1106_Command(0xC8);   // COM scan direction

    SH1106_Command(0xA6);   // Normal display

    SH1106_Command(0xA8);
    SH1106_Command(0x3F);   // 1/64 duty

    SH1106_Command(0xD3);
    SH1106_Command(0x00);   // Display offset

    SH1106_Command(0xD5);
    SH1106_Command(0x80);   // Oscillator

    SH1106_Command(0xD9);
    SH1106_Command(0x22);   // Pre-charge

    SH1106_Command(0xDB);
    SH1106_Command(0x35);   // VCOMH

    SH1106_Command(0xAD);
    SH1106_Command(0x8B);   // DC-DC ON (SH1106)

    SH1106_Command(0xAF);   // Display ON
    SH1106_Command(0x00);   // column low
    SH1106_Command(0x10);   // column high

}

#define SH1106_VISIBLE_COLS 128
#define SH1106_COL_OFFSET  2
void SH1106_SetCursor(uint8_t page, uint8_t col)
{
    uint8_t real_col = col + SH1106_COL_OFFSET;

    SH1106_Command(0xB0 | page);
    SH1106_Command(0x00 | (real_col & 0x0F));
    SH1106_Command(0x10 | (real_col >> 4));
}

void SH1106_DataBurst(uint8_t *buf, uint16_t len)
{
    uint16_t i;
    uint8_t tx[1 + SH1106_VISIBLE_COLS];  // 129 bytes max

    tx[0] = 0x40;

    for (i = 0; i < len; i++)
        tx[i + 1] = buf[i];

    while (g_iica0_status & ON_COMMU);
    R_Config_IICA0_Master_Send(OLED_ADDR, tx, len + 1, 0);
    R_Config_IICA0_Wait_Comend(1);
}



void SH1106_Clear(void)
{
    uint8_t page;
    uint8_t zero[SH1106_VISIBLE_COLS] = {0};

    for (page = 0; page < 8; page++)
    {
        SH1106_Command(0xB0 | page);

        SH1106_Command(0x00 | (SH1106_COL_OFFSET & 0x0F));
        SH1106_Command(0x10 | (SH1106_COL_OFFSET >> 4));

        SH1106_DataBurst(zero, SH1106_VISIBLE_COLS);
    }
}



const uint8_t Font5x8[96][5] =
{
/* SPACE (32) */
{0x00,0x00,0x00,0x00,0x00},

/* ! */
{0x00,0x00,0x5F,0x00,0x00},

/* " */
{0x00,0x07,0x00,0x07,0x00},

/* # */
{0x14,0x7F,0x14,0x7F,0x14},

/* $ */
{0x24,0x2A,0x7F,0x2A,0x12},

/* % */
{0x23,0x13,0x08,0x64,0x62},

/* & */
{0x36,0x49,0x55,0x22,0x50},

/* ' */
{0x00,0x05,0x03,0x00,0x00},

/* ( */
{0x00,0x1C,0x22,0x41,0x00},

/* ) */
{0x00,0x41,0x22,0x1C,0x00},

/* * */
{0x14,0x08,0x3E,0x08,0x14},

/* + */
{0x08,0x08,0x3E,0x08,0x08},

/* , */
{0x00,0x50,0x30,0x00,0x00},

/* - */
{0x08,0x08,0x08,0x08,0x08},

/* . */
{0x00,0x60,0x60,0x00,0x00},

/* / */
{0x20,0x10,0x08,0x04,0x02},

/* 0 */
{0x3E,0x51,0x49,0x45,0x3E},

/* 1 */
{0x00,0x42,0x7F,0x40,0x00},

/* 2 */
{0x42,0x61,0x51,0x49,0x46},

/* 3 */
{0x21,0x41,0x45,0x4B,0x31},

/* 4 */
{0x18,0x14,0x12,0x7F,0x10},

/* 5 */
{0x27,0x45,0x45,0x45,0x39},

/* 6 */
{0x3C,0x4A,0x49,0x49,0x30},

/* 7 */
{0x01,0x71,0x09,0x05,0x03},

/* 8 */
{0x36,0x49,0x49,0x49,0x36},

/* 9 */
{0x06,0x49,0x49,0x29,0x1E},

/* : */
{0x00,0x36,0x36,0x00,0x00},

/* ; */
{0x00,0x56,0x36,0x00,0x00},

/* < */
{0x08,0x14,0x22,0x41,0x00},

/* = */
{0x14,0x14,0x14,0x14,0x14},

/* > */
{0x00,0x41,0x22,0x14,0x08},

/* ? */
{0x02,0x01,0x51,0x09,0x06},

/* @ */
{0x32,0x49,0x79,0x41,0x3E},

/* A */
{0x7E,0x11,0x11,0x11,0x7E},

/* B */
{0x7F,0x49,0x49,0x49,0x36},

/* C */
{0x3E,0x41,0x41,0x41,0x22},

/* D */
{0x7F,0x41,0x41,0x22,0x1C},

/* E */
{0x7F,0x49,0x49,0x49,0x41},

/* F */
{0x7F,0x09,0x09,0x09,0x01},

/* G */
{0x3E,0x41,0x49,0x49,0x7A},

/* H */
{0x7F,0x08,0x08,0x08,0x7F},

/* I */
{0x00,0x41,0x7F,0x41,0x00},

/* J */
{0x20,0x40,0x41,0x3F,0x01},

/* K */
{0x7F,0x08,0x14,0x22,0x41},

/* L */
{0x7F,0x40,0x40,0x40,0x40},

/* M */
{0x7F,0x02,0x04,0x02,0x7F},

/* N */
{0x7F,0x04,0x08,0x10,0x7F},

/* O */
{0x3E,0x41,0x41,0x41,0x3E},

/* P */
{0x7F,0x09,0x09,0x09,0x06},

/* Q */
{0x3E,0x41,0x51,0x21,0x5E},

/* R */
{0x7F,0x09,0x19,0x29,0x46},

/* S */
{0x46,0x49,0x49,0x49,0x31},

/* T */
{0x01,0x01,0x7F,0x01,0x01},

/* U */
{0x3F,0x40,0x40,0x40,0x3F},

/* V */
{0x1F,0x20,0x40,0x20,0x1F},

/* W */
{0x7F,0x20,0x18,0x20,0x7F},

/* X */
{0x63,0x14,0x08,0x14,0x63},

/* Y */
{0x03,0x04,0x78,0x04,0x03},

/* Z */
{0x61,0x51,0x49,0x45,0x43},
};

void SH1106_PrintChar(uint8_t page, uint8_t col, char c)
{
    uint8_t i;
    uint8_t buf[6];

    if (c < 32 || c > 127)
        c = ' ';

    for (i = 0; i < 5; i++)
        buf[i] = Font5x8[c - 32][i];

    buf[5] = 0x00;   // spacing

    SH1106_SetCursor(page, col);
    SH1106_DataBurst(buf, 6);
}


void SH1106_Print(uint8_t page, uint8_t col, const char *str)
{
    while (*str)
    {
        SH1106_PrintChar(page, col, *str);
        col += 6;

        if (col > 126)
        {
            col = 0;
            page++;
        }
        str++;
      //  delay_ms(5);
    }
}

void SH1106_DrawHLine(uint8_t page, uint8_t x1, uint8_t x2, uint8_t bit)
{
    uint8_t line[128] = {0};

    if (page > 7) return;

    if (x1 > x2)
    {
        uint8_t t = x1;
        x1 = x2;
        x2 = t;
    }

    if (x2 >= 128) x2 = 127;

    for (uint8_t x = x1; x <= x2; x++)
        line[x] = bit;

    SH1106_SetCursor(page, 0);
    SH1106_DataBurst(line, 128);
}

void SH1106_DrawVLine(uint8_t x,
                      uint8_t y1,
                      uint8_t y2,
                      uint8_t width)
{
    if (y2 < y1)
    {
        uint8_t t = y1;
        y1 = y2;
        y2 = t;
    }

    for (uint8_t w = 0; w < width; w++)
    {
        uint8_t col = x + w;
        if (col >= 128) break;

        for (uint8_t page = y1 / 8; page <= y2 / 8; page++)
        {
            uint8_t mask = 0x00;

            for (uint8_t y = page * 8; y < (page * 8 + 8); y++)
            {
                if (y >= y1 && y <= y2)
                    mask |= (1 << (y % 8));
            }

            SH1106_SetCursor(page, col);
            SH1106_Data(mask);
        }
    }
}


void main(void)
{
	    R_Systeminit();
	    EI();

	   R_Config_IICA0_Create();
	    gpio_init();

	    SH1106_Init();

   SH1106_Clear();
	LED_OFF();
    delay_ms(5);
    SH1106_Clear();
   // while(1){
    /*    P4_3_SET;
   SH1106_Print(2, 5, "CHAIN LUBRICATION");
 delay_ms(10);
 P4_3_CLR;
   SH1106_Clear();
   delay_ms(10);
    }*/

    	for(;;){
    		// if (SW4 == 0) {
    			// LED_ON();
    			 //delay_ms(100);
    			 SH1106_Print(2, 5, "CHAIN LUBRICATION");
    	//	}
    		//if(SW2 == 0){
    			//  SH1106_Clear();
    			//LED_OFF();
    			//delay_ms(100);
    	//}
    }
 /*   SH1106_Clear();
    SH1106_DrawHLine(4, 0, 127,1);
    SH1106_DrawVLine(63, 0, 63, 1);
    SH1106_Print(0, 1, "MOTOR 1");
    SH1106_Print(2, 1, "MODE:");

    SH1106_Print(0, 65, "MOTOR 2");
    SH1106_Print(2, 65, "MODE:");

    SH1106_Print(5, 1, "MOTOR 3");
    SH1106_Print(7, 1, "MODE:");

    SH1106_Print(5, 65, "MOTOR 4");
    SH1106_Print(7, 65, "MODE:");

for(;;)
    {
        if (SW4 == 0)
        {
            SH1106_Clear();

            SH1106_Print(0, 3, "MOTOR 1");
            SH1106_Print(2, 3, "MOTOR 2");
            SH1106_Print(4, 3, "MOTOR 3");
            SH1106_Print(6, 3, "MOTOR 4");

            delay_ms(2);   // simple debounce
        }
    }*/
}

