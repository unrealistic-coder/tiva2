################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
%.obj: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccs1031/ccs/tools/compiler/ti-cgt-arm_20.2.4.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/Users/14169/workspace_v10/enet_udpecho_client_tirtos" --include_path="/products/ndk_2_25_00_09/packages/ti/ndk/inc/bsd" --include_path="C:/Users/14169/workspace_v10/enet_udpecho_client_tirtos" --include_path="/products/TivaWare_C_Series-2.1.1.71b" --include_path="/products/bios_6_45_01_29/packages/ti/sysbios/posix" --include_path="C:/ti/ccs1031/ccs/tools/compiler/ti-cgt-arm_20.2.4.LTS/include" --define=ccs="ccs" --define=PART_TM4C1294NCPDT --define=ccs --define=TIVAWARE -g --gcc --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

build-930023681:
	@$(MAKE) --no-print-directory -Onone -f subdir_rules.mk build-930023681-inproc

build-930023681-inproc: ../udpEcho.cfg
	@echo 'Building file: "$<"'
	@echo 'Invoking: XDCtools'
	"C:/ti/ccs1031/xdctools_3_62_00_08_core/xs" --xdcpath= xdc.tools.configuro -o configPkg -t ti.targets.arm.elf.M4F -p ti.platforms.tiva:TM4C1294NCPDT -r release -c "C:/ti/ccs1031/ccs/tools/compiler/ti-cgt-arm_20.2.4.LTS" --compileOptions "-mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path=\"C:/Users/14169/workspace_v10/enet_udpecho_client_tirtos\" --include_path=\"/products/ndk_2_25_00_09/packages/ti/ndk/inc/bsd\" --include_path=\"C:/Users/14169/workspace_v10/enet_udpecho_client_tirtos\" --include_path=\"/products/TivaWare_C_Series-2.1.1.71b\" --include_path=\"/products/bios_6_45_01_29/packages/ti/sysbios/posix\" --include_path=\"C:/ti/ccs1031/ccs/tools/compiler/ti-cgt-arm_20.2.4.LTS/include\" --define=ccs=\"ccs\" --define=PART_TM4C1294NCPDT --define=ccs --define=TIVAWARE -g --gcc --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi  " "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

configPkg/linker.cmd: build-930023681 ../udpEcho.cfg
configPkg/compiler.opt: build-930023681
configPkg/: build-930023681


