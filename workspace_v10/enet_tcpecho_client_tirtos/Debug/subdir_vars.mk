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
../tcpEchoClient.c \
../tcpEchoHooks.c 

GEN_CMDS += \
./configPkg/linker.cmd 

GEN_FILES += \
./configPkg/linker.cmd \
./configPkg/compiler.opt 

GEN_MISC_DIRS += \
./configPkg/ 

C_DEPS += \
./EK_TM4C1294XL.d \
./tcpEchoClient.d \
./tcpEchoHooks.d 

GEN_OPTS += \
./configPkg/compiler.opt 

OBJS += \
./EK_TM4C1294XL.obj \
./tcpEchoClient.obj \
./tcpEchoHooks.obj 

GEN_MISC_DIRS__QUOTED += \
"configPkg\" 

OBJS__QUOTED += \
"EK_TM4C1294XL.obj" \
"tcpEchoClient.obj" \
"tcpEchoHooks.obj" 

C_DEPS__QUOTED += \
"EK_TM4C1294XL.d" \
"tcpEchoClient.d" \
"tcpEchoHooks.d" 

GEN_FILES__QUOTED += \
"configPkg\linker.cmd" \
"configPkg\compiler.opt" 

C_SRCS__QUOTED += \
"../EK_TM4C1294XL.c" \
"../tcpEchoClient.c" \
"../tcpEchoHooks.c" 


