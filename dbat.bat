docker run -d -it ^
	--mount type=bind,source=d:/Projects/PVPIE-ns3/pvpie/script,target=/home/ns-allinone-3.27/ns-3.27/scratch/ ^
	--mount type=bind,source=d:/Projects/PVPIE-ns3/pvpie/pvpie,target=/home/ns-allinone-3.27/ns-3.27/src/pvpie ^
	--mount type=bind,source=d:/Projects/PVPIE-ns3/output/,target=/home/ns-allinone-3.27/ns-3.27/output/ ^
	23571113/nsc /bin/bash
