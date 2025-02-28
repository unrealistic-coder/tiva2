## THIS IS A GENERATED FILE -- DO NOT EDIT
.configuro: .libraries,em4f linker.cmd package/cfg/httpsget_pem4f.oem4f

# To simplify configuro usage in makefiles:
#     o create a generic linker command file name 
#     o set modification times of compiler.opt* files to be greater than
#       or equal to the generated config header
#
linker.cmd: package/cfg/httpsget_pem4f.xdl
	$(SED) 's"^\"\(package/cfg/httpsget_pem4fcfg.cmd\)\"$""\"C:/Users/a0321879/Documents/CCS/TM4C/My CCS EKTM4C1294XL 2.2.0/sntp_EK_TM4C1294XL_TI_TivaTM4C1294NCPDT/.config/xconfig_httpsget/\1\""' package/cfg/httpsget_pem4f.xdl > $@
	-$(SETDATE) -r:max package/cfg/httpsget_pem4f.h compiler.opt compiler.opt.defs
