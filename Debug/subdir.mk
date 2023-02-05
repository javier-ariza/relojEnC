################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../coreWatch.c \
../fsm.c \
../kbhit.c \
../reloj.c \
../teclado_TL04.c \
../tmr.c 

C_DEPS += \
./coreWatch.d \
./fsm.d \
./kbhit.d \
./reloj.d \
./teclado_TL04.d \
./tmr.d 

OBJS += \
./coreWatch.o \
./fsm.o \
./kbhit.o \
./reloj.o \
./teclado_TL04.o \
./tmr.o 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-linux-gnueabihf-gcc -I"C:\SysGCC\Raspberry\include" -I"C:\SysGCC\Raspberry\include\wiringPi" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean--2e-

clean--2e-:
	-$(RM) ./coreWatch.d ./coreWatch.o ./fsm.d ./fsm.o ./kbhit.d ./kbhit.o ./reloj.d ./reloj.o ./teclado_TL04.d ./teclado_TL04.o ./tmr.d ./tmr.o

.PHONY: clean--2e-

