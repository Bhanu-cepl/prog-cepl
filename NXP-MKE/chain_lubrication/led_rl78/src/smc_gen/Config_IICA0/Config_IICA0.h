/*
* Copyright (c) 2021 - 2025 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
*/

/***********************************************************************************************************************
* File Name        : Config_IICA0.h
* Component Version: 1.10.0
* Device(s)        : R5F121BCxFP
* Description      : This file implements device driver for Config_IICA0.
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "r_cg_iica.h"

#ifndef CFG_Config_IICA0_H
#define CFG_Config_IICA0_H

/***********************************************************************************************************************
Macro definitions (Register bit)
***********************************************************************************************************************/

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/

#define _10_IICA0_MASTERADDRESS                (0x10U)    /* IICA0 address */
#define _59_IICA0_IICWL_VALUE                  (0x59U)
#define _64_IICA0_IICWH_VALUE                  (0x64U)

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
Global functions
***********************************************************************************************************************/
void R_Config_IICA0_Create (void);
void R_Config_IICA0_Stop (void);
uint8_t R_Config_IICA0_StopCondition (void);
uint8_t R_Config_IICA0_Master_Send (uint8_t sladr8, uint8_t * const tx_buf, uint16_t tx_num, uint8_t wait);
uint8_t R_Config_IICA0_Master_Receive (uint8_t sladr8, uint8_t * const rx_buf, uint16_t rx_num, uint8_t wait);
uint8_t R_Config_IICA0_Check_Comstate (void);
uint8_t R_Config_IICA0_Poll (void);
uint8_t R_Config_IICA0_Wait_Comend (uint8_t stop);
uint8_t R_Config_IICA0_Bus_Check (void);
uint8_t R_Config_IICA0_StartCondition (void);
void R_Config_IICA0_Wait_Time (void);
void R_Config_IICA0_Create_UserInit (void);
/* Start user code for function. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */
#endif

