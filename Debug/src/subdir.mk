################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/ATcmd_object.c \
../src/DefaultFonts.c \
../src/FSMCDriver.c \
../src/delay.c \
../src/lcd_object.c \
../src/main.c \
../src/stm32f10x_fsmc.c \
../src/stm32f10x_usart.c \
../src/uart_object.c 

OBJS += \
./src/ATcmd_object.o \
./src/DefaultFonts.o \
./src/FSMCDriver.o \
./src/delay.o \
./src/lcd_object.o \
./src/main.o \
./src/stm32f10x_fsmc.o \
./src/stm32f10x_usart.o \
./src/uart_object.o 

C_DEPS += \
./src/ATcmd_object.d \
./src/DefaultFonts.d \
./src/FSMCDriver.d \
./src/delay.d \
./src/lcd_object.d \
./src/main.d \
./src/stm32f10x_fsmc.d \
./src/stm32f10x_usart.d \
./src/uart_object.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal  -g3 -DDEBUG -DUSE_FULL_ASSERT -DTRACE -DOS_USE_TRACE_ITM -DSTM32F10X_HD -DUSE_STDPERIPH_DRIVER -DHSE_VALUE=8000000 -I"../include" -I"../system/include" -I"../system/include/cmsis" -I"../system/include/stm32f1-stdperiph" -std=gnu11 -Wmissing-prototypes -Wstrict-prototypes -Wbad-function-cast -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


