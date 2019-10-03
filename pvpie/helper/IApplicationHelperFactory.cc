#include "ns3/applications-module.h"
#include "IApplicationHelperFactory.h"

namespace ns3
{

IApplicationHelperFactory* IApplicationHelperFactory::CreateFactory(APPLICATION_HELPERS factory)
{
    if ( factory == APPLICATION_HELPERS::ONOFF )
    {
        return new OnOffApplicationFactory();
    }
    else if ( factory == APPLICATION_HELPERS::BULKSEND )
    {
        return new BulkSendApplicationFactory();
    }

}

}