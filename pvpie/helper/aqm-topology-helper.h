/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef AQMTOPOLOGYHELPER_H
#define AQMTOPOLOGYHELPER_H

#include <vector>

#include "ns3/point-to-point-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-interface-container.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-control-module.h"

#include "leaf-configuration-helper.h"

namespace ns3 {

class AQMTopologyHelper {

  public:
    AQMTopologyHelper(std::string bottleneckBandwidth,
                     std::string bottleneckDelay,
                     std::string accessBandwidth,
                     std::string accessDelay);

    AQMTopologyHelper(PointToPointHelper bottleneckHelper,
                     PointToPointHelper leftHelper);

    ~AQMTopologyHelper();

    uint32_t GetClientCount();

    Ptr<Node> GetLeft () const;
    Ptr<Node> GetLeft (uint32_t i) const;

    Ptr<Node> GetRight () const;

    uint32_t  LeftCount () const;

    void Initialize();
    void InstallStack();
    void InstallStack(InternetStackHelper stack);
    void AssignIpv4Addresses(Ipv4AddressHelper leftIp,
                             Ipv4AddressHelper routerIp);

    void InstallTrafficControl(TrafficControlHelper trafficControlHelper);
    void InstallPvPieTrafficControl();

    void InstallPacketMarker(uint32_t i, DelayClass delayClass);
    void InstallPacketMarkers();

    void InstallSinkApplication();
    void InstallSinkApplication(Time startTime);

    void InstallSourceApplication(uint32_t i, Time startTime, Time stopTime);
    void InstallSourceApplications();

    void ConfigureLeaf(uint32_t i, DelayClass delayClass, Time startTime, Time stopTime);

  // private:
    const int m_port = 777;

    bool m_initialized = false;

    PointToPointHelper m_bottleneckHelper;
    PointToPointHelper m_leftHelper;

    std::vector<LeafConfigurationHelper> m_leafConfigurations;

    NodeContainer m_leftLeaf;
    NetDeviceContainer m_leftLeafDevices;

    NodeContainer m_routers;
    NetDeviceContainer m_routerDevices;
    NetDeviceContainer m_leftRouterDevices;

    Ipv4InterfaceContainer m_leftLeafInterfaces;
    Ipv4InterfaceContainer m_leftRouterInterfaces;
    Ipv4InterfaceContainer m_routerInterfaces;

    QueueDiscContainer m_bottleneckQueueDisc;
    QueueDiscContainer m_leftLeafQueueDiscs;
    QueueDiscContainer m_goldQueueDiscs;
    QueueDiscContainer m_silverQueueDiscs;
    QueueDiscContainer m_backgroundQueueDiscs;

    ApplicationContainer m_sinkApp;
    ApplicationContainer m_leftLeafApplications;
};

}

#endif /* AQMTOPOLOGYHELPER_H */