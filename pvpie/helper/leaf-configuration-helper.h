/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef LEAF_CONFIGURATION_HELPER_H
#define LEAF_CONFIGURATION_HELPER_H


#include "IApplicationHelperFactory.h"

namespace ns3 {

enum class DelayClass { Gold, Silver, Background};

enum class ApplicatonType { Continuous, Bulk };

class LeafConfigurationHelper {

public:
    LeafConfigurationHelper(DelayClass delayClass,
               Time startTime,
               Time stopTime);

    LeafConfigurationHelper(DelayClass delayClass,
            std::string transferProtocolClass,
            Time startTime,
            Time stopTime);

    DelayClass GetDelayClass();
    std::string GetTransferProtocolClass();
    IApplicationHelperFactory::APPLICATION_HELPERS GetApplicationType();
    Time GetStartTime();
    Time GetStopTime();

private:
    DelayClass m_delayClass;
    std::string m_transferProtocolClass;
    Time m_startTime;
    Time m_stopTime;
};

}

#endif /* LEAF_CONFIGURATION_HELPER_H */