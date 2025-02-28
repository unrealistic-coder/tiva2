# invoke SourceDir generated makefile for tcpEcho.pem4f
tcpEcho.pem4f: .libraries,tcpEcho.pem4f
.libraries,tcpEcho.pem4f: package/cfg/tcpEcho_pem4f.xdl
	$(MAKE) -f C:\Users\14169\workspace_v10\modbus_master_tcp_tirtos/src/makefile.libs

clean::
	$(MAKE) -f C:\Users\14169\workspace_v10\modbus_master_tcp_tirtos/src/makefile.libs clean

