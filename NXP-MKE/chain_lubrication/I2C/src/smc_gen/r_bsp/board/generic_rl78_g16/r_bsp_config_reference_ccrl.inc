;;/*
;;* Copyright (c) 2023 Renesas Electronics Corporation and/or its affiliates
;;*
;;* SPDX-License-Identifier: BSD-3-Clause
;;*/
;;/***********************************************************************************************************************
;;* File Name    : r_bsp_config_reference.inc
;;* H/W Platform : GENERIC_RL78_G16
;;* Description  : 
;;***********************************************************************************************************************/
;;/***********************************************************************************************************************
;;* History : DD.MM.YYYY Version  Description
;;*         : 31.01.2023 1.50     First Release
;;*         : 04.07.2025 1.92     Changed the disclaimer.
;;***********************************************************************************************************************/

$IFDEF __r_bsp_config_inc__
$ELSE

__r_bsp_config_inc__                .EQU    1
BSP_CFG_ASM_RTOS_USED               .EQU    0

$ENDIF    ; __r_bsp_config_inc__
