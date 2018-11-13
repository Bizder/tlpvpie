SET PVPIE_REPO_PATH=c:/Users/tokodi/OTHER/pvpie

docker run -d -it ^
	--mount type=bind,source=c:/Users/tokodi/OTHER/ns3,target=/ns3/ ^
	--mount type=bind,source=%PVPIE_REPO_PATH%/scripts,target=/ns3/ns-3.29/scratch/ ^
	--mount type=bind,source=%PVPIE_REPO_PATH%/pvpie,target=/ns3/ns-3.29/src/pvpie/ ^
	--mount type=bind,source=%PVPIE_REPO_PATH%/output,target=/ns3/ns-3.29/output/ ^
	bizder/ns3 /bin/bash
