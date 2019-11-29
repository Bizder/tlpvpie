 /* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

/*
Packet value expressed as value per bit.
Congestion Threshold Value in queuedisc [pvpie] (CTV)

Quantize data: b[i] = 10^(1+1/30*i) kbps , i e [0, 149] [10 kbps, 1Gbps]
			   V[i] = V(b[i])

Tocken buckets:
				l_max[i] = b[i]*d (averaging delay)
				l[i] = l_max[i] <- intialized
				filled continuously with b[i] speed


		DoEnqueue:
			packet size: s bytes
			PV: V[k] -> k = min([1,149] | l[i] >= s)  -> l[j] = l[j] - s if j > k
*/

#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/abort.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/net-device-queue-interface.h"
#include "ns3/queue-disc.h"
#include "packet-marker-queue-disc.h"
#include "packet-value-tag.h"

namespace ns3 {


NS_LOG_COMPONENT_DEFINE("PacketMarkerQueueDisc");

NS_OBJECT_ENSURE_REGISTERED(PacketMarkerQueueDisc);

TypeId PacketMarkerQueueDisc::GetTypeId (void)
{
static TypeId tid = TypeId ("ns3::PacketMarkerQueueDisc")
	.SetParent<QueueDisc>()
	.SetGroupName ("pvpie")
	.AddConstructor<PacketMarkerQueueDisc>()
	.AddAttribute ("A",
					"Value of alpha in EWMA calculation",
					DoubleValue (0.1),
					MakeDoubleAccessor(&PacketMarkerQueueDisc::m_a),
					MakeDoubleChecker<double> ())
	.AddTraceSource("PacketValue",
					"Probability of packet droping",
					MakeTraceSourceAccessor (&PacketMarkerQueueDisc::m_lastpv),
					"ns3::TracedValueCallback::UInteger")
	.AddTraceSource("TransferRate",
					"Transfer rate of application calculated by EWMA",
					MakeTraceSourceAccessor (&PacketMarkerQueueDisc::m_transferRate),
					"ns3::TracedValueCallback::UInteger")
    .AddAttribute ("MaxSize",
                   "The maximum number of packets accepted by this queue disc",
                   QueueSizeValue (QueueSize ("5p")),
                   MakeQueueSizeAccessor (&QueueDisc::SetMaxSize,
                                          &QueueDisc::GetMaxSize),
                   MakeQueueSizeChecker ())
	;

	return tid;
}

PacketMarkerQueueDisc::PacketMarkerQueueDisc() : QueueDisc()
{
	NS_LOG_FUNCTION (this);
	m_uv = CreateObject<UniformRandomVariable> ();
	// m_rtrsEvent = Simulator::Schedule (m_sUpdate, &PvPieQueueDisc::CalculateP, this);
}

PacketMarkerQueueDisc::~PacketMarkerQueueDisc ()
{
	NS_LOG_FUNCTION (this);
}

void PacketMarkerQueueDisc::DoDispose (void)
{
	NS_LOG_FUNCTION (this);
	m_uv = 0;
	// Simulator::Remove (m_rtrsEvent);
	QueueDisc::DoDispose();
}

int64_t PacketMarkerQueueDisc::AssignStreams (int64_t stream)
{
	NS_LOG_FUNCTION (this << stream);
	m_uv->SetStream (stream);
	return 1;
}

void PacketMarkerQueueDisc::InitializeParams(void)
{
	NS_LOG_INFO ("Initializing marker params.");
	m_transferRate = 0;
	m_lastSend = Time(Seconds(0));
}

bool PacketMarkerQueueDisc::DoEnqueue (Ptr<QueueDiscItem> item)
{
	NS_LOG_FUNCTION (this << item);

	double u = m_uv->GetValue();

	uint32_t packet_value = throughputValueFunction(m_transferRate);

	packet_value = (500000000 - packet_value) * u + packet_value;

	PacketValueTag tag;
	tag.SetPacketValue(packet_value);
	tag.SetDelayClass(getPriority());
	item->GetPacket()->AddPacketTag(tag);

	bool retval = GetInternalQueue(0)->Enqueue(item);

	Time now = Simulator::Now();
	double time_delta = now.GetSeconds() - m_lastSend.GetSeconds();
	uint32_t current_rate = item->GetSize() * 8 / time_delta;

	m_transferRate = m_a * current_rate + ( 1 - m_a ) * m_transferRate;
	m_lastSend = now;

	m_lastpv = packet_value;

	NS_LOG_LOGIC ("\t bytesInQueue  " << GetInternalQueue (0)->GetNBytes ());
	NS_LOG_LOGIC ("\t packetsInQueue  " << GetInternalQueue (0)->GetNPackets ());

	return retval;
}

Ptr<QueueDiscItem> PacketMarkerQueueDisc::DoDequeue (void)
{
	NS_LOG_FUNCTION (this);

	if (GetInternalQueue (0)->IsEmpty ())
	{
		NS_LOG_LOGIC ("Queue empty");
		return 0;
	}

	Ptr<QueueDiscItem> item = GetInternalQueue (0)->Dequeue();

	return item;
}

Ptr<const QueueDiscItem> PacketMarkerQueueDisc::DoPeek (void) const
{
	NS_LOG_FUNCTION (this);
	if (GetInternalQueue (0)->IsEmpty())
	{
		NS_LOG_LOGIC ("Queue empty");
		return 0;
	}

	Ptr<const QueueDiscItem> item = GetInternalQueue(0)->Peek();

	NS_LOG_LOGIC ("Number packets " << GetInternalQueue (0)->GetNPackets());
	NS_LOG_LOGIC ("Number bytes " << GetInternalQueue (0)->GetNBytes());

	return item;
}

bool PacketMarkerQueueDisc::CheckConfig (void)
{
	NS_LOG_FUNCTION (this);
	if (GetNQueueDiscClasses () > 0)
	{
		NS_LOG_ERROR ("PacketMarkerQueueDisc cannot have classes");
		return false;
	}

	if (GetNPacketFilters () > 0)
	{
		NS_LOG_ERROR ("PacketMarkerQueueDisc cannot have packet filters");
		return false;
	}

	if (GetNInternalQueues () == 0)
	{
		// create a DropTail queue
      AddInternalQueue (CreateObjectWithAttributes<DropTailQueue<QueueDiscItem> >
                        ("MaxSize", QueueSizeValue (GetMaxSize ())));
	}

	if (GetNInternalQueues () != 1)
	{
		NS_LOG_ERROR ("PacketMarkerQueueDisc needs 1 internal queue");
		return false;
	}

	return true;
}

uint32_t PacketMarkerQueueDisc::throughputValueFunction(uint32_t rate_kbps)
{
	return 0;
}

uint8_t PacketMarkerQueueDisc::getPriority()
{
	return 0;
}

/******************************** GOLD MARKER ********************************/

NS_OBJECT_ENSURE_REGISTERED(GoldPacketMarkerQueueDisc);

TypeId GoldPacketMarkerQueueDisc::GetTypeId (void)
{
static TypeId tid = TypeId ("ns3::GoldPacketMarkerQueueDisc")
	.SetParent<PacketMarkerQueueDisc>()
	.SetGroupName ("pvpie")
	.AddConstructor<GoldPacketMarkerQueueDisc>()
	;

	return tid;
}

// x is bit per second
uint32_t GoldPacketMarkerQueueDisc::throughputValueFunction(uint32_t x)
{
	uint32_t y = 500000000 - x * 5;
	//uint32_t y = 100000 - x;
	// double y = 212500000.0 / 3123.0 - (2125.0 * rate_kbps ) / 3123.0;
	return y > 0 ? y : 0;
}

uint8_t GoldPacketMarkerQueueDisc::getPriority()
{
	return 1;
}

/******************************** SILVER MARKER ********************************/

NS_OBJECT_ENSURE_REGISTERED(SilverPacketMarkerQueueDisc);

TypeId SilverPacketMarkerQueueDisc::GetTypeId (void)
{
static TypeId tid = TypeId ("ns3::SilverPacketMarkerQueueDisc")
	.SetParent<PacketMarkerQueueDisc>()
	.SetGroupName ("pvpie")
	.AddConstructor<SilverPacketMarkerQueueDisc>()
	;

	return tid;
}

uint32_t SilverPacketMarkerQueueDisc::throughputValueFunction(uint32_t x)
{
	uint32_t y = 500000000 - x * 10;
	// uint32_t y = 100000 - x * 1.5;
	// double y = 212500000.0 / 3123.0 - ( 4250.0 * rate_kbps ) / 3123.0;
	return y > 0 ? y : 0;
}

uint8_t SilverPacketMarkerQueueDisc::getPriority()
{
	return 2;
}

/******************************** BACKGROUND MARKER ********************************/

NS_OBJECT_ENSURE_REGISTERED(BackgroundPacketMarkerQueueDisc);

TypeId BackgroundPacketMarkerQueueDisc::GetTypeId (void)
{
static TypeId tid = TypeId ("ns3::BackgroundPacketMarkerQueueDisc")
	.SetParent<PacketMarkerQueueDisc>()
	.SetGroupName ("pvpie")
	.AddConstructor<BackgroundPacketMarkerQueueDisc>()
	;

	return tid;
}

uint32_t BackgroundPacketMarkerQueueDisc::throughputValueFunction(uint32_t x)
{
	uint32_t y = 500000000 - x * 100;
	// uint32_t y = 100000 - x * 1.5 * 5;
	// double y = 212500000.0 / 3123.0 - ( 4250.0 * rate_kbps ) / 3123.0;
	return y > 0 ? y : 0;
}

uint8_t BackgroundPacketMarkerQueueDisc::getPriority()
{
	return 3;
}

} /* namespace ns3 */
