#ifndef TRACEHELPER_H
#define TRACEHELPER_H

#include "ns3/core-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/packet-value-tag.h"
#include "ns3/packet-marker-queue-disc.h"
#include <vector>


namespace ns3
{

    uint32_t receivedPackets = 0;
    uint32_t goldPackets = 0;
    uint32_t silverPackets = 0;
    uint32_t backgroundPackets = 0;

    std::vector<uint32_t> flowPackets;

    void DropProbabilityTrace(ns3::Ptr<ns3::OutputStreamWrapper> stream, double oldValue, double newValue)
    {
        *stream->GetStream() << ns3::Simulator::Now().GetSeconds() << "\t" << newValue << std::endl;
    }

    void QueueDelayTrace(ns3::Ptr<ns3::OutputStreamWrapper> stream,  ns3::Time oldValue,  ns3::Time newValue)
    {
        *stream->GetStream() << ns3::Simulator::Now().GetSeconds() << "\t" << newValue.GetMilliSeconds() << std::endl;
    }

    void ThresholdValueTrace(ns3::Ptr<ns3::OutputStreamWrapper> stream, uint32_t oldValue,  uint32_t newValue)
    {
        *stream->GetStream() << ns3::Simulator::Now().GetSeconds() << "\t" << newValue << std::endl;
    }

    void PacketValueTrace(ns3::Ptr<ns3::OutputStreamWrapper> stream, uint32_t oldValue, uint32_t newValue)
    {
        *stream->GetStream() << ns3::Simulator::Now().GetSeconds() << "\t" << newValue << std::endl;
    }

    void ecdfTrace(ns3::Ptr<ns3::OutputStreamWrapper> stream, ns3::Ptr<ns3::PvPieQueueDisc> pqd)
    {
        *stream->GetStream() << ns3::Simulator::Now().GetSeconds() << std::endl;

        std::vector<uint32_t> values = pqd->eCDF_GetValues();
        for ( uint32_t i = 0; i < values.size(); ++i )
        {
            *stream->GetStream() << values[i] << std::endl;
        }
        *stream->GetStream() << std::endl;
    }

    void ThroughputTrace(ns3::Ptr<ns3::OutputStreamWrapper> stream, ns3::Ptr<ns3::OutputStreamWrapper> gold, ns3::Ptr<ns3::OutputStreamWrapper> silver, ns3::Ptr<ns3::OutputStreamWrapper> background)
    {
        *stream->GetStream() << ns3::Simulator::Now().GetSeconds() << "\t" << receivedPackets * 8 / 1000.0 / 1000.0 << std::endl;
        *gold->GetStream() << ns3::Simulator::Now().GetSeconds() << "\t" << goldPackets * 8 / 1000.0 / 1000.0 << std::endl;
        *silver->GetStream() << ns3::Simulator::Now().GetSeconds() << "\t" << silverPackets * 8 / 1000.0 / 1000.0 << std::endl;
        *background->GetStream() << ns3::Simulator::Now().GetSeconds() << "\t" << backgroundPackets * 8 / 1000.0 / 1000.0 << std::endl;

        receivedPackets = 0;
        goldPackets = 0;
        silverPackets = 0;
        backgroundPackets = 0;
        ns3::Simulator::Schedule(ns3::Seconds(1.0), &ThroughputTrace, stream, gold, silver, background);
    }

    void TransferRateTrace(ns3::Ptr<ns3::OutputStreamWrapper> stream, uint32_t oldValue, uint32_t newValue)
    {
        *stream->GetStream() << ns3::Simulator::Now().GetSeconds() << "\t" << newValue << std::endl;
    }

    void RxTrace (ns3::Ptr<const ns3::Packet> packet)
    {
        receivedPackets += packet->GetSize();

        ns3::PacketValueTag tag;
        packet->PeekPacketTag(tag);
        uint8_t delayclass = tag.GetDelayClass();

        if ( delayclass == 1 )
        {
            goldPackets += packet->GetSize();
        }
        else if ( delayclass == 2 )
        {
            silverPackets += packet->GetSize();
        }
        else
        {
            backgroundPackets += packet->GetSize();
        }
    }

    void TimeTrace()
    {
        std::cerr << ns3::Simulator::Now().GetSeconds() << std::endl;
        ns3::Simulator::Schedule(ns3::Seconds(1.0), &TimeTrace);
    }

    void ThroughputMonitor (std::vector< ns3::Ptr<ns3::OutputStreamWrapper> > flows, ns3::Ptr<ns3::FlowMonitor> monitor, Ptr<Ipv4FlowClassifier> classifier)
    {
        std::map<ns3::FlowId, ns3::FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
        for (std::map<ns3::FlowId, ns3::FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
        {
        ns3::Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
        if (t.destinationAddress == "99.9.1.2")
        {
            uint32_t num = t.sourceAddress.Get();
            num = ((num - 1 - 167772160 - 65536) / 256 ) -1 ;

            ns3::Ptr<ns3::OutputStreamWrapper> s;
            s = flows[num];

            double data = 0;

            if (int(ns3::Simulator::Now().GetSeconds()) % 30 > 0 )
            {
                data = (i->second.rxBytes - flowPackets[num] ) / double((int(ns3::Simulator::Now().GetSeconds()) % 30)) * 8 / 1000.0 / 1000.0;
            }
            else
            {
                data = (i->second.rxBytes - flowPackets[num] ) / ns3::Simulator::Now().GetSeconds() * 8 / 1000.0 / 1000.0;
            }

            if ( data > 0 && ns3::Simulator::Now().GetSeconds() > 65.0)
            {
                *s->GetStream() << ns3::Simulator::Now().GetSeconds() << "\t" << data << std::endl;
            }

            if ( int(ns3::Simulator::Now().GetSeconds()) % 30 == 0 )
            {
                flowPackets[num] = i->second.rxBytes;
            }
        }
        }
        ns3::Simulator::Schedule(ns3::Seconds(1.0), &ThroughputMonitor, flows, monitor, classifier);
    }

}

#endif /* TRACEHELPER_H */