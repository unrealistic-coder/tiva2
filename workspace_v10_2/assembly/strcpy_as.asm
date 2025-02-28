 .global asmfunc
 .global gvar
asmfunc:
 LDR r1, gvar_a
 LDR r2, [r1, #0]
 ADD r0, r0, r2
 STR r0, [r1, #0]
 MOV pc, lr
gvar_a .field gvar, 32
