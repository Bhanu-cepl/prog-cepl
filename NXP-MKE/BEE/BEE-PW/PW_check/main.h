#ifndef MAIN_H
#define MAIN_H
#include "stdint.h"

#define KEY1 1UL<<13
#define KEY2 1UL<<14

#define KEY2_PRESSED ((GPIOA->PDIR>>13)&0X01)
#define KEY1_PRESSED ((GPIOA->PDIR>>14)&0X01)


void Delay(uint32_t dlyTicks);
void updat_password(int digit);
void shift_place(void);
void flashReadPassword(void);
void flashWritePassword(void);
#endif