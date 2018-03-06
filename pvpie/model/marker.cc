/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

/*
Packet value expressed as value per bit.
Congestion Threshold Value in queuedisc [pvpie] (CTV)

*/

#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/abort.h"
#include "marker.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/net-device-queue-interface.h"


namespace ns3 {


NS_LOG_COMPONENT_DEFINE("PacketMarkerQueueDisc");

NS_OBJECT_ENSURE_REGISTERED(PacketMarkerQueueDisc);

TypeId PacketMarkerQueueDisc::GetTypeId (void)
{
static TypeId tid = TypeId ("ns3::PvPieQueueDisc")
	.SetParent<QueueDisc>()
	.SetGroupName ("pvpie")
	.AddConstructor<PacketMarkerQueueDisc>()
	// .AddAttribute ("A",
	// 				"Value of alpha",
	// 				DoubleValue (0.125),
	// 				MakeDoubleAccessor (&PvPieQueueDisc::m_a),
	// 				MakeDoubleChecker<double> ())
	// .AddTraceSource("Probability",
	// 				"Probability of packet droping",
	// 				MakeTraceSourceAccessor (&PvPieQueueDisc::m_dropProb),
	// 				"ns3::TracedValueCallback::Double")

	return tid;
}

PacketMarkerQueueDisc::PacketMarkerQueueDisc() : QueueDisc()
{
	NS_LOG_FUNCTION (this);
	// schedule iterative calculations
	// initialize variables if needed
}

PacketMarkerQueueDisc::InitializeParams(void)
{
	NS_LOG_INFO ("Initializing marker params.");
}






} /* namespace ns3 */
