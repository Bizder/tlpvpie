Build modules:

./waf configure --with-nsc=nsc-0.5.3/ --enable-examples --enable-tests
./waf Build
./test.py


TODO:

[x] Marker -> send data, add (constant) PV to header field
	comment: Marker sets a bytetag for every packet (ns3 only, not included in real IP header)
[x] Rework MyTag class to have meaningful names
[x] PVPIE -> Check for header field (tag)
[x] Marker rate calculation (EWMA)
[x] Add delay class to tags for easier statistics
[x] PVPIE - eCDF function
[x] PIE to PVPIE modification
[ ] python script to run from config files
[ ] trace different delay class latencies (delay class + current delay)
[ ] add inner PIE controller to replace eCDF
[ ]
[ ]
[ ]

