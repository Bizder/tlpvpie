#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-control-module.h"
// Flow monitor helper
#include "ns3/pv-pie-queue-disc.h"
#include "ns3/packet-marker-queue-disc.h"
#include "ns3/valuedapp.h"

#define OUTPUT_FOLDER std::string("output/")

NS_LOG_COMPONENT_DEFINE ("PvPieSimulator");

void DropProbabilityTrace(ns3::Ptr<ns3::OutputStreamWrapper> stream, double oldValue, double newValue){
	*stream->GetStream() << ns3::Simulator::Now().GetSeconds() << "\t" << newValue << std::endl;
}

void DelayTrace(ns3::Ptr<ns3::OutputStreamWrapper> stream,  ns3::Time oldValue,  ns3::Time newValue){
	*stream->GetStream() << ns3::Simulator::Now().GetSeconds() << "\t" << newValue.GetMilliSeconds() << std::endl;
}

void TcPacketsInQueueTrace(ns3::Ptr<ns3::OutputStreamWrapper> stream, uint32_t oldValue, uint32_t newValue)
{
	*stream->GetStream() << "TcPacketsInQueue " << oldValue << " to " << newValue << std::endl;
}

void DevicePacketsInQueueTrace(ns3::Ptr<ns3::OutputStreamWrapper> stream, uint32_t oldValue, uint32_t newValue)
{
	*stream->GetStream() << "DevicePacketsInQueue " << oldValue << " to " << newValue << std::endl;
}

int main (int argc, char *argv[])
{
	ns3::CommandLine cmd;
	cmd.Parse (argc, argv);

	int port = 777;
	float startTime = 0.0;
	float stopTime = 120.0;

	int nClients = 10;
	unsigned int packetSize = 1000;

	ns3::Config::SetDefault ("ns3::OnOffApplication::PacketSize", ns3::UintegerValue(packetSize));
	ns3::Config::SetDefault ("ns3::OnOffApplication::DataRate", ns3::StringValue("3Mbps"));

	std::string accessBandwidth = "10Mbps";
	std::string accessDelay = "20ms";

	std::string bottleneckBandwidth ="1Mbps";
	std::string bottleneckDelay = "2ms";
	ns3::Time::SetResolution (ns3::Time::NS);

	// Create link helpers
	ns3::PointToPointHelper bottleNeckLink;
	bottleNeckLink.SetDeviceAttribute("DataRate", ns3::StringValue(bottleneckBandwidth));
	bottleNeckLink.SetChannelAttribute("Delay", ns3::StringValue(bottleneckDelay));
	// Limit the capacity o device queue
	bottleNeckLink.SetQueue ("ns3::DropTailQueue", "Mode", ns3::StringValue ("QUEUE_MODE_PACKETS"), "MaxPackets", ns3::UintegerValue (1));

	ns3::PointToPointHelper leafLink;
	leafLink.SetDeviceAttribute("DataRate", ns3::StringValue(accessBandwidth));
	leafLink.SetChannelAttribute("Delay", ns3::StringValue(accessDelay));

	// Creating Nodes
	ns3::NodeContainer routers;
	routers.Create(2);

	ns3::NodeContainer leftleaves;
	leftleaves.Create(nClients);

	// Add links to connect Nodes
	ns3::NetDeviceContainer routerdevices = bottleNeckLink.Install(routers);

	ns3::NetDeviceContainer leftrouterdevices;
	ns3::NetDeviceContainer leftleafdevices;

	for (int i = 0; i < nClients ; ++i)
	{
		ns3::NetDeviceContainer cleft = leafLink.Install(routers.Get(0), leftleaves.Get(i));
		leftrouterdevices.Add(cleft.Get(0));
		leftleafdevices.Add(cleft.Get(1));
	}

	// Assign IPv4 addresses
	ns3::InternetStackHelper stack;

	stack.Install(routers);
	stack.Install(leftleaves);

	// Install Traffic control helper
	ns3::Config::SetDefault("ns3::PvPieQueueDisc::Mode", ns3::StringValue("QUEUE_DISC_MODE_PACKETS"));
	ns3::Config::SetDefault("ns3::PvPieQueueDisc::QueueDelayReference", ns3::TimeValue(ns3::MilliSeconds (20))); // 40 ms
	ns3::Config::SetDefault("ns3::PvPieQueueDisc::Tupdate", ns3::TimeValue(ns3::MilliSeconds (32))); // 32 ms
	ns3::Config::SetDefault("ns3::PvPieQueueDisc::DequeueThreshold", ns3::UintegerValue(10000)); // 10 Kb for packets between 1Kb and 1,5Kb
	ns3::Config::SetDefault("ns3::PvPieQueueDisc::MeanPktSize", ns3::UintegerValue(packetSize));
	ns3::Config::SetDefault("ns3::PvPieQueueDisc::A", ns3::DoubleValue(0.125));
	ns3::Config::SetDefault("ns3::PvPieQueueDisc::B", ns3::DoubleValue(1.25));

	ns3::TrafficControlHelper tch;
	tch.SetRootQueueDisc("ns3::PvPieQueueDisc");
	ns3::QueueDiscContainer pvpie = tch.Install(routerdevices.Get(0));

	// Install gold markers
	ns3::TrafficControlHelper tch_gold;
	tch_gold.SetRootQueueDisc("ns3::PacketMarkerQueueDisc");
	ns3::QueueDiscContainer goldMarkers;
	for ( int i = 0; i < nClients / 2 ; ++i )
	{
		goldMarkers.Add(tch_gold.Install(leftleafdevices.Get(i)));
	}

	// Install silver markers
	ns3::TrafficControlHelper tch_silver;
	tch_silver.SetRootQueueDisc("ns3::PacketMarkerQueueDisc");
	ns3::QueueDiscContainer silverMarkers;
	for ( int i = nClients / 2; i < nClients; ++i )
	{
		silverMarkers.Add(tch_silver.Install(leftleafdevices.Get(i)));
	}

	// TODO: Redesign address allocating ( same helper )
	ns3::Ipv4AddressHelper routerips = ns3::Ipv4AddressHelper("99.9.1.0", "255.255.255.0");
	ns3::Ipv4AddressHelper leftips   = ns3::Ipv4AddressHelper("10.1.1.0", "255.255.255.0");

	ns3::Ipv4InterfaceContainer routerifs;
	ns3::Ipv4InterfaceContainer leftleafifs;
	ns3::Ipv4InterfaceContainer leftrouterifs;

	routerifs = routerips.Assign(routerdevices);

	for (int i = 0; i < nClients; ++i )
	{
		ns3::NetDeviceContainer ndcleft;
		ndcleft.Add(leftleafdevices.Get(i));
		ndcleft.Add(leftrouterdevices.Get(i));
		ns3::Ipv4InterfaceContainer ifcleft = leftips.Assign(ndcleft);
		leftleafifs.Add(ifcleft.Get(0));
		leftrouterifs.Add(ifcleft.Get(1));
		leftips.NewNetwork();
	}

	// InetSocketAddress rmt (interfaces.GetAddress (0), port);
	// rmt.SetTos (0xb8);

	// App layer
	ns3::Address sinkLocalAddress(ns3::InetSocketAddress(ns3::Ipv4Address::GetAny(), port));
	ns3::PacketSinkHelper TcpPacketSinkHelper("ns3::TcpSocketFactory", sinkLocalAddress);
	ns3::ApplicationContainer sinkApp = TcpPacketSinkHelper.Install(routers.Get(1));
	sinkApp.Start(ns3::Seconds(startTime));
	// sinkApp.Stop(ns3::seconds(stopTime));

	ns3::ApplicationContainer sourceApps;

	for ( int i = 0; i < nClients; ++i)
	{

		ns3::Address remoteAddress(ns3::InetSocketAddress(routerifs.GetAddress(1), port));
		ns3::OnOffHelper clientHelper("ns3::TcpSocketFactory", remoteAddress);
		clientHelper.SetAttribute ("OnTime", ns3::StringValue("ns3::ConstantRandomVariable[Constant=1]"));
		clientHelper.SetAttribute ("OffTime", ns3::StringValue("ns3::ConstantRandomVariable[Constant=0]"));
		// clientHelper.SetAttribute ("OnTime", ns3::StringValue("ns3::UniformRandomVariable[Min=0.|Max=1.]"));
		// clientHelper.SetAttribute ("OffTime", ns3::StringValue("ns3::UniformRandomVariable[Min=0.|Max=1.]"));
		sourceApps = clientHelper.Install (leftleaves.Get (i));
		sourceApps.Start(ns3::Seconds(startTime));
		// sourceApps.Stop(ns3::Seconds(stopTime));
	}

	ns3::Ipv4GlobalRoutingHelper::PopulateRoutingTables();

	ns3::Simulator::Stop(ns3::Seconds(stopTime));

	// Configure logging
	bottleNeckLink.EnablePcap(OUTPUT_FOLDER + "/pcap/BN_", routers.Get(0)->GetId(), 0);

	ns3::AsciiTraceHelper asciiTraceHelper;

	ns3::Ptr<ns3::OutputStreamWrapper> qdiscLengthStream = asciiTraceHelper.CreateFileStream (OUTPUT_FOLDER + "ascii/qdlen.bn");
	ns3::Ptr<ns3::QueueDisc> qd = pvpie.Get(0);
	qd->TraceConnectWithoutContext ("PacketsInQueue", ns3::MakeBoundCallback(&TcPacketsInQueueTrace, qdiscLengthStream));

	ns3::Ptr<ns3::PvPieQueueDisc> pqd = ns3::DynamicCast<ns3::PvPieQueueDisc>(qd);
	ns3::Ptr<ns3::OutputStreamWrapper> dropProbStream = asciiTraceHelper.CreateFileStream (OUTPUT_FOLDER + "ascii/p.bn");
	pqd->TraceConnectWithoutContext ("Probability", ns3::MakeBoundCallback(&DropProbabilityTrace, dropProbStream));
	ns3::Ptr<ns3::OutputStreamWrapper> delayStream = asciiTraceHelper.CreateFileStream (OUTPUT_FOLDER + "ascii/delay.bn");
	pqd->TraceConnectWithoutContext ("QueueingDelay", ns3::MakeBoundCallback(&DelayTrace, delayStream));

	ns3::Ptr<ns3::OutputStreamWrapper> qLengthStream = asciiTraceHelper.CreateFileStream (OUTPUT_FOLDER + "ascii/qlen.bn");
	ns3::Ptr<ns3::PointToPointNetDevice> ptpnd = ns3::DynamicCast<ns3::PointToPointNetDevice>(routerdevices.Get(0));
	ns3::Ptr<ns3::Queue<ns3::Packet> > queue = ptpnd->GetQueue();
	queue->TraceConnectWithoutContext ("PacketsInQueue", ns3::MakeBoundCallback(&DevicePacketsInQueueTrace, qLengthStream));


	ns3::Simulator::Run ();
	ns3::Simulator::Destroy();
	return 0;
}
