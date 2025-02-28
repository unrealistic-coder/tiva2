# invoke SourceDir generated makefile for httpget.pem4f
httpget.pem4f: .libraries,httpget.pem4f
.libraries,httpget.pem4f: package/cfg/httpget_pem4f.xdl
	$(MAKE) -f C:\Users\14169\workspace_v10\enet_httpget_tirtos/src/makefile.libs

clean::
	$(MAKE) -f C:\Users\14169\workspace_v10\enet_httpget_tirtos/src/makefile.libs clean

