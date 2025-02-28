################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
boot_loader/bl_autobaud.obj: D:/tivaware/boot_loader/bl_autobaud.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccs1031/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -O2 --include_path="C:/Users/14169/workspace_v10_2/boot_emac_flash" --include_path="C:/Users/14169/workspace_v10_2/boot_emac_flash" --include_path="D:/tivaware" --include_path="D:/tivaware/boot_loader" --include_path="C:/ti/ccs1031/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/include" --define=ccs="ccs" --define=PART_TM4C1294NCPDT --define=TARGET_IS_TM4C129_RA2 -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --ual --preproc_with_compile --preproc_dependency="boot_loader/$(basename $(<F)).d_raw" --obj_directory="boot_loader" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

boot_loader/bl_can.obj: D:/tivaware/boot_loader/bl_can.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccs1031/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -O2 --include_path="C:/Users/14169/workspace_v10_2/boot_emac_flash" --include_path="C:/Users/14169/workspace_v10_2/boot_emac_flash" --include_path="D:/tivaware" --include_path="D:/tivaware/boot_loader" --include_path="C:/ti/ccs1031/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/include" --define=ccs="ccs" --define=PART_TM4C1294NCPDT --define=TARGET_IS_TM4C129_RA2 -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --ual --preproc_with_compile --preproc_dependency="boot_loader/$(basename $(<F)).d_raw" --obj_directory="boot_loader" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

boot_loader/bl_check.obj: D:/tivaware/boot_loader/bl_check.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccs1031/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -O2 --include_path="C:/Users/14169/workspace_v10_2/boot_emac_flash" --include_path="C:/Users/14169/workspace_v10_2/boot_emac_flash" --include_path="D:/tivaware" --include_path="D:/tivaware/boot_loader" --include_path="C:/ti/ccs1031/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/include" --define=ccs="ccs" --define=PART_TM4C1294NCPDT --define=TARGET_IS_TM4C129_RA2 -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --ual --preproc_with_compile --preproc_dependency="boot_loader/$(basename $(<F)).d_raw" --obj_directory="boot_loader" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

boot_loader/bl_crc32.obj: D:/tivaware/boot_loader/bl_crc32.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccs1031/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -O2 --include_path="C:/Users/14169/workspace_v10_2/boot_emac_flash" --include_path="C:/Users/14169/workspace_v10_2/boot_emac_flash" --include_path="D:/tivaware" --include_path="D:/tivaware/boot_loader" --include_path="C:/ti/ccs1031/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/include" --define=ccs="ccs" --define=PART_TM4C1294NCPDT --define=TARGET_IS_TM4C129_RA2 -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --ual --preproc_with_compile --preproc_dependency="boot_loader/$(basename $(<F)).d_raw" --obj_directory="boot_loader" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

boot_loader/bl_decrypt.obj: D:/tivaware/boot_loader/bl_decrypt.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccs1031/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -O2 --include_path="C:/Users/14169/workspace_v10_2/boot_emac_flash" --include_path="C:/Users/14169/workspace_v10_2/boot_emac_flash" --include_path="D:/tivaware" --include_path="D:/tivaware/boot_loader" --include_path="C:/ti/ccs1031/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/include" --define=ccs="ccs" --define=PART_TM4C1294NCPDT --define=TARGET_IS_TM4C129_RA2 -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --ual --preproc_with_compile --preproc_dependency="boot_loader/$(basename $(<F)).d_raw" --obj_directory="boot_loader" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

boot_loader/bl_emac.obj: D:/tivaware/boot_loader/bl_emac.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccs1031/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -O2 --include_path="C:/Users/14169/workspace_v10_2/boot_emac_flash" --include_path="C:/Users/14169/workspace_v10_2/boot_emac_flash" --include_path="D:/tivaware" --include_path="D:/tivaware/boot_loader" --include_path="C:/ti/ccs1031/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/include" --define=ccs="ccs" --define=PART_TM4C1294NCPDT --define=TARGET_IS_TM4C129_RA2 -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --ual --preproc_with_compile --preproc_dependency="boot_loader/$(basename $(<F)).d_raw" --obj_directory="boot_loader" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

boot_loader/bl_flash.obj: D:/tivaware/boot_loader/bl_flash.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccs1031/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -O2 --include_path="C:/Users/14169/workspace_v10_2/boot_emac_flash" --include_path="C:/Users/14169/workspace_v10_2/boot_emac_flash" --include_path="D:/tivaware" --include_path="D:/tivaware/boot_loader" --include_path="C:/ti/ccs1031/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/include" --define=ccs="ccs" --define=PART_TM4C1294NCPDT --define=TARGET_IS_TM4C129_RA2 -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --ual --preproc_with_compile --preproc_dependency="boot_loader/$(basename $(<F)).d_raw" --obj_directory="boot_loader" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

boot_loader/bl_i2c.obj: D:/tivaware/boot_loader/bl_i2c.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccs1031/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -O2 --include_path="C:/Users/14169/workspace_v10_2/boot_emac_flash" --include_path="C:/Users/14169/workspace_v10_2/boot_emac_flash" --include_path="D:/tivaware" --include_path="D:/tivaware/boot_loader" --include_path="C:/ti/ccs1031/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/include" --define=ccs="ccs" --define=PART_TM4C1294NCPDT --define=TARGET_IS_TM4C129_RA2 -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --ual --preproc_with_compile --preproc_dependency="boot_loader/$(basename $(<F)).d_raw" --obj_directory="boot_loader" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

boot_loader/bl_main.obj: D:/tivaware/boot_loader/bl_main.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccs1031/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -O2 --include_path="C:/Users/14169/workspace_v10_2/boot_emac_flash" --include_path="C:/Users/14169/workspace_v10_2/boot_emac_flash" --include_path="D:/tivaware" --include_path="D:/tivaware/boot_loader" --include_path="C:/ti/ccs1031/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/include" --define=ccs="ccs" --define=PART_TM4C1294NCPDT --define=TARGET_IS_TM4C129_RA2 -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --ual --preproc_with_compile --preproc_dependency="boot_loader/$(basename $(<F)).d_raw" --obj_directory="boot_loader" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

boot_loader/bl_packet.obj: D:/tivaware/boot_loader/bl_packet.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccs1031/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -O2 --include_path="C:/Users/14169/workspace_v10_2/boot_emac_flash" --include_path="C:/Users/14169/workspace_v10_2/boot_emac_flash" --include_path="D:/tivaware" --include_path="D:/tivaware/boot_loader" --include_path="C:/ti/ccs1031/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/include" --define=ccs="ccs" --define=PART_TM4C1294NCPDT --define=TARGET_IS_TM4C129_RA2 -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --ual --preproc_with_compile --preproc_dependency="boot_loader/$(basename $(<F)).d_raw" --obj_directory="boot_loader" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

boot_loader/bl_ssi.obj: D:/tivaware/boot_loader/bl_ssi.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccs1031/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -O2 --include_path="C:/Users/14169/workspace_v10_2/boot_emac_flash" --include_path="C:/Users/14169/workspace_v10_2/boot_emac_flash" --include_path="D:/tivaware" --include_path="D:/tivaware/boot_loader" --include_path="C:/ti/ccs1031/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/include" --define=ccs="ccs" --define=PART_TM4C1294NCPDT --define=TARGET_IS_TM4C129_RA2 -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --ual --preproc_with_compile --preproc_dependency="boot_loader/$(basename $(<F)).d_raw" --obj_directory="boot_loader" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

boot_loader/bl_startup_ccs.obj: D:/tivaware/boot_loader/bl_startup_ccs.s $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccs1031/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -O2 --include_path="C:/Users/14169/workspace_v10_2/boot_emac_flash" --include_path="C:/Users/14169/workspace_v10_2/boot_emac_flash" --include_path="D:/tivaware" --include_path="D:/tivaware/boot_loader" --include_path="C:/ti/ccs1031/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/include" --define=ccs="ccs" --define=PART_TM4C1294NCPDT --define=TARGET_IS_TM4C129_RA2 -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --ual --preproc_with_compile --preproc_dependency="boot_loader/$(basename $(<F)).d_raw" --obj_directory="boot_loader" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

boot_loader/bl_uart.obj: D:/tivaware/boot_loader/bl_uart.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccs1031/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -O2 --include_path="C:/Users/14169/workspace_v10_2/boot_emac_flash" --include_path="C:/Users/14169/workspace_v10_2/boot_emac_flash" --include_path="D:/tivaware" --include_path="D:/tivaware/boot_loader" --include_path="C:/ti/ccs1031/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/include" --define=ccs="ccs" --define=PART_TM4C1294NCPDT --define=TARGET_IS_TM4C129_RA2 -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --ual --preproc_with_compile --preproc_dependency="boot_loader/$(basename $(<F)).d_raw" --obj_directory="boot_loader" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

boot_loader/bl_usb.obj: D:/tivaware/boot_loader/bl_usb.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccs1031/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -O2 --include_path="C:/Users/14169/workspace_v10_2/boot_emac_flash" --include_path="C:/Users/14169/workspace_v10_2/boot_emac_flash" --include_path="D:/tivaware" --include_path="D:/tivaware/boot_loader" --include_path="C:/ti/ccs1031/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/include" --define=ccs="ccs" --define=PART_TM4C1294NCPDT --define=TARGET_IS_TM4C129_RA2 -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --ual --preproc_with_compile --preproc_dependency="boot_loader/$(basename $(<F)).d_raw" --obj_directory="boot_loader" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

boot_loader/bl_usbfuncs.obj: D:/tivaware/boot_loader/bl_usbfuncs.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccs1031/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -O2 --include_path="C:/Users/14169/workspace_v10_2/boot_emac_flash" --include_path="C:/Users/14169/workspace_v10_2/boot_emac_flash" --include_path="D:/tivaware" --include_path="D:/tivaware/boot_loader" --include_path="C:/ti/ccs1031/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/include" --define=ccs="ccs" --define=PART_TM4C1294NCPDT --define=TARGET_IS_TM4C129_RA2 -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --ual --preproc_with_compile --preproc_dependency="boot_loader/$(basename $(<F)).d_raw" --obj_directory="boot_loader" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


