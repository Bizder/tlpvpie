/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <iostream>
#include "ns3/log.h"
#include "aqm-topology-helper.h"
#include "IApplicationHelperFactory.h"


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("AQMTopologyHelper");

/* TODO: encapsulate to string method in the DelayClass enum */
const std::string DelayClassToString(DelayClass e)
{
  const std::map<DelayClass,std::string> DelayEnumStrings {
      { DelayClass::Gold, "DelayClass::Gold" },
      { DelayClass::Silver, "DelayClass::Silver" },
      { DelayClass::Background, "DelayClass::Background" }
  };

  auto   it  = DelayEnumStrings.find(e);

  if ( it == DelayEnumStrings.end() )
  {
    std::runtime_error("DelayclassToString: Invalid DelayClass");
  }

  return it->second;
}

AQMTopologyHelper::AQMTopologyHelper(std::string bottleneckBandwidth,
                                   std::string bottleneckDelay,
                                   std::string accessBandwidth,
                                   std::string accessDelay)
{
  NS_LOG_FUNCTION (this);

  PointToPointHelper bottleneckLink;
  bottleneckLink.SetDeviceAttribute("DataRate", StringValue(bottleneckBandwidth));
  bottleneckLink.SetChannelAttribute("Delay", StringValue(bottleneckDelay));
  bottleneckLink.SetQueue ("ns3::DropTailQueue", "MaxSize", QueueSizeValue (QueueSize ("1p")));

  PointToPointHelper leafLink;
  leafLink.SetDeviceAttribute("DataRate", StringValue(accessBandwidth));
  leafLink.SetChannelAttribute("Delay", StringValue(accessDelay));

  m_bottleneckHelper = bottleneckLink;
  m_leftHelper = leafLink;
}

AQMTopologyHelper::AQMTopologyHelper(PointToPointHelper bottleneckHelper,
                                   PointToPointHelper leftHelper)
{
  NS_LOG_FUNCTION (this);

  m_bottleneckHelper = bottleneckHelper;
  m_leftHelper = leftHelper;
}

AQMTopologyHelper::~AQMTopologyHelper()
{
  NS_LOG_FUNCTION (this);
}

Ptr<Node> AQMTopologyHelper::GetLeft () const
{
  NS_LOG_FUNCTION (this);

  return m_routers.Get (0);
}

Ptr<Node> AQMTopologyHelper::GetLeft (uint32_t index) const
{
  NS_LOG_FUNCTION (this << index);

  return m_leftLeaf.Get (index);
}

Ptr<Node> AQMTopologyHelper::GetRight () const
{
  NS_LOG_FUNCTION (this);

  return m_routers.Get (1);
}

uint32_t  AQMTopologyHelper::LeftCount () const
{
  NS_LOG_FUNCTION (this);

  return m_leafConfigurations.size();
}

void AQMTopologyHelper::Initialize()
{
  NS_LOG_FUNCTION (this);

  m_initialized = true;

  m_routers.Create(2);
  m_leftLeaf.Create (LeftCount());

  m_routerDevices = m_bottleneckHelper.Install (m_routers);
  for (uint32_t i = 0; i < LeftCount(); ++i)
    {
      NetDeviceContainer c = m_leftHelper.Install (m_routers.Get (0),
                                                 m_leftLeaf.Get (i));
      m_leftRouterDevices.Add (c.Get (0));
      m_leftLeafDevices.Add (c.Get (1));
    }
}

void AQMTopologyHelper::InstallStack()
{
  NS_LOG_FUNCTION (this);

	InternetStackHelper stack;
  InstallStack(stack);
}

void AQMTopologyHelper::InstallStack(InternetStackHelper stack)
{
  NS_LOG_FUNCTION (this);

  stack.Install (m_routers);
  stack.Install (m_leftLeaf);
}

void AQMTopologyHelper::AssignIpv4Addresses (Ipv4AddressHelper leftIp,
                                            Ipv4AddressHelper routerIp)
{
  NS_LOG_FUNCTION (this);

  m_routerInterfaces = routerIp.Assign (m_routerDevices);

  for (uint32_t i = 0; i < LeftCount (); ++i)
    {
      NetDeviceContainer ndc;
      ndc.Add (m_leftLeafDevices.Get (i));
      ndc.Add (m_leftRouterDevices.Get (i));
      Ipv4InterfaceContainer ifc = leftIp.Assign (ndc);
      m_leftLeafInterfaces.Add (ifc.Get (0));
      m_leftRouterInterfaces.Add (ifc.Get (1));
      leftIp.NewNetwork();
    }
}

void AQMTopologyHelper::InstallTrafficControl(TrafficControlHelper trafficControlHelper)
{
  NS_LOG_FUNCTION (this);

  m_bottleneckQueueDisc = trafficControlHelper.Install(m_routerDevices.Get(0));
}

void AQMTopologyHelper::InstallPvPieTrafficControl()
{
  NS_LOG_FUNCTION (this);

  TrafficControlHelper trafficControlHelper;
  trafficControlHelper.SetRootQueueDisc("ns3::PvPieQueueDisc");
  InstallTrafficControl(trafficControlHelper);
}

void AQMTopologyHelper::InstallTlPvPieTrafficControl()
{
  NS_LOG_FUNCTION (this);

  TrafficControlHelper trafficControlHelper;
  trafficControlHelper.SetRootQueueDisc("ns3::TlPvPieQueueDisc");
  InstallTrafficControl(trafficControlHelper);
}

void AQMTopologyHelper::InstallPacketMarker(uint32_t index, DelayClass delayClass)
{
  NS_LOG_FUNCTION (this << index);

  std::string qdClass;
  switch(delayClass)
  {
    case DelayClass::Gold: qdClass = "Gold"; break;
    case DelayClass::Silver: qdClass = "Silver"; break;
    case DelayClass::Background: qdClass = "Background"; break;
  }

  TrafficControlHelper trafficControlHelper;
  trafficControlHelper.SetRootQueueDisc("ns3::" + qdClass + "PacketMarkerQueueDisc");
  QueueDiscContainer qd = trafficControlHelper.Install(m_leftLeafDevices.Get(index));
  m_leftLeafQueueDiscs.Add(qd);

  switch(delayClass)
  {
    case DelayClass::Gold: m_goldQueueDiscs.Add(qd); break;
    case DelayClass::Silver: m_silverQueueDiscs.Add(qd); break;
    case DelayClass::Background: m_backgroundQueueDiscs.Add(qd); break;
  }
}

void AQMTopologyHelper::InstallPacketMarkers()
{
  NS_LOG_FUNCTION (this);

  for ( uint32_t i = 0; i < LeftCount(); ++i )
  {
    InstallPacketMarker(i, m_leafConfigurations[i].GetDelayClass());
  }
}

void AQMTopologyHelper::InstallSinkApplication()
{
  NS_LOG_FUNCTION (this);

  InstallSinkApplication(Seconds(0));
}

void AQMTopologyHelper::InstallSinkApplication(Time startTime)
{
  NS_LOG_FUNCTION (this);

  Address sinkLocalAddress(InetSocketAddress(Ipv4Address::GetAny(), m_port));
  PacketSinkHelper TcpPacketSinkHelper("ns3::TcpSocketFactory", sinkLocalAddress);
  m_sinkApp = TcpPacketSinkHelper.Install(GetRight());
  m_sinkApp.Start(startTime);
}

void AQMTopologyHelper::InstallSourceApplication(uint32_t index,
                                                 std::string transferProtocolClass,
                                                 IApplicationHelperFactory::APPLICATION_HELPERS applicationHelper,
                                                 Time startTime,
                                                 Time stopTime)
{
  NS_LOG_FUNCTION (this);

  if (( transferProtocolClass != "ns3::UdpSocketFactory" ) && ( transferProtocolClass != "ns3::TcpSocketFactory" ))
  {
    throw std::runtime_error("InstallSourceApplicaiton: Transfer protocol class has to be either ns3::UdpSocketFactory or ns3::TcpSocketFactory!");
  }

  Address remoteAddress(InetSocketAddress(m_routerInterfaces.GetAddress(1), m_port));

  IApplicationHelperFactory *factory = IApplicationHelperFactory::CreateFactory(applicationHelper);
  ApplicationContainer sourceApp = factory->GetApplicationHelper()->Install(m_leftLeaf.Get(index), transferProtocolClass, remoteAddress);
  sourceApp.Start(startTime);
  if ( stopTime > 0 )
  {
    sourceApp.Stop(stopTime);
  }
  m_leftLeafApplications.Add(sourceApp);
}

void AQMTopologyHelper::InstallSourceApplications()
{
  NS_LOG_FUNCTION (this);

  for ( uint32_t i = 0; i < LeftCount(); ++i )
  {
    InstallSourceApplication(i, m_leafConfigurations[i].GetTransferProtocolClass(), IApplicationHelperFactory::APPLICATION_HELPERS::ONOFF, m_leafConfigurations[i].GetStartTime(), m_leafConfigurations[i].GetStopTime());
  }
}

void AQMTopologyHelper::ConfigureLeaf(DelayClass delayClass,
                                      Time startTime,
                                      Time stopTime)
{



  NS_LOG_FUNCTION (this << DelayClassToString(delayClass) << startTime.GetSeconds() << stopTime.GetSeconds());

  if ( m_initialized )
  {
    throw std::runtime_error("Cannot add new Leaf after IP addressess are configured!");
  }

  m_leafConfigurations.push_back(LeafConfigurationHelper(delayClass, startTime, stopTime));
}

}