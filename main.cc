#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

NS_LOG_COMPONENT_DEFINE ("PIE");

class UnlimitedRateApp : public ns3::Application
{
	public:
		UnlimitedRateApp();
		virtual ~ UnlimitedRateApp();

		void Setup(ns3::Ptr<ns3::Socket> socket, ns3::Address address, uint32_t packetSize);

	private:
		// inherited from Application base class.
		virtual void StartApplication(void); // Called at time specified by Start, inherited
		virtual void StopApplication(void); // Called at time specified by Stop, inherited
	
		void SendData();

		ns3::Ptr<ns3::Socket> m_socket;  // Associated socket
		ns3::Address m_peer;        // Peer address
		bool m_connected;           // True if connected
		uint32_t m_packetSize;      // Size of data to send each time

		void ConnectionSucceeded(ns3::Ptr<ns3::Socket> socket);
		void ConnectionFailed(ns3::Ptr<ns3::Socket> socket);
		void DataSend(ns3::Ptr<ns3::Socket>, uint32_t);  // for socket's SetSendCallback
		void Ignore(ns3::Ptr<ns3::Socket> socket);
};

UnlimitedRateApp::UnlimitedRateApp(): m_socket(0), m_connected(false), m_packetSize(512)
{
}

UnlimitedRateApp::~UnlimitedRateApp()
{
	m_socket = 0;
}

void UnlimitedRateApp::Setup(ns3::Ptr<ns3::Socket> socket, ns3::Address address, uint32_t packetSize)
{
	m_socket = socket;
	m_peer = address;
	m_packetSize = packetSize;

	// Fatal error if socket type is not NS3_SOCK_STREAM or NS3_SOCK_SEQPACKET
	if (m_socket->GetSocketType() != ns3::Socket::NS3_SOCK_STREAM &&
		m_socket->GetSocketType() != ns3::Socket::NS3_SOCK_SEQPACKET)
	{
		
		NS_FATAL_ERROR
			("Using UnlimitedRateApp with an incompatible socket type. "
			 "UnlimitedRateApp requires SOCK_STREAM or SOCK_SEQPACKET. "
			 "In other words, use TCP instead of UDP.");
	}
}

void UnlimitedRateApp::StartApplication()
{
	m_socket->Bind();
	m_socket->Connect(m_peer);
	m_socket->ShutdownRecv();
	m_socket->SetConnectCallback(ns3::MakeCallback(&UnlimitedRateApp::ConnectionSucceeded, this), ns3::MakeCallback(&UnlimitedRateApp::ConnectionFailed, this));
	m_socket->SetSendCallback(ns3::MakeCallback(&UnlimitedRateApp::DataSend, this));
	if (m_connected)
	{
		SendData();
	}
}

void UnlimitedRateApp::StopApplication()
{
	if (m_socket != 0) {
		m_socket->Close();
		m_connected = false;
	}
}

void UnlimitedRateApp::SendData(void)
{
	// We exit this loop when actual<toSend as the send side
	// buffer is full. The "DataSend" callback will pop when
	// some buffer space has freed ip.
	for (;;) {
		ns3::Ptr<ns3::Packet> packet = ns3::Create<ns3::Packet>(m_packetSize);
		// set header
		// TODO VP-1: set packet
		int actual = m_socket->Send(packet);
		if ((unsigned) actual != m_packetSize) 
		{
			break;
		}
	}
}

void UnlimitedRateApp::ConnectionSucceeded(ns3::Ptr<ns3::Socket> socket)
{
	m_connected = true;
	SendData();
}

void UnlimitedRateApp::ConnectionFailed(ns3::Ptr<ns3::Socket> socket)
{
}

void UnlimitedRateApp::DataSend(ns3::Ptr<ns3::Socket>, uint32_t)
{
	if (m_connected) {
		ns3::Simulator::ScheduleNow(&UnlimitedRateApp::SendData, this);
	}
}

int main (int argc, char *argv[])
{
	ns3::CommandLine cmd;
	cmd.Parse (argc, argv);

	int number_of_clients = 1;
	int port = 9000;

	std::string access_bandwidth = "100Mbps";
	std::string access_delay = "20ms";

	std::string bottleneck_bandwidth = "10kbps";
	std::string bottleneck_delay = "20ms";

	ns3::Time::SetResolution (ns3::Time::NS);
	// ns3::LogComponentEnable ("UdpEchoClientApplication", ns3::LOG_LEVEL_INFO);
	// ns3::LogComponentEnable ("UdpEchoServerApplication", ns3::LOG_LEVEL_INFO);
	// ns3::LogComponentEnable ("UnlimitedRateApp", ns3::LOG_LEVEL_INFO);

	// // Create link helpers
	ns3::PointToPointHelper bottleNeckLink;
	bottleNeckLink.SetDeviceAttribute("DataRate", ns3::StringValue(bottleneck_bandwidth));
	bottleNeckLink.SetChannelAttribute("Delay", ns3::StringValue(bottleneck_delay));

	ns3::PointToPointHelper leafLink;
	leafLink.SetDeviceAttribute("DataRate", ns3::StringValue(access_bandwidth));
	leafLink.SetChannelAttribute("Delay", ns3::StringValue(access_delay));

	// // Creating Nodes
	ns3::NodeContainer routers;
	// TODO VP-3: add an APP
	ns3::Ptr<ns3::Node> leftrouter = ns3::CreateObject<ns3::Node>();
	routers.Add(leftrouter);
	ns3::Ptr<ns3::Node> rightrouter = ns3::CreateObject<ns3::Node>();
	routers.Add(rightrouter);

	ns3::NodeContainer leftleaves;
	ns3::NodeContainer rightleaves;
	leftleaves.Create(number_of_clients);
	rightleaves.Create(number_of_clients);

	// // Add links to connect Nodes
	ns3::NetDeviceContainer routerdevices = bottleNeckLink.Install(routers);

	ns3::NetDeviceContainer leftrouterdevices;
	ns3::NetDeviceContainer leftleafdevices;
	ns3::NetDeviceContainer rightrouterdevices;
	ns3::NetDeviceContainer rightleafdevices;

	for (int i = 0; i < number_of_clients ; ++i)
	{
		ns3::NetDeviceContainer cleft = leafLink.Install(routers.Get(0), leftleaves.Get(i));
		leftrouterdevices.Add(cleft.Get(0));
		leftleafdevices.Add(cleft.Get(1));

		ns3::NetDeviceContainer cright = leafLink.Install(routers.Get(1), rightleaves.Get(i));
		rightrouterdevices.Add(cright.Get(0));
		rightleafdevices.Add(cright.Get(1));
	}

	// // Assign IPv4 addresses
	ns3::InternetStackHelper stack;
	stack.Install(routers);
	stack.Install(leftleaves);
	stack.Install(rightleaves);

	ns3::Ipv4AddressHelper routerips = ns3::Ipv4AddressHelper("10.1.3.0", "255.255.255.0");
	ns3::Ipv4AddressHelper leftips   = ns3::Ipv4AddressHelper("10.1.1.0", "255.255.255.0");
	ns3::Ipv4AddressHelper rightips  = ns3::Ipv4AddressHelper("10.1.2.0", "255.255.255.0");

	ns3::Ipv4InterfaceContainer routerifs;
	ns3::Ipv4InterfaceContainer leftleafifs;
	ns3::Ipv4InterfaceContainer leftrouterifs;
	ns3::Ipv4InterfaceContainer rightleafifs;
	ns3::Ipv4InterfaceContainer rightrouterifs;

	routerifs = routerips.Assign(routerdevices);

	for (int i = 0; i < number_of_clients ; ++i )
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

	// // App layer
	ns3::ApplicationContainer sinkApps, udpApp;
	ns3::Address sinkLocalAddress(ns3::InetSocketAddress(ns3::Ipv4Address::GetAny(), port));
	ns3::PacketSinkHelper TcpPacketSinkHelper("ns3::TcpSocketFactory", sinkLocalAddress);

	for ( int i = 0; i < number_of_clients; ++i)
	{
		ns3::Ptr<ns3::Socket> sockptr;
		unsigned int pkgsize = 512;
		float start = 2.0;
		// APP = UR, TCP, 1024, 2.0
		// TCP flow
		sockptr = ns3::Socket::CreateSocket(leftleaves.Get(i), ns3::TcpSocketFactory::GetTypeId());
		ns3::Ptr<ns3::TcpSocket> tcpsockptr = ns3::DynamicCast<ns3::TcpSocket> (sockptr);
		tcpsockptr->SetAttribute("SegmentSize", ns3::UintegerValue(pkgsize));

		// std::stringstream nodeidss;
		// nodeidss << leftleaves.Get(i)->GetId();
		// std::string prefix = "/NodeList/" + nodeidss.str();

		sinkApps.Add(TcpPacketSinkHelper.Install(rightleaves.Get(i)));
		float stop = 60.0;

		ns3::Ptr<UnlimitedRateApp> app = ns3::CreateObject<UnlimitedRateApp> ();;
		app->Setup(sockptr, ns3::InetSocketAddress(rightleafifs.GetAddress(i), port), pkgsize);
		leftleaves.Get(i)->AddApplication(app);
		app->SetStartTime(ns3::Seconds(start));
		app->SetStopTime(ns3::Seconds(stop));
	}

	// TODO VP-1: check header / header
	sinkApps.Start(ns3::Seconds(0.0));

	// TODO VP-2: populate manually
	ns3::Ipv4GlobalRoutingHelper::PopulateRoutingTables();

	leafLink.EnablePcapAll("VP_", true);

	float simstop = 60.0;
	if (simstop > 0.0) {
		ns3::Simulator::Stop(ns3::Seconds(simstop));
	}

	ns3::Simulator::Run ();

	std::cerr << "Dumbbell Left Bottleneck NodeID " << routers.Get(0)->GetId() << std::endl;
	std::cerr << "Dumbbell Right Bottleneck NodeID " << routers.Get(1)->GetId() << std::endl;

	ns3::Simulator::Destroy ();
	return 0;
}

