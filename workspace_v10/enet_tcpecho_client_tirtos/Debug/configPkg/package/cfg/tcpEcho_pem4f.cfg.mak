# invoke SourceDir generated makefile for tcpEcho.pem4f
tcpEcho.pem4f: .libraries,tcpEcho.pem4f
.libraries,tcpEcho.pem4f: package/cfg/tcpEcho_pem4f.xdl
	$(MAKE) -f C:\Users\a0321879\DOCUME~1\MYCCSE~2\enet_tcpecho_client_tirtos/src/makefile.libs

clean::
	$(MAKE) -f C:\Users\a0321879\DOCUME~1\MYCCSE~2\enet_tcpecho_client_tirtos/src/makefile.libs clean

