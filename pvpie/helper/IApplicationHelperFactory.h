/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef IAPPLICATIONHELPERFACTORY_H
#define IAPPLICATIONHELPERFACTORY_H

#include "ns3/applications-module.h"
#include "IApplicationHelper.h"

namespace ns3
{

class IApplicationHelperFactory {
    public:
        enum APPLICATION_HELPERS
        {
            ONOFF,
            BULKSEND
        };

        virtual IApplicationHelper* GetApplicationHelper() = 0;

        static IApplicationHelperFactory* CreateFactory(APPLICATION_HELPERS factory);
};


class OnOffApplicationFactory : public IApplicationHelperFactory
{
    public:
        IApplicationHelper* GetApplicationHelper()
        {
            return new OnOffApplicationHelper();
        }
};

class BulkSendApplicationFactory : public IApplicationHelperFactory
{
    public:
        IApplicationHelper* GetApplicationHelper()
        {
            return new BulkSendApplicationHelper();
        }
};

}

#endif /* IAPPLICATIONHELPERFACTORY_H */