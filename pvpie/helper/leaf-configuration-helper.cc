/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "aqm-topology-helper.h"
#include "leaf-configuration-helper.h"


namespace ns3 {

LeafConfigurationHelper::LeafConfigurationHelper(DelayClass delayClass,
                       Time startTime,
                       Time stopTime)
{
    m_delayClass = delayClass;
    m_transferProtocolClass = "ns3::UdpSocketFactory";
    m_startTime = startTime;
    m_stopTime = stopTime;
}

LeafConfigurationHelper::LeafConfigurationHelper(DelayClass delayClass,
                       std::string transferProtocolClass,
                       Time startTime,
                       Time stopTime)
{
    m_delayClass = delayClass;
    m_transferProtocolClass = transferProtocolClass;
    m_startTime = startTime;
    m_stopTime = stopTime;
}

DelayClass LeafConfigurationHelper::GetDelayClass()
{
    return m_delayClass;
}

std::string LeafConfigurationHelper::GetTransferProtocolClass()
{
    return m_transferProtocolClass;
}

Time LeafConfigurationHelper::GetStartTime()
{
    return m_startTime;
}

Time LeafConfigurationHelper::GetStopTime()
{
    return m_stopTime;
}

}