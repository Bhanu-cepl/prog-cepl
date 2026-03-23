/*
* Copyright (c) 2021 - 2025 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
*/

/***********************************************************************************************************************
* File Name        : Pin.h
* Version          : 1.0.0
* Device(s)        : R5F121BCxFP
* Description      : This file implements SMC pin code generation.
***********************************************************************************************************************/

#ifndef PIN_H
#define PIN_H

/***********************************************************************************************************************
Macro definitions (Register bit)
***********************************************************************************************************************/

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/

/* User's guide for pin function assignment macros
 * The generated macro definitions can be used in the user application as follows: 
 *
 * Example: Set SCLA0 port to 1U.
 *  PIN_WRITE(SMC_PIN_TO02) = 1;
 *  Xxx = PIN_READ(R_xxx_TI00_Pin);
 */

/* PIOR pin function assignments */
#define SMC_PIN_SDAA0        0,7
#define SMC_PIN_SCLA0        0,6

/* Pin write helper */
#define PIN_WRITE_HELPER(x,y)                    ((P##x##_bit.no##y))
/* Pin read helper */
#define PIN_READ_HELPER(x,y)                     ((P##x##_bit.no##y))

/* Pin write API */
#define PIN_WRITE(...)                           (PIN_WRITE_HELPER(__VA_ARGS__))
/* Pin read API */
#define PIN_READ(...)                            (PIN_READ_HELPER(__VA_ARGS__))

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
Global functions
***********************************************************************************************************************/
void R_Pins_Create(void);
/* Start user code for function. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */
#endif
