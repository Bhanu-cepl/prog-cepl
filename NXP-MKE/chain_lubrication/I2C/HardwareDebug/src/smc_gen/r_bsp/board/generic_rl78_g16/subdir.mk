################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables
C_SRCS += \
../src/smc_gen/r_bsp/board/generic_rl78_g16/hdwinit.c \
../src/smc_gen/r_bsp/board/generic_rl78_g16/r_bsp_init.c 

ASM_SRCS += \
../src/smc_gen/r_bsp/board/generic_rl78_g16/stkinit.asm 

ASSEMBLER_OBJS += \
src/smc_gen/r_bsp/board/generic_rl78_g16/stkinit.obj 

ASM_DEPS += \
src/smc_gen/r_bsp/board/generic_rl78_g16/stkinit.d 

COMPILER_OBJS += \
src/smc_gen/r_bsp/board/generic_rl78_g16/hdwinit.obj \
src/smc_gen/r_bsp/board/generic_rl78_g16/r_bsp_init.obj 

C_DEPS += \
src/smc_gen/r_bsp/board/generic_rl78_g16/hdwinit.d \
src/smc_gen/r_bsp/board/generic_rl78_g16/r_bsp_init.d 

# Each subdirectory must supply rules for building sources it contributes
src/smc_gen/r_bsp/board/generic_rl78_g16/%.obj: ../src/smc_gen/r_bsp/board/generic_rl78_g16/%.c 
	@echo 'Scanning and building file: $<'
	ccrl -subcommand="src\smc_gen\r_bsp\board\generic_rl78_g16\cDepSubCommand.tmp" -o "$(@:%.obj=%.d)" -MT="$(@:%.obj=%.obj)" -MT="$(@:%.obj=%.d)" -msg_lang=english "$<"
	ccrl -subcommand="src\smc_gen\r_bsp\board\generic_rl78_g16\cSubCommand.tmp" -msg_lang=english -o "$(@:%.d=%.obj)" "$<"


src/smc_gen/r_bsp/board/generic_rl78_g16/%.obj: ../src/smc_gen/r_bsp/board/generic_rl78_g16/%.asm 
	@echo 'Scanning and building file: $<'
	ccrl -subcommand="src\smc_gen\r_bsp\board\generic_rl78_g16\asmDepSubCommand.tmp" -asmopt=-MF="$(@:%.obj=%.d)" -asmopt=-MT="$(@:%.obj=%.obj)" -asmopt=-MT="$(@:%.obj=%.d)" -msg_lang=english "$<"
	ccrl -subcommand="src\smc_gen\r_bsp\board\generic_rl78_g16\asmSubCommand.tmp" -msg_lang=english -o "$(@:%.d=%.obj)" "$<"


