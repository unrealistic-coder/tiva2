# invoke SourceDir generated makefile for tcpEcho.pem4f
tcpEcho.pem4f: .libraries,tcpEcho.pem4f
.libraries,tcpEcho.pem4f: package/cfg/tcpEcho_pem4f.xdl
	$(MAKE) -f C:\Users\14169\workspace_v10_1\webserver_FS/src/makefile.libs

clean::
	$(MAKE) -f C:\Users\14169\workspace_v10_1\webserver_FS/src/makefile.libs clean

