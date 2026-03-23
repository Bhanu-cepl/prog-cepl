#ifndef OLED_H
#define OLED_H
#include "stdint.h"

#define NO_0F_BYTES_CHAR      73    //number of bytes in the single line
#define HOR_COL_REQ           21    //22X21  where 21 is the horizontal space
#define VER_PAGE_REQ          3    // 21/8

extern const unsigned char hom[];
extern const uint8_t home_icon[];
extern const uint8_t block_16x2[];
extern const uint8_t low_level_icon_32x32[];
extern const uint8_t single_arrow_32x16[] ;
void i2c_int(void);
void i2c_start(void);
void i2c_add(char address, char RW);
void i2c_data(char data);
void i2c_stop(void);
void i2c_write(char address,char data[]);
void oled_cmd_1byte(char data);
void oled_cmd_2byte(char data[]);				
void oled_init(void);
void oled_data(char data);
void oled_pos(char Ypos, char Xpos);
void oled_blank(void);
void print_char(char ch, char x_cord, char y_cord);
void print_string(char *s,char x,char y,int length);
void draw_icon(const uint8_t *icon, char x, char y_page, char width, char pages);
void oled_vertical_clear_center(uint16_t delay_ms);
				
				
#endif