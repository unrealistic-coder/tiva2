# invoke SourceDir generated makefile for httpServer.pem4f
httpServer.pem4f: .libraries,httpServer.pem4f
.libraries,httpServer.pem4f: package/cfg/httpServer_pem4f.xdl
	$(MAKE) -f C:\Users\14169\workspace_v10\enet_httpServer_tirtos/src/makefile.libs

clean::
	$(MAKE) -f C:\Users\14169\workspace_v10\enet_httpServer_tirtos/src/makefile.libs clean

