/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "aqm-topology-helper.h"
#include "leaf-helper.h"


namespace ns3 {

LeafHelper::LeafHelper(AQMTopologyHelper::DelayClass delayClass,
                       Time startTime,
                       Time stopTime)
{
    m_delayClass = delayClass;
    m_startTime = startTime;
    m_stopTime = stopTime;
}

LeafHelper::GetDelayClass()
{
    return m_delayClass;
}

LeafHelper::GetStartTime()
{
    return m_startTime;
}

LeafHelper::GetStopTime()
{
    return m_stopTime;
}

}