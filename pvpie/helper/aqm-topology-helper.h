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

    // unsigned int packetSize = 1000;

    AQMTopologyHelper(std::string bottleneckBandwidth,
                     std::string bottleneckDelay,
                     std::string accessBandwidth,
                     std::string accessDelay,
                     uint32_t nLeftLeaf);
    AQMTopologyHelper(PointToPointHelper bottleneckHelper,
                     uint32_t nLeftLeaf,
                     PointToPointHelper leftHelper);

    void Initialize(PointToPointHelper bottleneckHelper,
                    uint32_t nLeftLeaf,
                    PointToPointHelper leftHelper);

    ~AQMTopologyHelper();

    Ptr<Node> GetLeft () const;
    Ptr<Node> GetLeft (uint32_t i) const;

    Ptr<Node> GetRight () const;

    uint32_t  LeftCount () const;

    void InstallStack();
    void InstallStack(InternetStackHelper stack);
    void AssignIpv4Addresses(Ipv4AddressHelper leftIp,
                             Ipv4AddressHelper routerIp);

    void InstallPvPieTrafficControl();
    void InstallTrafficControl(TrafficControlHelper trafficControlHelper);
    void InstallTrafficControl(uint32_t i, DelayClass delayClass);

    void InstallSinkApplication();
    void InstallSinkApplication(Time startTime);

    void InstallApplication(uint32_t i);
    void InstallApplication(uint32_t i, Time startTime, Time stopTime);

    void ConfigureLeaf(uint32_t i, DelayClass delayClass, Time startTime, Time stopTime);

  // private:
    const int m_port = 777;

    bool m_assigned = false;

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