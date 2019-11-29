SET NS3_PATH=c:/Users/tokodi/OTHER/pvpie-project/ns-allinone-3.30.1
SET PVPIE_REPO_PATH=c:/Users/tokodi/OTHER/pvpie-project/pvpie

docker run -d -it ^
	--mount type=bind,source=%NS3_PATH%/,target=/ns3/ ^
	--mount type=bind,source=%PVPIE_REPO_PATH%/scripts,target=/ns3/ns-3.30.1/scratch/ ^
	--mount type=bind,source=%PVPIE_REPO_PATH%/pvpie,target=/ns3/ns-3.30.1/src/pvpie/ ^
	--mount type=bind,source=%PVPIE_REPO_PATH%/output,target=/ns3/ns-3.30.1/output/ ^
	ns3.30.1 /bin/bash
