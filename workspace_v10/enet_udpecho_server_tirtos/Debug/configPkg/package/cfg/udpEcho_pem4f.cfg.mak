# invoke SourceDir generated makefile for udpEcho.pem4f
udpEcho.pem4f: .libraries,udpEcho.pem4f
.libraries,udpEcho.pem4f: package/cfg/udpEcho_pem4f.xdl
	$(MAKE) -f C:\Users\a0321879\DOCUME~1\MYCCSE~2\enet_udpecho_server_tirtos/src/makefile.libs

clean::
	$(MAKE) -f C:\Users\a0321879\DOCUME~1\MYCCSE~2\enet_udpecho_server_tirtos/src/makefile.libs clean

