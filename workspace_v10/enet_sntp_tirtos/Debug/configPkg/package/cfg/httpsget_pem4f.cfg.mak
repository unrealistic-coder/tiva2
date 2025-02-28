# invoke SourceDir generated makefile for httpsget.pem4f
httpsget.pem4f: .libraries,httpsget.pem4f
.libraries,httpsget.pem4f: package/cfg/httpsget_pem4f.xdl
	$(MAKE) -f C:\Users\14169\workspace_v10\enet_sntp_tirtos/src/makefile.libs

clean::
	$(MAKE) -f C:\Users\14169\workspace_v10\enet_sntp_tirtos/src/makefile.libs clean

