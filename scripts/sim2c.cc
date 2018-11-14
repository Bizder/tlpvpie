#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-control-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/ipv4-flow-classifier.h"
#include "ns3/pvpie-queue-disc.h"
#include "ns3/packet-value-tag.h"
#include "ns3/packet-marker-queue-disc.h"
#include "ns3/aqm-topology-helper.h"
#include <vector>

#define OUTPUT_FOLDER std::string("output/")

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("PvPieSimulator");

uint32_t receivedPackets = 0;
uint32_t goldPackets = 0;
uint32_t silverPackets = 0;
uint32_t backgroundPackets = 0;

std::vector<uint32_t> flowPackets;


void DropProbabilityTrace(ns3::Ptr<ns3::OutputStreamWrapper> stream, double oldValue, double newValue)
{
    *stream->GetStream() << ns3::Simulator::Now().GetSeconds() << "\t" << newValue << std::endl;
}

void DelayTrace(ns3::Ptr<ns3::OutputStreamWrapper> stream,  ns3::Time oldValue,  ns3::Time newValue)
{
    *stream->GetStream() << ns3::Simulator::Now().GetSeconds() << "\t" << newValue.GetMilliSeconds() << std::endl;
}

void TvTrace(ns3::Ptr<ns3::OutputStreamWrapper> stream, uint32_t oldValue,  uint32_t newValue)
{
    *stream->GetStream() << ns3::Simulator::Now().GetSeconds() << "\t" << newValue << std::endl;
}

void PacketValueTrace(ns3::Ptr<ns3::OutputStreamWrapper> stream, uint32_t oldValue, uint32_t newValue)
{
    *stream->GetStream() << ns3::Simulator::Now().GetSeconds() << "\t" << newValue << std::endl;
}

void TransferRateTrace(ns3::Ptr<ns3::OutputStreamWrapper> stream, uint32_t oldValue, uint32_t newValue)
{
    *stream->GetStream() << ns3::Simulator::Now().GetSeconds() << "\t" << newValue << std::endl;
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

void TimeTrace()
{
    std::cerr << ns3::Simulator::Now().GetSeconds() << std::endl;
    ns3::Simulator::Schedule(ns3::Seconds(1.0), &TimeTrace);
}

int main (int argc, char *argv[])
{
    ns3::CommandLine cmd;
    cmd.Parse (argc, argv);

    ns3::Time::SetResolution (ns3::Time::NS);
    float stopTime = 150.0;
    unsigned int packetSize = 1000;

    int nSilver = 0;
    int nGold = 3;
    int nBackground = 0;

    int nClients = nSilver + nGold + nBackground;

    for ( int i = 0; i < nClients ; ++i )
    {
        flowPackets.push_back(0);
    }

    ns3::AsciiTraceHelper asciiTraceHelper;

    std::string bottleneckBandwidth ="10Mbps";
    std::string bottleneckDelay = "20ms";
    std::string accessBandwidth = "10Mbps";
    std::string accessDelay = "2ms";

    ns3::Config::SetDefault("ns3::PvPieQueueDisc::MaxSize", ns3::QueueSizeValue(QueueSize ("200kB")));
    ns3::Config::SetDefault("ns3::PvPieQueueDisc::MeanPktSize", ns3::UintegerValue(packetSize));
    ns3::Config::SetDefault("ns3::PvPieQueueDisc::QueueDelayReference", ns3::StringValue("20ms"));
    ns3::Config::SetDefault("ns3::PvPieQueueDisc::Tupdate", ns3::TimeValue(ns3::MilliSeconds(100)));
    ns3::Config::SetDefault("ns3::PvPieQueueDisc::DequeueThreshold", ns3::UintegerValue(10000));
    ns3::Config::SetDefault("ns3::PvPieQueueDisc::A", ns3::DoubleValue(0.125));
    ns3::Config::SetDefault("ns3::PvPieQueueDisc::B", ns3::DoubleValue(1.25));

    ns3::Config::SetDefault ("ns3::OnOffApplication::PacketSize", ns3::UintegerValue(packetSize));
    ns3::Config::SetDefault ("ns3::OnOffApplication::DataRate", ns3::StringValue("10Mbps"));

    AQMTopologyHelper topology = AQMTopologyHelper(bottleneckBandwidth,
                                                   bottleneckDelay,
                                                   accessBandwidth,
                                                   accessDelay,
                                                   nClients);
    topology.InstallStack();
    topology.InstallPvPieTrafficControl();

    topology.InstallTrafficControl(1, DelayClass::Gold);

    topology.AssignIpv4Addresses(ns3::Ipv4AddressHelper("99.9.1.0", "255.255.255.0"),
                                 ns3::Ipv4AddressHelper("10.1.1.0", "255.255.255.0"));
    topology.InstallSinkApplication();
    topology.InstallApplication(1, Seconds(0), Seconds(150));


    ns3::Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    ns3::Simulator::Stop(ns3::Seconds(stopTime));

    /******************************************** Logging ********************************************/
    // bottleNeckLink.EnablePcap(OUTPUT_FOLDER + "/pcap/BN_", routers.Get(0)->GetId(), 0);

    ns3::Ptr<ns3::QueueDisc> qd = topology.m_bottleneckQueueDisc.Get(0);
    ns3::Ptr<ns3::PvPieQueueDisc> pqd = ns3::DynamicCast<ns3::PvPieQueueDisc>(qd);

    ns3::Ptr<ns3::OutputStreamWrapper> dropProbStream = asciiTraceHelper.CreateFileStream (OUTPUT_FOLDER + "ascii/p.bn");
    pqd->TraceConnectWithoutContext ("Probability", ns3::MakeBoundCallback(&DropProbabilityTrace, dropProbStream));
    ns3::Ptr<ns3::OutputStreamWrapper> delayStream = asciiTraceHelper.CreateFileStream (OUTPUT_FOLDER + "ascii/delay.bn");
    pqd->TraceConnectWithoutContext ("QueueingDelay", ns3::MakeBoundCallback(&DelayTrace, delayStream));
    ns3::Ptr<ns3::OutputStreamWrapper> tvStream = asciiTraceHelper.CreateFileStream (OUTPUT_FOLDER + "ascii/tv.bn");
    pqd->TraceConnectWithoutContext ("ThresholdValue", ns3::MakeBoundCallback(&TvTrace, tvStream));

    ns3::Ptr<ns3::OutputStreamWrapper> eCDFstream = asciiTraceHelper.CreateFileStream (OUTPUT_FOLDER + "ascii/ecdf.bn");
    ns3::Simulator::Schedule( ns3::Seconds(15.0), &ecdfTrace, eCDFstream, pqd);
    ns3::Simulator::Schedule( ns3::Seconds(45.0), &ecdfTrace, eCDFstream, pqd);
    ns3::Simulator::Schedule( ns3::Seconds(75.0), &ecdfTrace, eCDFstream, pqd);
    ns3::Simulator::Schedule( ns3::Seconds(105.0), &ecdfTrace, eCDFstream, pqd);
    ns3::Simulator::Schedule( ns3::Seconds(135.0), &ecdfTrace, eCDFstream, pqd);

    std::vector< ns3::Ptr<ns3::OutputStreamWrapper> > flowStreams;
    for ( int i = 0 ; i < nClients ; ++i )
    {
        ns3::Ptr<ns3::OutputStreamWrapper> flowstream = asciiTraceHelper.CreateFileStream (OUTPUT_FOLDER + "ascii/flow"+std::to_string(i)+".bn");
        flowStreams.push_back(flowstream);
    }

    // for ( int i = 0; i < nGold; ++i )
    // {
    //  ns3::Ptr<ns3::QueueDisc> gd = goldMarkers.Get(i);
    //  ns3::Ptr<ns3::GoldPacketMarkerQueueDisc> gqd = ns3::DynamicCast<ns3::GoldPacketMarkerQueueDisc>(gd);

    //  ns3::Ptr<ns3::OutputStreamWrapper> pvStream = asciiTraceHelper.CreateFileStream (OUTPUT_FOLDER + "ascii/gold_pv" + std::to_string(i) + ".bn");
    //  gqd->TraceConnectWithoutContext ("PacketValue", ns3::MakeBoundCallback(&PacketValueTrace, pvStream));
    //  ns3::Ptr<ns3::OutputStreamWrapper> transferRateStream = asciiTraceHelper.CreateFileStream (OUTPUT_FOLDER + "ascii/gold_dr" + std::to_string(i) + ".bn");
    //  gqd->TraceConnectWithoutContext ("TransferRate", ns3::MakeBoundCallback(&TransferRateTrace, transferRateStream));
    // }

    /***** throughput *****/
    ns3::Ptr<ns3::OutputStreamWrapper> throughputStream = asciiTraceHelper.CreateFileStream (OUTPUT_FOLDER + "ascii/throughput.bn");
    ns3::Ptr<ns3::OutputStreamWrapper> goldStream = asciiTraceHelper.CreateFileStream (OUTPUT_FOLDER + "ascii/throughput_gold.bn");
    ns3::Ptr<ns3::OutputStreamWrapper> silverStream = asciiTraceHelper.CreateFileStream (OUTPUT_FOLDER + "ascii/throughput_silver.bn");
    ns3::Ptr<ns3::OutputStreamWrapper> backgroundStream = asciiTraceHelper.CreateFileStream (OUTPUT_FOLDER + "ascii/throughput_background.bn");
    *throughputStream->GetStream() << std::to_string(0.0) << "\t" << 0 << std::endl;
    *goldStream->GetStream() << std::to_string(0.0) << "\t" << 0 << std::endl;
    *silverStream->GetStream() << std::to_string(0.0) << "\t" << 0 << std::endl;
    *backgroundStream->GetStream() << std::to_string(0.0) << "\t" << 0 << std::endl;
    topology.m_routerDevices.Get(1)->TraceConnectWithoutContext("PhyRxEnd", ns3::MakeCallback(&RxTrace));

    ns3::Simulator::Schedule( ns3::Seconds(1.0), &ThroughputTrace, throughputStream, goldStream, silverStream, backgroundStream);

    /******************************************** FLOW monitor ********************************************/

    ns3::Ptr<ns3::FlowMonitor> flowmon;
    ns3::FlowMonitorHelper flowmonHelper;
    flowmon = flowmonHelper.InstallAll();
    ns3::Simulator::Schedule( ns3::Seconds(0.0), &ThroughputMonitor, flowStreams, flowmon, DynamicCast<Ipv4FlowClassifier> (flowmonHelper.GetClassifier()));

    /********   ************************************ end of preconfiguration ********************************************/

    ns3::Simulator::Schedule( ns3::Seconds(1.0), &TimeTrace);

    ns3::Simulator::Run ();

    *throughputStream->GetStream() << std::to_string(stopTime) << "\t" << receivedPackets * 8 / 1000.0 / 1000.0 << std::endl;
    *goldStream->GetStream() << std::to_string(stopTime) << "\t" << goldPackets * 8 / 1000.0 / 1000.0 << std::endl;
    *silverStream->GetStream() << std::to_string(stopTime) << "\t" << silverPackets * 8 / 1000.0 / 1000.0 << std::endl;
    *backgroundStream->GetStream() << std::to_string(stopTime) << "\t" << backgroundPackets * 8 / 1000.0 / 1000.0 << std::endl;

    ns3::Simulator::Destroy();
    return 0;
}
