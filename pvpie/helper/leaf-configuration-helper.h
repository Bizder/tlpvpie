/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef LEAF_CONFIGURATION_HELPER_H
#define LEAF_CONFIGURATION_HELPER_H

namespace ns3 {

enum class DelayClass { Gold, Silver, Background};

class LeafConfigurationHelper {

public:
    LeafConfigurationHelper(DelayClass delayClass,
               Time startTime,
               Time stopTime);

    DelayClass GetDelayClass();
    Time GetStartTime();
    Time GetStopTime();

private:
    DelayClass m_delayClass;
    Time m_startTime;
    Time m_stopTime;
};

}

#endif /* LEAF_CONFIGURATION_HELPER_H */