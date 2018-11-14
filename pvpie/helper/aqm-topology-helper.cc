/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <iostream>
#include "ns3/log.h"
#include "aqm-topology-helper.h"


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("AQMTopologyHelper");

AQMTopologyHelper::AQMTopologyHelper(std::string bottleneckBandwidth,
                                   std::string bottleneckDelay,
                                   std::string accessBandwidth,
                                   std::string accessDelay)
{
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
  m_bottleneckHelper = bottleneckHelper;
  m_leftHelper = leftHelper;
}

AQMTopologyHelper::~AQMTopologyHelper()
{
}

Ptr<Node> AQMTopologyHelper::GetLeft () const
{
  return m_routers.Get (0);
}

Ptr<Node> AQMTopologyHelper::GetLeft (uint32_t) const
{
  return m_leftLeaf.Get (0);
}

Ptr<Node> AQMTopologyHelper::GetRight () const
{
  return m_routers.Get (1);
}

uint32_t  AQMTopologyHelper::LeftCount () const
{
  return m_leafConfigurations.size();
}

void AQMTopologyHelper::Initialize()
{
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
	InternetStackHelper stack;
  InstallStack(stack);
}

void AQMTopologyHelper::InstallStack(InternetStackHelper stack)
{
  stack.Install (m_routers);
  stack.Install (m_leftLeaf);
}

void AQMTopologyHelper::AssignIpv4Addresses (Ipv4AddressHelper leftIp,
                                            Ipv4AddressHelper routerIp)
{
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
  m_bottleneckQueueDisc = trafficControlHelper.Install(m_routerDevices.Get(0));
}

void AQMTopologyHelper::InstallPvPieTrafficControl()
{
  TrafficControlHelper trafficControlHelper;
  trafficControlHelper.SetRootQueueDisc("ns3::PvPieQueueDisc");
  InstallTrafficControl(trafficControlHelper);
}

void AQMTopologyHelper::InstallPacketMarker(uint32_t i, DelayClass delayClass)
{
    std::string qdClass;
    switch(delayClass)
    {
      case DelayClass::Gold: qdClass = "Gold"; break;
      case DelayClass::Silver: qdClass = "Silver"; break;
      case DelayClass::Background: qdClass = "Background"; break;
    }

    TrafficControlHelper trafficControlHelper;
    trafficControlHelper.SetRootQueueDisc("ns3::" + qdClass + "PacketMarkerQueueDisc");
    QueueDiscContainer qd = trafficControlHelper.Install(m_leftLeafDevices.Get(i));
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
  for ( uint32_t i = 0; i < LeftCount(); ++i )
  {
    InstallPacketMarker(i, m_leafConfigurations[i].GetDelayClass());
  }
}

void AQMTopologyHelper::InstallSinkApplication()
{
  InstallSinkApplication(Seconds(0));
}

void AQMTopologyHelper::InstallSinkApplication(Time startTime)
{
  Address sinkLocalAddress(InetSocketAddress(Ipv4Address::GetAny(), m_port));
  PacketSinkHelper TcpPacketSinkHelper("ns3::TcpSocketFactory", sinkLocalAddress);
  m_sinkApp = TcpPacketSinkHelper.Install(GetRight());
  m_sinkApp.Start(startTime);
}

void AQMTopologyHelper::InstallSourceApplication(uint32_t i, Time startTime, Time stopTime)
{
  Address remoteAddress(InetSocketAddress(m_routerInterfaces.GetAddress(1), m_port));
  OnOffHelper applicationHelper("ns3::TcpSocketFactory", remoteAddress);
  applicationHelper.SetAttribute ("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
  applicationHelper.SetAttribute ("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
  ApplicationContainer sourceApp = applicationHelper.Install(m_leftLeaf.Get(i));
  sourceApp.Start(startTime);
  if ( stopTime > 0 )
  {
    sourceApp.Stop(stopTime);
  }
  m_leftLeafApplications.Add(sourceApp);
}

void AQMTopologyHelper::InstallSourceApplications()
{
  for ( uint32_t i = 0; i < LeftCount(); ++i )
  {
    InstallSourceApplication(i, m_leafConfigurations[i].GetStartTime(), m_leafConfigurations[i].GetStopTime());
  }
}

void AQMTopologyHelper::ConfigureLeaf(uint32_t i, DelayClass delayClass, Time startTime, Time stopTime)
{
  if ( m_initialized )
  {
    throw std::runtime_error("Cannot add new Leaf, IP addressess already configured!");
  }

  m_leafConfigurations.push_back(LeafConfigurationHelper(delayClass, startTime, stopTime));
}

}