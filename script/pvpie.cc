#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-control-module.h"
// Flow monitor helper
#include "ns3/valuedapp.h"

NS_LOG_COMPONENT_DEFINE ("PVPIE");

void TcPacketsInQueueTrace (ns3::Ptr<ns3::OutputStreamWrapper> stream, uint32_t oldValue, uint32_t newValue)
{
	std::cout << "TcPacketsInQueue " << oldValue << " to " << newValue << std::endl;
	*stream->GetStream() << "TcPacketsInQueue " << oldValue << " to " << newValue << std::endl;
}

void DevicePacketsInQueueTrace (ns3::Ptr<ns3::OutputStreamWrapper> stream, uint32_t oldValue, uint32_t newValue)
{
	std::cout << "DevicePacketsInQueue " << oldValue << " to " << newValue << std::endl;
	*stream->GetStream() << "DevicePacketsInQueue " << oldValue << " to " << newValue << std::endl;
}

void SojournTimeTrace (ns3::Time oldValue, ns3::Time newValue)
{
	std::cout << "Sojourn time " << newValue.ToDouble (ns3::Time::MS) << "ms" << std::endl;
}

int main (int argc, char *argv[])
{
	ns3::CommandLine cmd;
	cmd.Parse (argc, argv);

	int port = 777;
	float startTime = 0.0;
	float stopTime = 5.0;

	int nClients = 30;
	unsigned int packetSize = 64;

	std::string accessBandwidth = "10Mbps";
	std::string accessDelay = "20ms";

	std::string bottleneckBandwidth ="1Mbps";
	std::string bottleneckDelay = "20ms";
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
	ns3::Ptr<ns3::Node> leftrouter = ns3::CreateObject<ns3::Node>();
	routers.Add(leftrouter);
	ns3::Ptr<ns3::Node> rightrouter = ns3::CreateObject<ns3::Node>();
	routers.Add(rightrouter);

	ns3::NodeContainer leftleaves;
	ns3::NodeContainer rightleaves;
	leftleaves.Create(nClients);
	rightleaves.Create(nClients);

	// Add links to connect Nodes
	ns3::NetDeviceContainer routerdevices = bottleNeckLink.Install(routers);

	ns3::NetDeviceContainer leftrouterdevices;
	ns3::NetDeviceContainer leftleafdevices;
	ns3::NetDeviceContainer rightrouterdevices;
	ns3::NetDeviceContainer rightleafdevices;

	for (int i = 0; i < nClients ; ++i)
	{
		ns3::NetDeviceContainer cleft = leafLink.Install(routers.Get(0), leftleaves.Get(i));
		leftrouterdevices.Add(cleft.Get(0));
		leftleafdevices.Add(cleft.Get(1));

		ns3::NetDeviceContainer cright = leafLink.Install(routers.Get(1), rightleaves.Get(i));
		rightrouterdevices.Add(cright.Get(0));
		rightleafdevices.Add(cright.Get(1));
	}

	// Assign IPv4 addresses
	ns3::InternetStackHelper stack;
	stack.Install(routers);
	stack.Install(leftleaves);
	stack.Install(rightleaves);

	// Install Traffic control helper
	ns3::TrafficControlHelper tch;
	tch.SetRootQueueDisc ("ns3::RedQueueDisc"); // set PVPIE QueueDisc
	ns3::QueueDiscContainer qdiscs = tch.Install (routerdevices.Get(0));

	// Connect tracesinks!
	ns3::AsciiTraceHelper asciiTraceHelper;
	ns3::Ptr<ns3::OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream ("output/pcap/qlen.bn");

	ns3::Ptr<ns3::QueueDisc> q = qdiscs.Get (0);
	q->TraceConnectWithoutContext ("PacketsInQueue", ns3::MakeBoundCallback(&TcPacketsInQueueTrace, stream));


	ns3::Config::ConnectWithoutContext ("/NodeList/0/$ns3::TrafficControlLayer/RootQueueDiscList/0/SojournTime", ns3::MakeCallback(&SojournTimeTrace));


	ns3::Ptr<ns3::OutputStreamWrapper> stream2 = asciiTraceHelper.CreateFileStream ("output/pcap/qlen2.bn");
	ns3::Ptr<ns3::NetDevice> nd = routerdevices.Get (0);

	ns3::Ptr<ns3::PointToPointNetDevice> ptpnd = ns3::DynamicCast<ns3::PointToPointNetDevice> (nd);
	ns3::Ptr<ns3::Queue<ns3::Packet> > queue = ptpnd->GetQueue ();
	queue->TraceConnectWithoutContext ("PacketsInQueue", ns3::MakeBoundCallback(&DevicePacketsInQueueTrace, stream2));

	// TODO: Redesign address allocating ( same helper )
	ns3::Ipv4AddressHelper routerips = ns3::Ipv4AddressHelper("10.3.1.0", "255.255.255.0");
	ns3::Ipv4AddressHelper leftips   = ns3::Ipv4AddressHelper("10.1.1.0", "255.255.255.0");
	ns3::Ipv4AddressHelper rightips  = ns3::Ipv4AddressHelper("10.2.1.0", "255.255.255.0");

	ns3::Ipv4InterfaceContainer routerifs;
	ns3::Ipv4InterfaceContainer leftleafifs;
	ns3::Ipv4InterfaceContainer leftrouterifs;
	ns3::Ipv4InterfaceContainer rightleafifs;
	ns3::Ipv4InterfaceContainer rightrouterifs;

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

		ns3::NetDeviceContainer ndcright;
		ndcright.Add(rightleafdevices.Get(i));
		ndcright.Add(rightrouterdevices.Get(i));
		ns3::Ipv4InterfaceContainer ifcright = rightips.Assign(ndcright);
		rightleafifs.Add(ifcright.Get(0));
		rightrouterifs.Add(ifcright.Get(1));
		rightips.NewNetwork();
	}

	// App layer
	ns3::ApplicationContainer sinkApps, udpApp;
	ns3::Address sinkLocalAddress(ns3::InetSocketAddress(ns3::Ipv4Address::GetAny(), port));
	ns3::PacketSinkHelper TcpPacketSinkHelper("ns3::TcpSocketFactory", sinkLocalAddress);

	// InetSocketAddress rmt (interfaces.GetAddress (0), port);
	// rmt.SetTos (0xb8);

	for ( int i = 0; i < nClients; ++i)
	{
		ns3::Ptr<ns3::Socket> sockptr;
		sockptr = ns3::Socket::CreateSocket(leftleaves.Get(i), ns3::TcpSocketFactory::GetTypeId());
		ns3::Ptr<ns3::TcpSocket> tcpsockptr = ns3::DynamicCast<ns3::TcpSocket> (sockptr);
		tcpsockptr->SetAttribute("SegmentSize", ns3::UintegerValue(packetSize));

		ns3::Ptr<ns3::ValuedApp> app = ns3::CreateObject<ns3::ValuedApp> ();;
		app->Setup(sockptr, ns3::InetSocketAddress(rightleafifs.GetAddress(i), port), packetSize, 4);
		app->SetStartTime(ns3::Seconds(startTime));
		leftleaves.Get(i)->AddApplication(app);

		sinkApps.Add(TcpPacketSinkHelper.Install(rightleaves.Get(i)));
	}

	sinkApps.Start(ns3::Seconds(0.0));

	ns3::Ipv4GlobalRoutingHelper::PopulateRoutingTables();

	bottleNeckLink.EnablePcap("output/pcap/BN_", routers.Get(0)->GetId(), 0);

	ns3::Simulator::Stop(ns3::Seconds(stopTime));
	ns3::Simulator::Run ();
	ns3::Simulator::Destroy();
	return 0;
}

