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
	;

	return tid;
}

PacketMarkerQueueDisc::PacketMarkerQueueDisc() : QueueDisc()
{
	NS_LOG_FUNCTION (this);
	// schedule iterative calculations
	// initialize variables if needed

}

PacketMarkerQueueDisc::~PacketMarkerQueueDisc ()
{
	NS_LOG_FUNCTION (this);
}

void PacketMarkerQueueDisc::DoDispose (void)
{
	NS_LOG_FUNCTION (this);
	QueueDisc::DoDispose();
}

void PacketMarkerQueueDisc::InitializeParams(void)
{
	m_granularity = 150;
	NS_LOG_INFO ("Initializing marker params.");
	for ( int i = 0; i < m_granularity; ++i )
	{
		m_tokenBuckets.push_back(TokenBucket());
	}
}

bool PacketMarkerQueueDisc::DoEnqueue (Ptr<QueueDiscItem> item)
{
	NS_LOG_FUNCTION (this << item);

	uint16_t pv = 3333;

	MyTag tag;
	tag.SetSimpleValue(pv);
	item->GetPacket()->AddPacketTag(tag);

	bool retval = GetInternalQueue(0)->Enqueue(item);

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
		Ptr<InternalQueue> queue = CreateObjectWithAttributes<DropTailQueue<QueueDiscItem> > ("Mode", EnumValue(QUEUE_DISC_MODE_PACKETS));
		queue->SetMaxPackets (5);
		AddInternalQueue (queue);
	}

	if (GetNInternalQueues () != 1)
	{
		NS_LOG_ERROR ("PacketMarkerQueueDisc needs 1 internal queue");
		return false;
	}

	return true;
}




} /* namespace ns3 */
