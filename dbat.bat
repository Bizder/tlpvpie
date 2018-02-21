docker run -d -it ^
	--mount type=bind,source=d:\Projects\PVPIE-ns3\pvpie\script,target=/home/tokodi/workspace/ns-allinone-3.27/ns-3.27/scratch/ ^
	--mount type=bind,source=d:\Projects\PVPIE-ns3\pvpie\pvpie,target=/home/tokodi/workspace/ns-allinone-3.27/ns-3.27/src/pvpie ^
	--mount type=bind,source=d:\Projects\PVPIE-ns3\pvpie\valuedapp,target=/home/tokodi/workspace/ns-allinone-3.27/ns-3.27/src/valuedapp ^
	--mount type=bind,source=d:\Projects\PVPIE-ns3\output/,target=/home/tokodi/workspace/ns-allinone-3.27/ns-3.27/output/ ^
	ns3 /bin/bash
