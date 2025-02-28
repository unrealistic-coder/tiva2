################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
%.obj: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs1031/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -O2 --include_path="C:/Users/14169/workspace_v10_2/boot_demo_emac_flash" --include_path="C:/Users/14169/workspace_v10_2/boot_demo_emac_flash" --include_path="D:/tivaware/examples/boards/ek-tm4c1294xl" --include_path="D:/tivaware" --include_path="D:/tivaware/third_party/lwip-1.4.1/src/include" --include_path="D:/tivaware/third_party/lwip-1.4.1/src/include/ipv4" --include_path="D:/tivaware/third_party/lwip-1.4.1/ports/tiva-tm4c129/include" --include_path="C:/ti/ccs1031/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/include" --define=ccs="ccs" --define=PART_TM4C1294NCPDT --define=TARGET_IS_TM4C129_RA2 --define=USE_FLASH_BOOT_LOADER -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --ual --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


