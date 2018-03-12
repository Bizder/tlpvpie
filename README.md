Build modules:

./waf configure --with-nsc=nsc-0.5.3/ --enable-examples --enable-tests
./waf build
./test.py


TODO:

[x] Marker -> send data, add (constant) PV to header field
	comment: Marker sets a bytetag for every packet (ns3 only, not included in real IP header)
[ ] Rework MyTag class to have meaningful names
[x] PVPIE -> Check for header field (tag)
[ ] Marker tocken bucket
[ ] PIE modification to PVPIE
