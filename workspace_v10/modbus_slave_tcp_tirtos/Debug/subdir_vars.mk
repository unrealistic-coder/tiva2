################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Add inputs and outputs from these tool invocations to the build variables 
CFG_SRCS += \
../tcpEcho.cfg 

CMD_SRCS += \
../EK_TM4C1294XL.cmd 

C_SRCS += \
../EK_TM4C1294XL.c \
../modbus_ti_server_final.c \
../tcpEchoHooks.c \
../tcpEchoServer.c 

GEN_CMDS += \
./configPkg/linker.cmd 

GEN_FILES += \
./configPkg/linker.cmd \
./configPkg/compiler.opt 

GEN_MISC_DIRS += \
./configPkg/ 

C_DEPS += \
./EK_TM4C1294XL.d \
./modbus_ti_server_final.d \
./tcpEchoHooks.d \
./tcpEchoServer.d 

GEN_OPTS += \
./configPkg/compiler.opt 

OBJS += \
./EK_TM4C1294XL.obj \
./modbus_ti_server_final.obj \
./tcpEchoHooks.obj \
./tcpEchoServer.obj 

GEN_MISC_DIRS__QUOTED += \
"configPkg\" 

OBJS__QUOTED += \
"EK_TM4C1294XL.obj" \
"modbus_ti_server_final.obj" \
"tcpEchoHooks.obj" \
"tcpEchoServer.obj" 

C_DEPS__QUOTED += \
"EK_TM4C1294XL.d" \
"modbus_ti_server_final.d" \
"tcpEchoHooks.d" \
"tcpEchoServer.d" 

GEN_FILES__QUOTED += \
"configPkg\linker.cmd" \
"configPkg\compiler.opt" 

C_SRCS__QUOTED += \
"../EK_TM4C1294XL.c" \
"../modbus_ti_server_final.c" \
"../tcpEchoHooks.c" \
"../tcpEchoServer.c" 


