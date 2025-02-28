################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
FREERTOS_2022/MemMang/%.obj: ../FREERTOS_2022/MemMang/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs1031/ccs/tools/compiler/ti-cgt-arm_20.2.4.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/Users/14169/workspace_v10_1/freertos_tcp" --include_path="C:/Users/14169/Downloads/FreeRTOSv202212.01/FreeRTOSv202212.01/FreeRTOS-Plus/Demo/FreeRTOS_Plus_TCP_Echo_Posix/Trace_Recorder_Configuration" --include_path="C:/Users/14169/Downloads/FreeRTOSv202212.01/FreeRTOSv202212.01/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/source/portable/NetworkInterface/include" --include_path="C:/Users/14169/Downloads/FreeRTOSv202212.01/FreeRTOSv202212.01/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/source/portable/Compiler/CCS" --include_path="C:/Users/14169/Downloads/FreeRTOS-Plus-TCP-V3.1.0/FreeRTOS-Plus-TCP/source/include" --include_path="C:/Users/14169/Downloads/FreeRTOSv202212.01/FreeRTOSv202212.01/FreeRTOS/Source/include" --include_path="C:/Users/14169/Downloads/FreeRTOSv202212.01/FreeRTOSv202212.01/FreeRTOS/Source/portable/CCS/ARM_CM4F" --include_path="C:/ti/TivaWare_C_Series-2.2.0.295" --include_path="C:/ti/ccs1031/ccs/tools/compiler/ti-cgt-arm_20.2.4.LTS/include" --define=ccs="ccs" --define=PART_TM4C1294NCPDT -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="FREERTOS_2022/MemMang/$(basename $(<F)).d_raw" --obj_directory="FREERTOS_2022/MemMang" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


