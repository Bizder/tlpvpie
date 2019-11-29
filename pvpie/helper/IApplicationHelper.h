/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef IAPPLICATIONHELPER_H
#define IAPPLICATIONHELPER_H


#include "ns3/core-module.h"
#include "ns3/applications-module.h"

namespace ns3
{

class IApplicationHelper {
    public:
        virtual ApplicationContainer Install(Ptr<Node> node, std::string transferProtocolClass, Address remoteAddress) = 0;
};


class OnOffApplicationHelper : public IApplicationHelper
{
public:
    ApplicationContainer Install(Ptr<Node> node, std::string transferProtocolClass, Address remoteAddress)
    {
        OnOffHelper applicationHelper(transferProtocolClass, remoteAddress);
        applicationHelper.SetAttribute ("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
        applicationHelper.SetAttribute ("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
        return applicationHelper.Install(node);
    }

};

class BulkSendApplicationHelper : public IApplicationHelper
{
public:
    ApplicationContainer Install(Ptr<Node> node, std::string transferProtocolClass, Address remoteAddress)
    {
        BulkSendHelper applicationHelper(transferProtocolClass, remoteAddress);
        return applicationHelper.Install(node);
    }

};

}

#endif /* IAPPLICATIONHELPER_H */