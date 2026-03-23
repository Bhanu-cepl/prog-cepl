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

void oled_cmd_1byte(char data);
void oled_cmd_2byte(char data[]);				
void oled_init(void);
void oled_data(char data);
void oled_pos(char Ypos, char Xpos);
void oled_blank(void);
void print_string(char *s,char x,char y,int length);
void draw_icon(const uint8_t *icon, char x, char y_page, char width, char pages);
void oled_scroll_right(void);			
void oled_scroll_left(void);
void oled_scroll_vertical_up(void);
#endif