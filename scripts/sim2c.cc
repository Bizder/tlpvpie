#include <vector>

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


#include "tracehelper.h"

#define OUTPUT_FOLDER std::string("output/")

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("PvPieSimulator");

int main (int argc, char *argv[])
{
    ns3::CommandLine cmd;
    cmd.Parse (argc, argv);

    ns3::Time::SetResolution (ns3::Time::NS);

    float stopTime = 150.0;
    unsigned int packetSize = 1000;

    int nGold = 3;
    int nSilver = 0;
    int nBackground = 0;
    int nClients = nGold + nSilver + nBackground;

    for ( int i = 0; i < nClients ; ++i )
    {
        flowPackets.push_back(0);
    }

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
                                                   accessDelay);

    topology.ConfigureLeaf(DelayClass::Gold, Seconds(0), Seconds(0));
    topology.ConfigureLeaf(DelayClass::Gold, Seconds(0), Seconds(0));
    topology.ConfigureLeaf(DelayClass::Gold, Seconds(0), Seconds(0));
    topology.ConfigureLeaf(DelayClass::Gold, Seconds(30), Seconds(120));
    topology.ConfigureLeaf(DelayClass::Gold, Seconds(30), Seconds(120));
    topology.ConfigureLeaf(DelayClass::Gold, Seconds(30), Seconds(120));
    topology.ConfigureLeaf(DelayClass::Gold, Seconds(60), Seconds(90));
    topology.ConfigureLeaf(DelayClass::Gold, Seconds(60), Seconds(90));
    topology.ConfigureLeaf(DelayClass::Gold, Seconds(60), Seconds(90));

    topology.ConfigureLeaf(DelayClass::Silver, Seconds(0), Seconds(0));
    topology.ConfigureLeaf(DelayClass::Silver, Seconds(0), Seconds(0));
    topology.ConfigureLeaf(DelayClass::Silver, Seconds(0), Seconds(0));
    topology.ConfigureLeaf(DelayClass::Silver, Seconds(30), Seconds(120));
    topology.ConfigureLeaf(DelayClass::Silver, Seconds(30), Seconds(120));
    topology.ConfigureLeaf(DelayClass::Silver, Seconds(30), Seconds(120));
    topology.ConfigureLeaf(DelayClass::Silver, Seconds(60), Seconds(90));
    topology.ConfigureLeaf(DelayClass::Silver, Seconds(60), Seconds(90));
    topology.ConfigureLeaf(DelayClass::Silver, Seconds(60), Seconds(90));

    for ( int i = 0; i < nBackground ; ++i ) {
        topology.ConfigureLeaf(DelayClass::Background, Seconds(0), Seconds(0));
    }

    topology.Initialize();
    topology.InstallStack();
    topology.InstallPvPieTrafficControl();
    topology.InstallPacketMarkers();
    topology.AssignIpv4Addresses(ns3::Ipv4AddressHelper("99.9.1.0", "255.255.255.0"),
                       ns3::Ipv4AddressHelper("10.1.1.0", "255.255.255.0"));
    topology.InstallSinkApplication();
    topology.InstallSourceApplications();

    /******************************************** Logging ********************************************/
    // bottleNeckLink.EnablePcap(OUTPUT_FOLDER + "/pcap/BN_", routers.Get(0)->GetId(), 0);

    ns3::AsciiTraceHelper asciiTraceHelper;

    ns3::Ptr<ns3::QueueDisc> qd = topology.m_bottleneckQueueDisc.Get(0);
    ns3::Ptr<ns3::PvPieQueueDisc> pqd = ns3::DynamicCast<ns3::PvPieQueueDisc>(qd);

    ns3::Ptr<ns3::OutputStreamWrapper> dropProbStream = asciiTraceHelper.CreateFileStream (OUTPUT_FOLDER + "ascii/p.bn");
    pqd->TraceConnectWithoutContext ("Probability", ns3::MakeBoundCallback(&DropProbabilityTrace, dropProbStream));
    ns3::Ptr<ns3::OutputStreamWrapper> delayStream = asciiTraceHelper.CreateFileStream (OUTPUT_FOLDER + "ascii/delay.bn");
    pqd->TraceConnectWithoutContext ("QueueingDelay", ns3::MakeBoundCallback(&QueueDelayTrace, delayStream));
    ns3::Ptr<ns3::OutputStreamWrapper> tvStream = asciiTraceHelper.CreateFileStream (OUTPUT_FOLDER + "ascii/tv.bn");
    pqd->TraceConnectWithoutContext ("ThresholdValue", ns3::MakeBoundCallback(&ThresholdValueTrace, tvStream));

    ns3::Ptr<ns3::OutputStreamWrapper> eCDFstream = asciiTraceHelper.CreateFileStream (OUTPUT_FOLDER + "ascii/ecdf.bn");
    for ( int i = 0; i < stopTime ; i = i + 15 ) { ns3::Simulator::Schedule( ns3::Seconds(0), &ecdfTrace, eCDFstream, pqd); }

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

    ns3::Ptr<ns3::FlowMonitor> flowmon;
    ns3::FlowMonitorHelper flowmonHelper;
    flowmon = flowmonHelper.InstallAll();
    ns3::Simulator::Schedule( ns3::Seconds(0.0), &ThroughputMonitor, flowStreams, flowmon, DynamicCast<Ipv4FlowClassifier>(flowmonHelper.GetClassifier()));

    ns3::Simulator::Schedule( ns3::Seconds(1.0), &TimeTrace);

    ns3::Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    ns3::Simulator::Stop(ns3::Seconds(stopTime));
    ns3::Simulator::Run ();
    ns3::Simulator::Destroy();

    *throughputStream->GetStream() << std::to_string(stopTime) << "\t" << receivedPackets * 8 / 1000.0 / 1000.0 << std::endl;
    *goldStream->GetStream() << std::to_string(stopTime) << "\t" << goldPackets * 8 / 1000.0 / 1000.0 << std::endl;
    *silverStream->GetStream() << std::to_string(stopTime) << "\t" << silverPackets * 8 / 1000.0 / 1000.0 << std::endl;
    *backgroundStream->GetStream() << std::to_string(stopTime) << "\t" << backgroundPackets * 8 / 1000.0 / 1000.0 << std::endl;

    return 0;
}


