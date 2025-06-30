################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/File\ nostri/number_display.c 

OBJS += \
./Core/Src/File\ nostri/number_display.o 

C_DEPS += \
./Core/Src/File\ nostri/number_display.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/File\ nostri/number_display.o: ../Core/Src/File\ nostri/number_display.c Core/Src/File\ nostri/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F303xC -c -I"/home/br1/STM32CubeIDE/workspace_1.12.0/Progetto_APC/Core/Inc/File nostri" -I../Core/Inc -I"/home/br1/STM32CubeIDE/workspace_1.12.0/Progetto_APC/Drivers/OLED" -I../Drivers/STM32F3xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F3xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F3xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"Core/Src/File nostri/number_display.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-File-20-nostri

clean-Core-2f-Src-2f-File-20-nostri:
	-$(RM) ./Core/Src/File\ nostri/number_display.cyclo ./Core/Src/File\ nostri/number_display.d ./Core/Src/File\ nostri/number_display.o ./Core/Src/File\ nostri/number_display.su

.PHONY: clean-Core-2f-Src-2f-File-20-nostri

