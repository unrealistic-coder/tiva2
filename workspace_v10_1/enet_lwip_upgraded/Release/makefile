################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

CG_TOOL_ROOT := C:/ti/ccs1031/ccs/tools/compiler/ti-cgt-arm_20.2.4.LTS

GEN_OPTS__FLAG := 
GEN_CMDS__FLAG := 

ORDERED_OBJS += \
"./enet_fs_updated.obj" \
"./enet_lwip.obj" \
"./fsdata.obj" \
"./tm4c1294ncpdt_startup_ccs.obj" \
"./drivers/pinout.obj" \
"./lib/emac.obj" \
"./third_party/fatfs/SST26VF032B.obj" \
"./third_party/fatfs/source/ff.obj" \
"./third_party/fatfs/source/ffsystem.obj" \
"./third_party/fatfs/source/ffunicode.obj" \
"./third_party/fatfs/source/user_diskio.obj" \
"./third_party/lwip-2.2.1/src/apps/http/httpd.obj" \
"./utils/locator.obj" \
"./utils/lwiplib.obj" \
"./utils/uartstdio.obj" \
"./utils/ustdlib.obj" \
"../tm4c1294ncpdt.cmd" \
$(GEN_CMDS__FLAG) \
-llibc.a \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include drivers/subdir_vars.mk
-include lib/subdir_vars.mk
-include third_party/fatfs/subdir_vars.mk
-include third_party/fatfs/source/subdir_vars.mk
-include third_party/lwip-2.2.1/src/apps/http/subdir_vars.mk
-include utils/subdir_vars.mk
-include subdir_rules.mk
-include drivers/subdir_rules.mk
-include lib/subdir_rules.mk
-include third_party/fatfs/subdir_rules.mk
-include third_party/fatfs/source/subdir_rules.mk
-include third_party/lwip-2.2.1/src/apps/http/subdir_rules.mk
-include utils/subdir_rules.mk
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
enet_lwip_upgraded.out \

EXE_OUTPUTS__QUOTED += \
"enet_lwip_upgraded.out" \

BIN_OUTPUTS += \
enet_lwip_upgraded.hex \

BIN_OUTPUTS__QUOTED += \
"enet_lwip_upgraded.hex" \


# All Target
all: $(OBJS) $(CMD_SRCS) $(GEN_CMDS)
	@$(MAKE) --no-print-directory -Onone "enet_lwip_upgraded.out"

# Tool invocations
enet_lwip_upgraded.out: $(OBJS) $(CMD_SRCS) $(GEN_CMDS)
	@echo 'Building target: "$@"'
	@echo 'Invoking: Arm Linker'
	"C:/ti/ccs1031/ccs/tools/compiler/ti-cgt-arm_20.2.4.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -O2 --define=ccs="ccs" --define=PART_TM4C1294NCPDT --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi -z -m"enet_lwip_upgraded.map" --heap_size=0 --stack_size=512 -i"C:/ti/ccs1031/ccs/tools/compiler/ti-cgt-arm_20.2.4.LTS/lib" -i"C:/ti/ccs1031/ccs/tools/compiler/ti-cgt-arm_20.2.4.LTS/include" --reread_libs --diag_wrap=off --display_error_number --warn_sections --xml_link_info="enet_lwip_upgraded_linkInfo.xml" --rom_model -o "enet_lwip_upgraded.out" $(ORDERED_OBJS)
	@echo 'Finished building target: "$@"'
	@echo ' '

enet_lwip_upgraded.hex: $(EXE_OUTPUTS)
	@echo 'Building secondary target: "$@"'
	@echo 'Invoking: Arm Hex Utility'
	"C:/ti/ccs1031/ccs/tools/compiler/ti-cgt-arm_20.2.4.LTS/bin/armhex" --diag_wrap=off -o "enet_lwip_upgraded.hex" $(EXE_OUTPUTS__QUOTED)
	@echo 'Finished building secondary target: "$@"'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(BIN_OUTPUTS__QUOTED)$(EXE_OUTPUTS__QUOTED)
	-$(RM) "enet_fs_updated.obj" "enet_lwip.obj" "fsdata.obj" "tm4c1294ncpdt_startup_ccs.obj" "drivers\pinout.obj" "lib\emac.obj" "third_party\fatfs\SST26VF032B.obj" "third_party\fatfs\source\ff.obj" "third_party\fatfs\source\ffsystem.obj" "third_party\fatfs\source\ffunicode.obj" "third_party\fatfs\source\user_diskio.obj" "third_party\lwip-2.2.1\src\apps\http\httpd.obj" "utils\locator.obj" "utils\lwiplib.obj" "utils\uartstdio.obj" "utils\ustdlib.obj" 
	-$(RM) "enet_fs_updated.d" "enet_lwip.d" "fsdata.d" "tm4c1294ncpdt_startup_ccs.d" "drivers\pinout.d" "lib\emac.d" "third_party\fatfs\SST26VF032B.d" "third_party\fatfs\source\ff.d" "third_party\fatfs\source\ffsystem.d" "third_party\fatfs\source\ffunicode.d" "third_party\fatfs\source\user_diskio.d" "third_party\lwip-2.2.1\src\apps\http\httpd.d" "utils\locator.d" "utils\lwiplib.d" "utils\uartstdio.d" "utils\ustdlib.d" 
	-@echo 'Finished clean'
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

