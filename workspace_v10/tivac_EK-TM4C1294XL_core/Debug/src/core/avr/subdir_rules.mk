################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
src/core/avr/dtostrf.o: C:/Users/14169/AppData/Local/Energia15/packages/energia/hardware/tivac/1.0.4/cores/tivac/avr/dtostrf.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: GNU Compiler'
	"C:/Users/14169/AppData/Local/Energia15/packages/energia/tools/arm-none-eabi-gcc/8.3.1-20190703/bin/arm-none-eabi-gcc-8.3.1.exe" -c -mcpu=cortex-m4 -march=armv7e-m -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Dprintf=iprintf -DF_CPU=120000000L -DENERGIA_EK_TM4C1294XL -DENERGIA_ARCH_TIVAC -DENERGIA=23 -DARDUINO=10610 -I"C:/Users/14169/AppData/Local/Energia15/packages/energia/hardware/tivac/1.0.4/system/driverlib" -I"C:/Users/14169/AppData/Local/Energia15/packages/energia/hardware/tivac/1.0.4/system/inc" -I"C:/Users/14169/AppData/Local/Energia15/packages/energia/hardware/tivac/1.0.4/system" -I"C:/Users/14169/AppData/Local/Energia15/packages/energia/hardware/tivac/1.0.4/cores/tivac" -I"C:/Users/14169/AppData/Local/Energia15/packages/energia/hardware/tivac/1.0.4/variants/EK-TM4C1294XL" -I"C:/Users/14169/workspace_v10/tivac_EK-TM4C1294XL_core" -I"C:/Users/14169/AppData/Local/Energia15/packages/energia/tools/arm-none-eabi-gcc/8.3.1-20190703/arm-none-eabi/include" -Os -ffunction-sections -fdata-sections -g -gdwarf-3 -gstrict-dwarf -w -Wall --param max-inline-insns-single=500 -mabi=aapcs -MMD -MP -MF"src/core/avr/$(basename $(<F)).d_raw" -MT"$(@)" -std=gnu11 $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


