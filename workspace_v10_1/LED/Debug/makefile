################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

CG_TOOL_ROOT := C:/ti/ccs1031/ccs/tools/compiler/ti-cgt-arm_20.2.4.LTS

GEN_OPTS__FLAG := 
GEN_CMDS__FLAG := 

ORDERED_OBJS += \
"./led_blink.obj" \
"./tm4c1294ncpdt_startup_ccs.obj" \
"./driverlib/adc.obj" \
"./driverlib/aes.obj" \
"./driverlib/can.obj" \
"./driverlib/comp.obj" \
"./driverlib/cpu.obj" \
"./driverlib/crc.obj" \
"./driverlib/des.obj" \
"./driverlib/eeprom.obj" \
"./driverlib/emac.obj" \
"./driverlib/epi.obj" \
"./driverlib/epi_workaround_ccs.obj" \
"./driverlib/flash.obj" \
"./driverlib/fpu.obj" \
"./driverlib/gpio.obj" \
"./driverlib/hibernate.obj" \
"./driverlib/i2c.obj" \
"./driverlib/interrupt.obj" \
"./driverlib/lcd.obj" \
"./driverlib/mpu.obj" \
"./driverlib/pwm.obj" \
"./driverlib/qei.obj" \
"./driverlib/shamd5.obj" \
"./driverlib/ssi.obj" \
"./driverlib/sw_crc.obj" \
"./driverlib/sysctl.obj" \
"./driverlib/sysexc.obj" \
"./driverlib/systick.obj" \
"./driverlib/timer.obj" \
"./driverlib/uart.obj" \
"./driverlib/udma.obj" \
"./driverlib/usb.obj" \
"./driverlib/watchdog.obj" \
"../tm4c1294ncpdt.cmd" \
"../driverlib/ccs/Debug/driverlib.lib" \
"../driverlib/rvmdk/driverlib.lib" \
$(GEN_CMDS__FLAG) \
-llibc.a \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include driverlib/subdir_vars.mk
-include driverlib/ccs/Debug/subdir_vars.mk
-include driverlib/ewarm/Exe/subdir_vars.mk
-include driverlib/gcc/subdir_vars.mk
-include driverlib/rvmdk/subdir_vars.mk
-include subdir_rules.mk
-include driverlib/subdir_rules.mk
-include driverlib/ccs/Debug/subdir_rules.mk
-include driverlib/ewarm/Exe/subdir_rules.mk
-include driverlib/gcc/subdir_rules.mk
-include driverlib/rvmdk/subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(INO_DEPS)),)
-include $(INO_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(SV7A_DEPS)),)
-include $(SV7A_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(PDE_DEPS)),)
-include $(PDE_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXE_OUTPUTS += \
LED.out \

EXE_OUTPUTS__QUOTED += \
"LED.out" \

BIN_OUTPUTS += \
LED.hex \

BIN_OUTPUTS__QUOTED += \
"LED.hex" \


# All Target
all: $(OBJS) $(CMD_SRCS) $(LIB_SRCS) $(A_SRCS) $(GEN_CMDS)
	@$(MAKE) --no-print-directory -Onone "LED.out" secondary-outputs

# Tool invocations
LED.out: $(OBJS) $(CMD_SRCS) $(LIB_SRCS) $(A_SRCS) $(GEN_CMDS)
	@echo 'Building target: "$@"'
	@echo 'Invoking: Arm Linker'
	"C:/ti/ccs1031/ccs/tools/compiler/ti-cgt-arm_20.2.4.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --define=ccs="ccs" --define=PART_TM4C1294NCPDT -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi -z -m"LED.map" --heap_size=0 --stack_size=512 -i"C:/ti/ccs1031/ccs/tools/compiler/ti-cgt-arm_20.2.4.LTS/lib" -i"C:/ti/ccs1031/ccs/tools/compiler/ti-cgt-arm_20.2.4.LTS/include" --reread_libs --diag_wrap=off --display_error_number --warn_sections --xml_link_info="LED_linkInfo.xml" --rom_model -o "LED.out" $(ORDERED_OBJS) $(A_SRCS)
	@echo 'Finished building target: "$@"'
	@echo ' '

LED.hex: $(EXE_OUTPUTS)
	@echo 'Building secondary target: "$@"'
	@echo 'Invoking: Arm Hex Utility'
	"C:/ti/ccs1031/ccs/tools/compiler/ti-cgt-arm_20.2.4.LTS/bin/armhex" --diag_wrap=off -o "LED.hex" $(EXE_OUTPUTS__QUOTED)
	@echo 'Finished building secondary target: "$@"'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(BIN_OUTPUTS__QUOTED)$(EXE_OUTPUTS__QUOTED)
	-$(RM) "led_blink.obj" "tm4c1294ncpdt_startup_ccs.obj" "driverlib\adc.obj" "driverlib\aes.obj" "driverlib\can.obj" "driverlib\comp.obj" "driverlib\cpu.obj" "driverlib\crc.obj" "driverlib\des.obj" "driverlib\eeprom.obj" "driverlib\emac.obj" "driverlib\epi.obj" "driverlib\epi_workaround_ccs.obj" "driverlib\flash.obj" "driverlib\fpu.obj" "driverlib\gpio.obj" "driverlib\hibernate.obj" "driverlib\i2c.obj" "driverlib\interrupt.obj" "driverlib\lcd.obj" "driverlib\mpu.obj" "driverlib\pwm.obj" "driverlib\qei.obj" "driverlib\shamd5.obj" "driverlib\ssi.obj" "driverlib\sw_crc.obj" "driverlib\sysctl.obj" "driverlib\sysexc.obj" "driverlib\systick.obj" "driverlib\timer.obj" "driverlib\uart.obj" "driverlib\udma.obj" "driverlib\usb.obj" "driverlib\watchdog.obj" 
	-$(RM) "led_blink.d" "tm4c1294ncpdt_startup_ccs.d" "driverlib\adc.d" "driverlib\aes.d" "driverlib\can.d" "driverlib\comp.d" "driverlib\cpu.d" "driverlib\crc.d" "driverlib\des.d" "driverlib\eeprom.d" "driverlib\emac.d" "driverlib\epi.d" "driverlib\flash.d" "driverlib\fpu.d" "driverlib\gpio.d" "driverlib\hibernate.d" "driverlib\i2c.d" "driverlib\interrupt.d" "driverlib\lcd.d" "driverlib\mpu.d" "driverlib\pwm.d" "driverlib\qei.d" "driverlib\shamd5.d" "driverlib\ssi.d" "driverlib\sw_crc.d" "driverlib\sysctl.d" "driverlib\sysexc.d" "driverlib\systick.d" "driverlib\timer.d" "driverlib\uart.d" "driverlib\udma.d" "driverlib\usb.d" "driverlib\watchdog.d" 
	-$(RM) "driverlib\epi_workaround_ccs.d" 
	-@echo 'Finished clean'
	-@echo ' '

secondary-outputs: $(BIN_OUTPUTS)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

