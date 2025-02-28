################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
third_party/lwip-2.2.1/src/apps/http/%.obj: ../third_party/lwip-2.2.1/src/apps/http/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs1031/ccs/tools/compiler/ti-cgt-arm_20.2.4.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/Users/14169/workspace_v10_1/enet_lwip_upgraded" --include_path="D:/tivaware/third_party/lwip-2.2.1/ports/tiva-tm4c129/include" --include_path="D:/tivaware/third_party/lwip-2.2.1/src/include" --include_path="D:/tivaware/third_party/lwip-2.2.1/src/apps" --include_path="C:/Users/14169/workspace_v10_1/enet_lwip_upgraded/third_party" --include_path="D:/tivaware" --include_path="C:/ti/ccs1031/ccs/tools/compiler/ti-cgt-arm_20.2.4.LTS/include" --define=ccs="ccs" --define=PART_TM4C1294NCPDT -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="third_party/lwip-2.2.1/src/apps/http/$(basename $(<F)).d_raw" --obj_directory="third_party/lwip-2.2.1/src/apps/http" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


