#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

NS_LOG_COMPONENT_DEFINE ("PVPIE");

class ValuedApp : public ns3::Application
{
	public:
		ValuedApp();
		virtual ~ ValuedApp();

		void Setup(ns3::Ptr<ns3::Socket> socket, ns3::Address address, uint32_t packetSize, uint8_t packetValue);

	private:
		virtual void StartApplication(void);
		virtual void StopApplication(void);

		void SendData();

		ns3::Ptr<ns3::Socket> m_socket;
		ns3::Address m_peer;
		bool m_connected;
		uint32_t m_packetSize;
		uint8_t m_packetValue;

		void ConnectionSucceeded(ns3::Ptr<ns3::Socket> socket);
		void ConnectionFailed(ns3::Ptr<ns3::Socket> socket);
		void DataSend(ns3::Ptr<ns3::Socket>, uint32_t);
		void Ignore(ns3::Ptr<ns3::Socket> socket);
};

ValuedApp::ValuedApp(): m_socket(0), m_connected(false), m_packetSize(512), m_packetValue(0)
{
}

ValuedApp::~ValuedApp()
{
	m_socket = 0;
}

void ValuedApp::Setup(ns3::Ptr<ns3::Socket> socket, ns3::Address address, uint32_t packetSize, uint8_t packetValue)
{
	m_socket = socket;
	m_peer = address;
	m_packetSize = packetSize;
	m_packetValue = packetValue;

	// Fatal error if socket type is not NS3_SOCK_STREAM or NS3_SOCK_SEQPACKET
	if (m_socket->GetSocketType() != ns3::Socket::NS3_SOCK_STREAM &&
		m_socket->GetSocketType() != ns3::Socket::NS3_SOCK_SEQPACKET)
	{
		NS_FATAL_ERROR
			("Using ValuedApp with an incompatible socket type. "
			 "ValuedApp requires SOCK_STREAM or SOCK_SEQPACKET. "
			 "In other words, use TCP instead of UDP.");
	}
}

void ValuedApp::StartApplication()
{
	m_socket->Bind();
	m_socket->Connect(m_peer);
	m_socket->SetIpTos(m_packetValue);
	m_socket->ShutdownRecv();
	m_socket->SetConnectCallback(ns3::MakeCallback(&ValuedApp::ConnectionSucceeded, this), ns3::MakeCallback(&ValuedApp::ConnectionFailed, this));
	m_socket->SetSendCallback(ns3::MakeCallback(&ValuedApp::DataSend, this));
	if (m_connected)
	{
		SendData();
	}
}

void ValuedApp::StopApplication()
{
	if (m_socket != 0) {
		m_socket->Close();
		m_connected = false;
	}
}

void ValuedApp::SendData(void)
{
	// We exit this loop when actual<toSend as the send side
	// buffer is full. The "DataSend" callback will pop when
	// some buffer space has freed ip.
	for (;;) {
		ns3::Ptr<ns3::Packet> packet = ns3::Create<ns3::Packet>(m_packetSize);
		int actual = m_socket->Send(packet);
		if ((unsigned) actual != m_packetSize)
		{
			break;
		}
	}
}

void ValuedApp::ConnectionSucceeded(ns3::Ptr<ns3::Socket> socket)
{
	m_connected = true;
	SendData();
}

void ValuedApp::ConnectionFailed(ns3::Ptr<ns3::Socket> socket)
{
}

void ValuedApp::DataSend(ns3::Ptr<ns3::Socket>, uint32_t)
{
	if (m_connected) {
		ns3::Simulator::ScheduleNow(&ValuedApp::SendData, this);
	}
}

int main (int argc, char *argv[])
{
	ns3::CommandLine cmd;
	cmd.Parse (argc, argv);

	int port = 9000;
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

	for ( int i = 0; i < nClients; ++i)
	{
		ns3::Ptr<ns3::Socket> sockptr;
		sockptr = ns3::Socket::CreateSocket(leftleaves.Get(i), ns3::TcpSocketFactory::GetTypeId());
		ns3::Ptr<ns3::TcpSocket> tcpsockptr = ns3::DynamicCast<ns3::TcpSocket> (sockptr);
		tcpsockptr->SetAttribute("SegmentSize", ns3::UintegerValue(packetSize));

		ns3::Ptr<ValuedApp> app = ns3::CreateObject<ValuedApp> ();;
		app->Setup(sockptr, ns3::InetSocketAddress(rightleafifs.GetAddress(i), port), packetSize, 4);
		app->SetStartTime(ns3::Seconds(startTime));
		leftleaves.Get(i)->AddApplication(app);

		sinkApps.Add(TcpPacketSinkHelper.Install(rightleaves.Get(i)));
	}

	sinkApps.Start(ns3::Seconds(0.0));

	// TODO VP-2: populate manually
	ns3::Ipv4GlobalRoutingHelper::PopulateRoutingTables();

	bottleNeckLink.EnablePcap("output/pcap/BN_", routers.Get(0)->GetId(), 0);

	ns3::Simulator::Stop(ns3::Seconds(stopTime));
	ns3::Simulator::Run ();
	ns3::Simulator::Destroy();
	return 0;
}

