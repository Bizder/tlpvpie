/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "aqm-topology-helper.h"
#include "leaf-configuration-helper.h"


namespace ns3 {

LeafConfigurationHelper::LeafConfigurationHelper(DelayClass delayClass,
                       Time startTime,
                       Time stopTime)
{
    m_delayClass = delayClass;
    m_startTime = startTime;
    m_stopTime = stopTime;
}

DelayClass LeafConfigurationHelper::GetDelayClass()
{
    return m_delayClass;
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