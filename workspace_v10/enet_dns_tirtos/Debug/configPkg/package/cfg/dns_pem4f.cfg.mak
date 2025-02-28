# invoke SourceDir generated makefile for dns.pem4f
dns.pem4f: .libraries,dns.pem4f
.libraries,dns.pem4f: package/cfg/dns_pem4f.xdl
	$(MAKE) -f C:\Users\a0321879\DOCUME~1\MYCCSE~2\enet_dns_tirtos/src/makefile.libs

clean::
	$(MAKE) -f C:\Users\a0321879\DOCUME~1\MYCCSE~2\enet_dns_tirtos/src/makefile.libs clean

