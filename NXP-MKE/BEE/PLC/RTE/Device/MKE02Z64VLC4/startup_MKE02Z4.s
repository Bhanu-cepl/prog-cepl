; * ---------------------------------------------------------------------------------------
; *  @file:    startup_MKE02Z4.s
; *  @purpose: CMSIS Cortex-M0P Core Device Startup File
; *            MKE02Z4
; *  @version: 1.0
; *  @date:    2017-5-19
; *  @build:   b170811
; * ---------------------------------------------------------------------------------------
; *
; * Copyright 1997-2016 Freescale Semiconductor, Inc.
; * Copyright 2016-2017 NXP
; * Redistribution and use in source and binary forms, with or without modification,
; * are permitted provided that the following conditions are met:
; *
; * 1. Redistributions of source code must retain the above copyright notice, this list
; *   of conditions and the following disclaimer.
; *
; * 2. Redistributions in binary form must reproduce the above copyright notice, this
; *   list of conditions and the following disclaimer in the documentation and/or
; *   other materials provided with the distribution.
; *
; * 3. Neither the name of the copyright holder nor the names of its
; *   contributors may be used to endorse or promote products derived from this
; *   software without specific prior written permission.
; *
; * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
; * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
; * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
; * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
; * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
; * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
; * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
; * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
; * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
; * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
; *
; *------- <<< Use Configuration Wizard in Context Menu >>> ------------------
; *
; *****************************************************************************/


                PRESERVE8
                THUMB


; Vector Table Mapped to Address 0 at Reset

                AREA    RESET, DATA, READONLY
                EXPORT  __Vectors
                EXPORT  __Vectors_End
                EXPORT  __Vectors_Size
                IMPORT  |Image$$ARM_LIB_STACK$$ZI$$Limit|

__Vectors       DCD     |Image$$ARM_LIB_STACK$$ZI$$Limit| ; Top of Stack
                DCD     Reset_Handler  ; Reset Handler
                DCD     NMI_Handler                         ;NMI Handler
                DCD     HardFault_Handler                   ;Hard Fault Handler
                DCD     0                                   ;Reserved
                DCD     0                                   ;Reserved
                DCD     0                                   ;Reserved
                DCD     0                                   ;Reserved
                DCD     0                                   ;Reserved
                DCD     0                                   ;Reserved
                DCD     0                                   ;Reserved
                DCD     SVC_Handler                         ;SVCall Handler
                DCD     0                                   ;Reserved
                DCD     0                                   ;Reserved
                DCD     PendSV_Handler                      ;PendSV Handler
                DCD     SysTick_Handler                     ;SysTick Handler

                                                            ;External Interrupts
                DCD     Reserved16_IRQHandler               ;Reserved interrupt
                DCD     Reserved17_IRQHandler               ;Reserved interrupt
                DCD     Reserved18_IRQHandler               ;Reserved interrupt
                DCD     Reserved19_IRQHandler               ;Reserved interrupt
                DCD     Reserved20_IRQHandler               ;Reserved interrupt
                DCD     FTMRH_IRQHandler                    ;Command complete and error interrupt
                DCD     PMC_IRQHandler                      ;Low-voltage warning
                DCD     IRQ_IRQHandler                      ;External interrupt
                DCD     I2C0_IRQHandler                     ;Single interrupt vector for all sources
                DCD     Reserved25_IRQHandler               ;Reserved interrupt
                DCD     SPI0_IRQHandler                     ;Single interrupt vector for all sources
                DCD     SPI1_IRQHandler                     ;Single interrupt vector for all sources
                DCD     UART0_IRQHandler                    ;Status and error
                DCD     UART1_IRQHandler                    ;Status and error
                DCD     UART2_IRQHandler                    ;Status and error
                DCD     ADC_IRQHandler                      ;ADC conversion complete interrupt
                DCD     ACMP0_IRQHandler                    ;Analog comparator 0 interrupt
                DCD     FTM0_IRQHandler                     ;FTM0 single interrupt vector for all sources
                DCD     FTM1_IRQHandler                     ;FTM1 single interrupt vector for all sources
                DCD     FTM2_IRQHandler                     ;FTM2 single interrupt vector for all sources
                DCD     RTC_IRQHandler                      ;RTC overflow
                DCD     ACMP1_IRQHandler                    ;Analog comparator 1 interrupt
                DCD     PIT_CH0_IRQHandler                  ;PIT CH0 overflow
                DCD     PIT_CH1_IRQHandler                  ;PIT CH1 overflow
                DCD     KBI0_IRQHandler                     ;Keyboard interrupt0
                DCD     KBI1_IRQHandler                     ;Keyboard interrupt1
                DCD     Reserved42_IRQHandler               ;Reserved interrupt
                DCD     ICS_IRQHandler                      ;Clock loss of lock
                DCD     WDOG_IRQHandler                     ;Watchdog timeout
                DCD     Reserved45_IRQHandler               ;Reserved interrupt
                DCD     Reserved46_IRQHandler               ;Reserved interrupt
                DCD     Reserved47_IRQHandler               ;Reserved interrupt
__Vectors_End

__Vectors_Size  EQU     __Vectors_End - __Vectors

; <h> Flash Configuration
;   <i> 16-byte flash configuration field that stores default protection settings (loaded on reset)
;   <i> and security information that allows the MCU to restrict access to the FTFL module.
;   <h> Backdoor Comparison Key
;   </h>
; </h>
                IF      :LNOT::DEF:RAM_TARGET
                AREA    FlashConfig, DATA, READONLY
__FlashConfig
                DCB     0xFF      , 0xFF      , 0xFF      , 0xFF
                DCB     0xFF      , 0xFF      , 0xFF      , 0xFF
                DCB     0xFF      , 0xFF      , 0xFF      , 0xFF
                DCB     0xFF      , 0xFF      , 0xFE      , 0xFF
                ENDIF


                AREA    |.text|, CODE, READONLY

; Reset Handler

Reset_Handler   PROC
                EXPORT  Reset_Handler             [WEAK]
                IMPORT  SystemInit
                IMPORT  __main

                IF      :LNOT::DEF:RAM_TARGET
                REQUIRE FlashConfig
                ENDIF

                CPSID   I               ; Mask interrupts
                LDR     R0, =0xE000ED08
                LDR     R1, =__Vectors
                STR     R1, [R0]
                LDR     R2, [R1]
                MSR     MSP, R2
                LDR     R0, =SystemInit
                BLX     R0
                CPSIE   i               ; Unmask interrupts
                LDR     R0, =__main
                BX      R0
                ENDP


; Dummy Exception Handlers (infinite loops which can be modified)
NMI_Handler\
                PROC
                EXPORT  NMI_Handler         [WEAK]
                B       .
                ENDP
HardFault_Handler\
                PROC
                EXPORT  HardFault_Handler         [WEAK]
                B       .
                ENDP
SVC_Handler\
                PROC
                EXPORT  SVC_Handler         [WEAK]
                B       .
                ENDP
PendSV_Handler\
                PROC
                EXPORT  PendSV_Handler         [WEAK]
                B       .
                ENDP
SysTick_Handler\
                PROC
                EXPORT  SysTick_Handler         [WEAK]
                B       .
                ENDP
I2C0_IRQHandler\
                PROC
                EXPORT  I2C0_IRQHandler         [WEAK]
                LDR     R0, =I2C0_DriverIRQHandler
                BX      R0
                ENDP

SPI0_IRQHandler\
                PROC
                EXPORT  SPI0_IRQHandler         [WEAK]
                LDR     R0, =SPI0_DriverIRQHandler
                BX      R0
                ENDP

SPI1_IRQHandler\
                PROC
                EXPORT  SPI1_IRQHandler         [WEAK]
                LDR     R0, =SPI1_DriverIRQHandler
                BX      R0
                ENDP

UART0_IRQHandler\
                PROC
                EXPORT  UART0_IRQHandler         [WEAK]
                LDR     R0, =UART0_DriverIRQHandler
                BX      R0
                ENDP

UART1_IRQHandler\
                PROC
                EXPORT  UART1_IRQHandler         [WEAK]
                LDR     R0, =UART1_DriverIRQHandler
                BX      R0
                ENDP

UART2_IRQHandler\
                PROC
                EXPORT  UART2_IRQHandler         [WEAK]
                LDR     R0, =UART2_DriverIRQHandler
                BX      R0
                ENDP

RTC_IRQHandler\
                PROC
                EXPORT  RTC_IRQHandler         [WEAK]
                LDR     R0, =RTC_DriverIRQHandler
                BX      R0
                ENDP

Default_Handler\
                PROC
                EXPORT  Reserved16_IRQHandler         [WEAK]
                EXPORT  Reserved17_IRQHandler         [WEAK]
                EXPORT  Reserved18_IRQHandler         [WEAK]
                EXPORT  Reserved19_IRQHandler         [WEAK]
                EXPORT  Reserved20_IRQHandler         [WEAK]
                EXPORT  FTMRH_IRQHandler         [WEAK]
                EXPORT  PMC_IRQHandler         [WEAK]
                EXPORT  IRQ_IRQHandler         [WEAK]
                EXPORT  I2C0_DriverIRQHandler         [WEAK]
                EXPORT  Reserved25_IRQHandler         [WEAK]
                EXPORT  SPI0_DriverIRQHandler         [WEAK]
                EXPORT  SPI1_DriverIRQHandler         [WEAK]
                EXPORT  UART0_DriverIRQHandler         [WEAK]
                EXPORT  UART1_DriverIRQHandler         [WEAK]
                EXPORT  UART2_DriverIRQHandler         [WEAK]
                EXPORT  ADC_IRQHandler         [WEAK]
                EXPORT  ACMP0_IRQHandler         [WEAK]
                EXPORT  FTM0_IRQHandler         [WEAK]
                EXPORT  FTM1_IRQHandler         [WEAK]
                EXPORT  FTM2_IRQHandler         [WEAK]
                EXPORT  RTC_DriverIRQHandler         [WEAK]
                EXPORT  ACMP1_IRQHandler         [WEAK]
                EXPORT  PIT_CH0_IRQHandler         [WEAK]
                EXPORT  PIT_CH1_IRQHandler         [WEAK]
                EXPORT  KBI0_IRQHandler         [WEAK]
                EXPORT  KBI1_IRQHandler         [WEAK]
                EXPORT  Reserved42_IRQHandler         [WEAK]
                EXPORT  ICS_IRQHandler         [WEAK]
                EXPORT  WDOG_IRQHandler         [WEAK]
                EXPORT  Reserved45_IRQHandler         [WEAK]
                EXPORT  Reserved46_IRQHandler         [WEAK]
                EXPORT  Reserved47_IRQHandler         [WEAK]
                EXPORT  DefaultISR         [WEAK]
Reserved16_IRQHandler
Reserved17_IRQHandler
Reserved18_IRQHandler
Reserved19_IRQHandler
Reserved20_IRQHandler
FTMRH_IRQHandler
PMC_IRQHandler
IRQ_IRQHandler
I2C0_DriverIRQHandler
Reserved25_IRQHandler
SPI0_DriverIRQHandler
SPI1_DriverIRQHandler
UART0_DriverIRQHandler
UART1_DriverIRQHandler
UART2_DriverIRQHandler
ADC_IRQHandler
ACMP0_IRQHandler
FTM0_IRQHandler
FTM1_IRQHandler
FTM2_IRQHandler
RTC_DriverIRQHandler
ACMP1_IRQHandler
PIT_CH0_IRQHandler
PIT_CH1_IRQHandler
KBI0_IRQHandler
KBI1_IRQHandler
Reserved42_IRQHandler
ICS_IRQHandler
WDOG_IRQHandler
Reserved45_IRQHandler
Reserved46_IRQHandler
Reserved47_IRQHandler
DefaultISR
                LDR    R0, =DefaultISR
                BX     R0
                ENDP
                  ALIGN


                END
