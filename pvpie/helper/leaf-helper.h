/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef LEAFHELPER_H
#define LEAFHELPER_H

namespace ns3 {

class Leafhelper {

public:
    LeafHelper(AQMTopologyHelper::DelayClass delayClass,
                       Time startTime,
                       Time stopTime);

    AQMTopologyHelper::DelayClass GetDelayClass();
    Time GetStartTime();
    Time GetStopTime();

private:
    AQMTopologyHelper::DelayClass m_delayClass;
    Time m_startTime;
    Time m_stopTime;
};


}

#endif /* LEAFHELPER_H */