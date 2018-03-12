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

TypeId PvPieQueueDisc::GetTypeId (void)
{
static TypeId tid = TypeId ("ns3::PvPieQueueDisc")
	.SetParent<QueueDisc> ()
	.SetGroupName ("pvpie")
	.AddConstructor<PvPieQueueDisc> ()
	.AddAttribute ("Mode",
					"Determines unit for QueueLimit",
					EnumValue (QUEUE_DISC_MODE_PACKETS),
					MakeEnumAccessor (&PvPieQueueDisc::SetMode),
					MakeEnumChecker (QUEUE_DISC_MODE_BYTES, "QUEUE_DISC_MODE_BYTES",
									QUEUE_DISC_MODE_PACKETS, "QUEUE_DISC_MODE_PACKETS"))
	.AddAttribute ("MeanPktSize",
					"Average of packet size",
					UintegerValue (64),
					MakeUintegerAccessor (&PvPieQueueDisc::m_meanPktSize),
					MakeUintegerChecker<uint32_t> ())
	.AddAttribute ("A",
					"Value of alpha",
					DoubleValue (0.125),
					MakeDoubleAccessor (&PvPieQueueDisc::m_a),
					MakeDoubleChecker<double> ())
	.AddAttribute ("B",
					"Value of beta",
					DoubleValue (1.25),
					MakeDoubleAccessor (&PvPieQueueDisc::m_b),
					MakeDoubleChecker<double> ())
	.AddAttribute ("Tupdate",
					"Time period to calculate drop probability",
					TimeValue (Seconds (0.032)),
					MakeTimeAccessor (&PvPieQueueDisc::m_tUpdate),
					MakeTimeChecker ())
	.AddAttribute ("Supdate",
					"Start time of the update timer",
					TimeValue (Seconds (0)),
					MakeTimeAccessor (&PvPieQueueDisc::m_sUpdate),
					MakeTimeChecker ())
	.AddAttribute ("QueueLimit",
					"Queue limit in bytes/packets",
					UintegerValue (25),
					MakeUintegerAccessor (&PvPieQueueDisc::SetQueueLimit),
					MakeUintegerChecker<uint32_t> ())
	.AddAttribute ("DequeueThreshold",
					"Minimum queue size in bytes before dequeue rate is measured",
					UintegerValue (10000),
					MakeUintegerAccessor (&PvPieQueueDisc::m_dqThreshold),
					MakeUintegerChecker<uint32_t> ())
	.AddAttribute ("QueueDelayReference",
					"Desired queue delay",
					TimeValue (Seconds (0.02)),
					MakeTimeAccessor (&PvPieQueueDisc::m_qDelayRef),
					MakeTimeChecker ())
	.AddAttribute ("MaxBurstAllowance",
					"Current max burst allowance in seconds before random drop",
					TimeValue (Seconds (0.1)),
					MakeTimeAccessor (&PvPieQueueDisc::m_maxBurst),
					MakeTimeChecker ())
	.AddTraceSource("Probability",
					"Probability of packet droping",
					MakeTraceSourceAccessor (&PvPieQueueDisc::m_dropProb),
					"ns3::TracedValueCallback::Double")
	.AddTraceSource("QueueingDelay",
					"Queueing Delay",
					MakeTraceSourceAccessor (&PvPieQueueDisc::m_qDelay),
					"ns3::Time::TracedValueCallback")
	;

	return tid;
}

PvPieQueueDisc::PvPieQueueDisc () : QueueDisc ()
{
	NS_LOG_FUNCTION (this);
	m_uv = CreateObject<UniformRandomVariable> ();
	m_rtrsEvent = Simulator::Schedule (m_sUpdate, &PvPieQueueDisc::CalculateP, this);
}

PvPieQueueDisc::~PvPieQueueDisc ()
{
	NS_LOG_FUNCTION (this);
}

void PvPieQueueDisc::DoDispose (void)
{
	NS_LOG_FUNCTION (this);
	m_uv = 0;
	Simulator::Remove (m_rtrsEvent);
	QueueDisc::DoDispose ();
}

void PvPieQueueDisc::SetMode (QueueDiscMode mode)
{
  NS_LOG_FUNCTION (this << mode);
  m_mode = mode;
}

PvPieQueueDisc::QueueDiscMode PvPieQueueDisc::GetMode (void)
{
  NS_LOG_FUNCTION (this);
  return m_mode;
}

void PvPieQueueDisc::SetQueueLimit (uint32_t lim)
{
  NS_LOG_FUNCTION (this << lim);
  m_queueLimit = lim;
}

uint32_t PvPieQueueDisc::GetQueueSize (void)
{
	NS_LOG_FUNCTION (this);
	if (GetMode () == QUEUE_DISC_MODE_BYTES)
	{
		return GetInternalQueue (0)->GetNBytes ();
	}
	else if (GetMode () == QUEUE_DISC_MODE_PACKETS)
	{
		return GetInternalQueue (0)->GetNPackets ();
	}
	else
	{
		NS_ABORT_MSG ("Unknown PIE mode.");
	}
}

Time PvPieQueueDisc::GetQueueDelay (void)
{
	NS_LOG_FUNCTION (this);
	return m_qDelay;
}

int64_t PvPieQueueDisc::AssignStreams (int64_t stream)
{
	NS_LOG_FUNCTION (this << stream);
	m_uv->SetStream (stream);
	return 1;
}

void PvPieQueueDisc::InitializeParams (void)
{
	// Initially queue is empty so variables are initialize to zero except m_dqCount
	m_inMeasurement = false;
	m_dqCount = -1;
	m_dropProb = 0;
	m_avgDqRate = 0.0;
	m_dqStart = 0;
	m_burstState = NO_BURST;
	m_qDelayOld = Time(Seconds(0));
}

bool PvPieQueueDisc::DoEnqueue (Ptr<QueueDiscItem> item)
{
	NS_LOG_FUNCTION (this << item);

	uint32_t nQueued = GetQueueSize ();

	if ((GetMode () == QUEUE_DISC_MODE_PACKETS && nQueued >= m_queueLimit)
		|| (GetMode () == QUEUE_DISC_MODE_BYTES && nQueued + item->GetSize () > m_queueLimit))
	{
		// Drops due to queue limit: reactive
		DropBeforeEnqueue (item, FORCED_DROP);
		return false;
	}
	else if (DropEarly (item, nQueued))
	{
		// Early probability drop: proactive
		DropBeforeEnqueue (item, UNFORCED_DROP);
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

bool PvPieQueueDisc::DropEarly (Ptr<QueueDiscItem> item, uint32_t qSize)
{
	NS_LOG_FUNCTION (this << item << qSize);
	if (m_burstAllowance.GetSeconds () > 0)
	{
		// If there is still burst_allowance left, skip random early drop.
		return false;
	}

	if (m_burstState == NO_BURST)
	{
		m_burstState = IN_BURST_PROTECTING;
		m_burstAllowance = m_maxBurst;
	}

	double p = m_dropProb;

	uint32_t packetSize = item->GetSize ();

	if (GetMode () == QUEUE_DISC_MODE_BYTES)
	{
		p = p * packetSize / m_meanPktSize;
	}
	double u =  m_uv->GetValue ();

	if ((m_qDelayOld.GetSeconds () < (0.5 * m_qDelayRef.GetSeconds ())) && (m_dropProb < 0.2))
	{
		return false;
	}
	else if (GetMode() == QUEUE_DISC_MODE_BYTES && qSize <= 2 * m_meanPktSize)
	{
		return false;
	}
	else if (GetMode() == QUEUE_DISC_MODE_PACKETS && qSize <= 2)
	{
		return false;
	}

	MyTag tag;
	item->GetPacket()->PeekPacketTag(tag);
	packet_vaue = tag.GetSimpleValue();

	if (u > p)
	{
		return false;
	}
	return true;
}

void PvPieQueueDisc::CalculateP()
{
	NS_LOG_FUNCTION (this);
	Time qDelay;
	double p = 0.0;
	bool missingInitFlag = false;
	if (m_avgDqRate > 0)
	{
		qDelay = Time (Seconds (GetInternalQueue (0)->GetNBytes () / m_avgDqRate));
	}
	else
	{
		qDelay = Time (Seconds (0));
		missingInitFlag = true;
	}

	m_qDelay = qDelay;

	if (m_burstAllowance.GetSeconds () > 0)
	{
		m_dropProb = 0;
	}
	else
	{
		p = m_a * (qDelay.GetSeconds () - m_qDelayRef.GetSeconds ()) + m_b * (qDelay.GetSeconds () - m_qDelayOld.GetSeconds ());
		if (m_dropProb < 0.001)
		{
			p /= 32;
		}
		else if (m_dropProb < 0.01)
		{
			p /= 8;
		}
		else if (m_dropProb < 0.1)
		{
			p /= 2;
		}
		else if (m_dropProb < 1)
		{
			p /= 0.5;
		}
		else if (m_dropProb < 10)
		{
			p /= 0.125;
		}
		else
		{
			p /= 0.03125;
		}
		if ((m_dropProb >= 0.1) && (p > 0.02))
		{
			p = 0.02;
		}
	}

	p += m_dropProb;

	// For non-linear drop in prob

	if (qDelay.GetSeconds () == 0 && m_qDelayOld.GetSeconds () == 0)
	{
		p *= 0.98;
	}
	else if (qDelay.GetSeconds () > 0.2)
	{
		p += 0.02;
	}

	m_dropProb = (p > 0) ? p : 0;
	if (m_burstAllowance < m_tUpdate)
	{
		m_burstAllowance =  Time (Seconds (0));
	}
	else
	{
		m_burstAllowance -= m_tUpdate;
	}

	uint32_t burstResetLimit = BURST_RESET_TIMEOUT / m_tUpdate.GetSeconds ();
	if ( (qDelay.GetSeconds () < 0.5 * m_qDelayRef.GetSeconds ()) && (m_qDelayOld.GetSeconds () < (0.5 * m_qDelayRef.GetSeconds ())) && (m_dropProb == 0) && !missingInitFlag )
	{
		m_dqCount = -1;
		m_avgDqRate = 0.0;
	}
	if ( (qDelay.GetSeconds () < 0.5 * m_qDelayRef.GetSeconds ()) && (m_qDelayOld.GetSeconds () < (0.5 * m_qDelayRef.GetSeconds ())) && (m_dropProb == 0) && (m_burstAllowance.GetSeconds () == 0))
	{
		if (m_burstState == IN_BURST_PROTECTING)
		{
			m_burstState = IN_BURST;
			m_burstReset = 0;
		}
		else if (m_burstState == IN_BURST)
		{
			m_burstReset++;
			if (m_burstReset > burstResetLimit)
			{
				m_burstReset = 0;
				m_burstState = NO_BURST;
			}
		}
	}
	else if (m_burstState == IN_BURST)
	{
		m_burstReset = 0;
	}

	m_qDelayOld = qDelay;
	m_rtrsEvent = Simulator::Schedule (m_tUpdate, &PvPieQueueDisc::CalculateP, this);
}

Ptr<QueueDiscItem> PvPieQueueDisc::DoDequeue ()
{
	NS_LOG_FUNCTION (this);

	if (GetInternalQueue (0)->IsEmpty ())
	{
		NS_LOG_LOGIC ("Queue empty");
		return 0;
	}

	Ptr<QueueDiscItem> item = GetInternalQueue (0)->Dequeue ();
	double now = Simulator::Now ().GetSeconds ();
	uint32_t pktSize = item->GetSize ();

	// if not in a measurement cycle and the queue has built up to dq_threshold,
	// start the measurement cycle

	if ( (GetInternalQueue (0)->GetNBytes () >= m_dqThreshold) && (!m_inMeasurement) )
	{
		m_dqStart = now;
		m_dqCount = 0;
		m_inMeasurement = true;
	}

	if (m_inMeasurement)
	{
		m_dqCount += pktSize;

		// done with a measurement cycle
		if (m_dqCount >= m_dqThreshold)
		{

			double tmp = now - m_dqStart;

			if (tmp > 0)
			{
				if (m_avgDqRate == 0)
				{
					m_avgDqRate = m_dqCount / tmp;
				}
				else
				{
					m_avgDqRate = (0.5 * m_avgDqRate) + (0.5 * (m_dqCount / tmp));
				}
			}

			// restart a measurement cycle if there is enough data
			if (GetInternalQueue (0)->GetNBytes () > m_dqThreshold)
			{
				m_dqStart = now;
				m_dqCount = 0;
				m_inMeasurement = true;
			}
			else
			{
				m_dqCount = 0;
				m_inMeasurement = false;
			}
		}
	}

	return item;
}

Ptr<const QueueDiscItem> PvPieQueueDisc::DoPeek () const
{
	NS_LOG_FUNCTION (this);
	if (GetInternalQueue (0)->IsEmpty ())
	{
		NS_LOG_LOGIC ("Queue empty");
		return 0;
	}

	Ptr<const QueueDiscItem> item = GetInternalQueue (0)->Peek ();

	NS_LOG_LOGIC ("Number packets " << GetInternalQueue (0)->GetNPackets ());
	NS_LOG_LOGIC ("Number bytes " << GetInternalQueue (0)->GetNBytes ());

	return item;
}

bool PvPieQueueDisc::CheckConfig (void)
{
	NS_LOG_FUNCTION (this);
	if (GetNQueueDiscClasses () > 0)
	{
		NS_LOG_ERROR ("PvPieQueueDisc cannot have classes");
		return false;
	}

	if (GetNPacketFilters () > 0)
	{
		NS_LOG_ERROR ("PvPieQueueDisc cannot have packet filters");
		return false;
	}

	if (GetNInternalQueues () == 0)
	{
		// create a DropTail queue
		Ptr<InternalQueue> queue = CreateObjectWithAttributes<DropTailQueue<QueueDiscItem> > ("Mode", EnumValue (m_mode));
		if (m_mode == QUEUE_DISC_MODE_PACKETS)
		{
			queue->SetMaxPackets (m_queueLimit);
		}
		else
		{
			queue->SetMaxBytes (m_queueLimit);
		}
		AddInternalQueue (queue);
	}

	if (GetNInternalQueues () != 1)
	{
		NS_LOG_ERROR ("PvPieQueueDisc needs 1 internal queue");
		return false;
	}

	if ((GetInternalQueue (0)->GetMode () == QueueBase::QUEUE_MODE_PACKETS && m_mode == QUEUE_DISC_MODE_BYTES) ||
		(GetInternalQueue (0)->GetMode () == QueueBase::QUEUE_MODE_BYTES && m_mode == QUEUE_DISC_MODE_PACKETS))
	{
		NS_LOG_ERROR ("The mode of the provided queue does not match the mode set on the PvPieQueueDisc");
		return false;
	}

	if ((m_mode ==  QUEUE_DISC_MODE_PACKETS && GetInternalQueue (0)->GetMaxPackets () != m_queueLimit)
		|| (m_mode ==  QUEUE_DISC_MODE_BYTES && GetInternalQueue (0)->GetMaxBytes () != m_queueLimit))
	{
		NS_LOG_ERROR ("The size of the internal queue differs from the queue disc limit");
		return false;
	}

	return true;
}

} // namespace ns3

