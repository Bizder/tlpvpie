/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/log.h"
#include "ns3/enum.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/simulator.h"
#include "ns3/abort.h"
#include "pv-pie-queue-disc.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/net-device-queue-interface.h"
#include "packet-value-tag.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("PvPieQueueDisc");

NS_OBJECT_ENSURE_REGISTERED(PvPieQueueDisc);

TypeId PvPieQueueDisc::GetTypeId(void)
{
static TypeId tid = TypeId("ns3::PvPieQueueDisc")
	.SetParent<QueueDisc>()
	.SetGroupName("pvpie")
	.AddConstructor<PvPieQueueDisc> ()
	.AddAttribute ("MeanPktSize",
					"Average of packet size",
					UintegerValue(64),
					MakeUintegerAccessor (&PvPieQueueDisc::m_meanPktSize),
					MakeUintegerChecker<uint32_t> ())
	.AddAttribute ("A",
					"Value of alpha",
					DoubleValue(0.125),
					MakeDoubleAccessor(&PvPieQueueDisc::m_a),
					MakeDoubleChecker<double> ())
	.AddAttribute ("B",
					"Value of beta",
					DoubleValue(1.25),
					MakeDoubleAccessor(&PvPieQueueDisc::m_b),
					MakeDoubleChecker<double> ())
	.AddAttribute ("Tupdate",
					"Time period to calculate drop probability",
					TimeValue(MilliSeconds(100)),
					MakeTimeAccessor (&PvPieQueueDisc::m_tUpdate),
					MakeTimeChecker())
	.AddAttribute ("QueueLimit",
					"Queue limit in bytes/packets",
					UintegerValue(100000), // 100 kbytes
					MakeUintegerAccessor(&PvPieQueueDisc::SetQueueLimit),
					MakeUintegerChecker<uint32_t>())
	.AddAttribute ("DequeueThreshold",
					"Minimum queue size in bytes before dequeue rate is measured",
					UintegerValue(20000),
					MakeUintegerAccessor (&PvPieQueueDisc::m_dqThreshold),
					MakeUintegerChecker<uint32_t>())
	.AddAttribute ("QueueDelayReference",
					"Desired queue delay",
					TimeValue(MilliSeconds(20)),
					MakeTimeAccessor(&PvPieQueueDisc::m_qDelayRef),
					MakeTimeChecker())
	.AddAttribute ("MaxBurstAllowance",
					"Current max burst allowance in seconds before random drop",
					TimeValue(MilliSeconds(100)),
					MakeTimeAccessor(&PvPieQueueDisc::m_maxBurst),
					MakeTimeChecker())
	.AddTraceSource("QueueingDelay",
					"Queueing Delay",
					MakeTraceSourceAccessor(&PvPieQueueDisc::m_qDelay),
					"ns3::Time::TracedValueCallback")
	;

	return tid;
}

PvPieQueueDisc::PvPieQueueDisc(void) : QueueDisc ()
{
	NS_LOG_FUNCTION (this);
	m_rtrsEvent = Simulator::Schedule(TimeValue(Seconds(0)), &PvPieQueueDisc::CalculateP, this);
	m_ecdf = eCDF();
}

PvPieQueueDisc::~PvPieQueueDisc(void)
{
	NS_LOG_FUNCTION (this);
}

void PvPieQueueDisc::DoDispose (void)
{
	NS_LOG_FUNCTION (this);
	Simulator::Remove(m_rtrsEvent);
	QueueDisc::DoDispose();
}

void PvPieQueueDisc::SetQueueLimit(uint32_t lim)
{
  NS_LOG_FUNCTION (this << lim);
  m_queueLimit = lim;
}

uint32_t PvPieQueueDisc::GetQueueSize(void)
{
	NS_LOG_FUNCTION (this);
	return GetInternalQueue(0)->GetNBytes();
}

Time PvPieQueueDisc::GetQueueDelay(void)
{
	NS_LOG_FUNCTION (this);
	return m_qDelay;
}

void PvPieQueueDisc::InitializeParams(void)
{
	m_inMeasurement = false;
	m_dqCount = 0;
	m_thresholdValue = 0;
	m_avgDqRate = 0.0;
	m_start = 0;
	m_burstState = NO_BURST;
	m_qDelayOld = Time(MilliSeconds(0));
}

bool PvPieQueueDisc::DoEnqueue(Ptr<QueueDiscItem> item)
{
	NS_LOG_FUNCTION (this << item);

	uint32_t nQueued = GetQueueSize();

	PacketValueTag tag;
	item->GetPacket()->PeekPacketTag(tag);
	m_ecdf.AddValue(Simulator::Now(), tag.GetPacketValue());

	if (nQueued + item->GetSize() > m_queueLimit)
	{
		// Drops due to queue limit: reactive
		DropBeforeEnqueue(item, FORCED_DROP);
		return false;
	}
	else if (DropEarly(item, nQueued, tag.GetPacketValue()))
	{
		// Early probability drop: proactive
		DropBeforeEnqueue(item, UNFORCED_DROP);
		return false;
	}

	// No drop
	bool retval = GetInternalQueue(0)->Enqueue(item);

	// If Queue::Enqueue fails, QueueDisc::DropBeforeEnqueue is called by the
	// internal queue because QueueDisc::AddInternalQueue sets the trace callback

	NS_LOG_LOGIC ("\t bytesInQueue  " << GetInternalQueue (0)->GetNBytes ());
	NS_LOG_LOGIC ("\t packetsInQueue  " << GetInternalQueue (0)->GetNPackets ());

	return retval;
}

bool PvPieQueueDisc::DropEarly(Ptr<QueueDiscItem> item, uint32_t qSize, uint32_t packet_value)
{
	NS_LOG_FUNCTION (this << item << qSize);
	if (m_burstAllowance.GetSeconds() > 0)
	{
		return false;
	}

	return packet_value < m_thresholdValue;
}

void PvPieQueueDisc::CalculateP(void)
{
	NS_LOG_FUNCTION (this);

	m_ecdf.RemoveOldValues();

	double p = 0.0;
	if (m_avgDqRate > 0)
	{
		m_qDelay = Time(Seconds(GetInternalQueue(0)->GetNBytes() / m_avgDqRate));
	}
	else
	{
		m_qDelay = Time(Seconds(0));
	}

	if (m_burstAllowance.GetSeconds() > 0)
	{
		m_dropProb = 0;
	}
	else
	{
		p = m_a * (m_qDelay.GetSeconds() - m_qDelayRef.GetSeconds()) + m_b * (m_qDelay.GetSeconds() - m_qDelayOld.GetSeconds());
		if (p < 0.01)
		{
			p /= 8;
		}
		else if (p < 0.1)
		{
			p /= 2;
		}
	}

	m_dropProb += p;
	m_dropProb = (m_dropProb > 0) ? m_dropProb : 0;

	m_thresholdValue = ecdf.GetThresholdValue(m_dropProb);

	m_qDelayOld = qDelay;
	m_rtrsEvent = Simulator::Schedule(m_tUpdate, &PvPieQueueDisc::CalculateP, this);
}

Ptr<QueueDiscItem> PvPieQueueDisc::DoDequeue(void)
{
	NS_LOG_FUNCTION (this);

	if ( GetInternalQueue (0)->IsEmpty() )
	{
		NS_LOG_LOGIC ("Queue empty");
		return 0;
	}

	Ptr<QueueDiscItem> item = GetInternalQueue(0)->Dequeue();
	double now = Simulator::Now().GetSeconds();
	uint32_t dqPktSize = item->GetSize();
	double e = 0.5;

	if ( (GetInternalQueue(0)->GetNBytes() >= m_dqThreshold) && (!m_inMeasurement) )
	{
		m_inMeasurement = true;
	}

	if (m_inMeasurement)
	{
		m_dqCount += dqPktSize;

		if (m_dqCount > m_dqThreshold)
		{
			double dqInt = now - m_start;
			double dqRate = m_dqCount / dqInt;
			m_avgDqRate = (1-e)*m_avgDqRate + e*dqRate;
			m_dqCount = 0;
			m_start = now;

			m_burstAllowance -= dqInt;

			if (m_dropProb == 0 && m_qDelay < m_qDelayRef / 2 && m_qDelayOld < m_qDelayRef / 2)
			{
				m_burstAllowance = m_maxBurst;
			}

			m_inMeasurement = false;
		}
	}

	return item;
}

Ptr<const QueueDiscItem> PvPieQueueDisc::DoPeek(void) const
{
	NS_LOG_FUNCTION (this);
	if (GetInternalQueue (0)->IsEmpty())
	{
		NS_LOG_LOGIC ("Queue empty");
		return 0;
	}

	Ptr<const QueueDiscItem> item = GetInternalQueue(0)->Peek();

	NS_LOG_LOGIC ("Number packets " << GetInternalQueue(0)->GetNPackets());
	NS_LOG_LOGIC ("Number bytes " << GetInternalQueue(0)->GetNBytes());

	return item;
}

bool PvPieQueueDisc::CheckConfig(void)
{
	NS_LOG_FUNCTION (this);
	if (GetNQueueDiscClasses() > 0)
	{
		NS_LOG_ERROR ("PvPieQueueDisc cannot have classes");
		return false;
	}

	if (GetNPacketFilters() > 0)
	{
		NS_LOG_ERROR ("PvPieQueueDisc cannot have packet filters");
		return false;
	}

	if (GetNInternalQueues() == 0)
	{
		Ptr<InternalQueue> queue = CreateObjectWithAttributes<DropTailQueue<QueueDiscItem> > ("Mode", EnumValue(QueueBase::QueueMode::QUEUE_MODE_PACKETS));
		queue->SetMaxBytes(m_queueLimit);
		AddInternalQueue(queue);
	}

	if (GetNInternalQueues () != 1)
	{
		NS_LOG_ERROR ("PvPieQueueDisc needs 1 internal queue");
		return false;
	}

	return true;
}

} // namespace ns3

