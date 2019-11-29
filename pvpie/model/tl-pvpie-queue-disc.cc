/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2016 NITK Surathkal
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Shravya Ks <shravya.ks0@gmail.com>
 *          Smriti Murali <m.smriti.95@gmail.com>
 *          Mohit P. Tahiliani <tahiliani@nitk.edu.in>
 */

/*
 * PORT NOTE: This code was ported from ns-2.36rc1 (queue/pie.cc).
 * Most of the comments are also ported from the same.
 */

#include "ns3/log.h"
#include "ns3/enum.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/simulator.h"
#include "ns3/abort.h"
#include "tl-pvpie-queue-disc.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/net-device-queue-interface.h"
#include "packet-value-tag.h"
#include <algorithm>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("TlPieQueueDisc");

NS_OBJECT_ENSURE_REGISTERED(TlPieQueueDisc);

TypeId TlPieQueueDisc::GetTypeId(void)
{
  static TypeId tid = TypeId ("ns3::TlPieQueueDisc")
  .SetParent<QueueDisc>()
  .SetGroupName("TlPie")
    .AddConstructor<TlPieQueueDisc> ()
    .AddAttribute ("MeanPktSize",
                   "Average of packet size",
                   UintegerValue (1000),
                   MakeUintegerAccessor (&TlPieQueueDisc::m_meanPktSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("A",
                   "Value of alpha",
                   DoubleValue (0.125),
                   MakeDoubleAccessor (&TlPieQueueDisc::m_a),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("B",
                   "Value of beta",
                   DoubleValue (1.25),
                   MakeDoubleAccessor (&TlPieQueueDisc::m_b),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("F",
                   "Value of phi",
                   DoubleValue (0.125),
                   MakeDoubleAccessor (&TlPieQueueDisc::m_f),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("P",
                   "Value of psi",
                   DoubleValue (1.25),
                   MakeDoubleAccessor (&TlPieQueueDisc::m_p),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Tupdate1",
                   "Time period to calculate drop probability",
                   TimeValue (Seconds (0.03)),
                   MakeTimeAccessor (&TlPieQueueDisc::m_tUpdate1),
                   MakeTimeChecker ())
    .AddAttribute ("Supdate1",
                   "Start time of the probability update timer",
                   TimeValue (Seconds (0)),
                   MakeTimeAccessor (&TlPieQueueDisc::m_sUpdate1),
                   MakeTimeChecker ())
    .AddAttribute ("Tupdate2",
                   "Time period to calculate threshold value",
                   TimeValue (Seconds (0.01)),
                   MakeTimeAccessor (&TlPieQueueDisc::m_tUpdate2),
                   MakeTimeChecker())
    .AddAttribute ("Supdate2",
                   "Start time of the value update timer",
                   TimeValue (Seconds (0)),
                   MakeTimeAccessor (&TlPieQueueDisc::m_sUpdate2),
                   MakeTimeChecker ())
    .AddAttribute ("MaxSize",
                   "The maximum number of packets accepted by this queue disc",
                   QueueSizeValue (QueueSize ("25p")),
                   MakeQueueSizeAccessor (&QueueDisc::SetMaxSize,
                                          &QueueDisc::GetMaxSize),
                   MakeQueueSizeChecker ())
    .AddAttribute ("DequeueThreshold",
                   "Minimum queue size in bytes before dequeue rate is measured",
                   UintegerValue (10000),
                   MakeUintegerAccessor (&TlPieQueueDisc::m_dqThreshold),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("QueueDelayReference",
                   "Desired queue delay",
                   TimeValue (Seconds (0.02)),
                   MakeTimeAccessor (&TlPieQueueDisc::m_qDelayRef),
                   MakeTimeChecker ())
    .AddAttribute ("MaxBurstAllowance",
                   "Current max burst allowance in seconds before random drop",
                   TimeValue (Seconds (0.1)),
                   MakeTimeAccessor (&TlPieQueueDisc::m_maxBurst),
                   MakeTimeChecker ())
    .AddAttribute("DropDelta",
                   "Droprate measurement time limit",
                   DoubleValue(1.0),
                   MakeDoubleAccessor (&TlPieQueueDisc::m_drop_timedelta),
                   MakeDoubleChecker<double> ())
    .AddAttribute("VMin",
                   "Droprate measurement time limit",
                   IntegerValue(0),
                   MakeIntegerAccessor(&TlPieQueueDisc::m_vmin),
                   MakeIntegerChecker<int32_t> ())
    .AddAttribute("VMax",
                   "Droprate measurement time limit",
                   IntegerValue(4294967295),
                   MakeIntegerAccessor(&TlPieQueueDisc::m_vmax),
                   MakeIntegerChecker<int32_t> ())
    .AddTraceSource("QueueingDelay",
                  "Queueing Delay",
                  MakeTraceSourceAccessor(&TlPieQueueDisc::m_qDelay),
                  "ns3::Time::TracedValueCallback")
    .AddTraceSource("Probability",
                  "Probability of packet droping",
                  MakeTraceSourceAccessor(&TlPieQueueDisc::m_dropProb),
                  "ns3::TracedValueCallback::Double")
    .AddTraceSource("ThresholdValue",
                  "Threshold Value of packet droping",
                  MakeTraceSourceAccessor(&TlPieQueueDisc::m_thresholdValue),
                  "ns3::TracedValueCallback::UInteger")
  ;

  return tid;
}

TlPieQueueDisc::TlPieQueueDisc ()
  : QueueDisc ()
{
  NS_LOG_FUNCTION (this);
  m_uv = CreateObject<UniformRandomVariable> ();
  m_rtrsEvent_p = Simulator::Schedule (m_sUpdate1, &TlPieQueueDisc::CalculateP, this);
  m_rtrsEvent_v = Simulator::Schedule (m_sUpdate2, &TlPieQueueDisc::CalculateV, this);
}

TlPieQueueDisc::~TlPieQueueDisc ()
{
  NS_LOG_FUNCTION (this);
}

void
TlPieQueueDisc::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_uv = 0;
  Simulator::Remove (m_rtrsEvent_p);
  Simulator::Remove (m_rtrsEvent_v);
  QueueDisc::DoDispose ();
}

Time
TlPieQueueDisc::GetQueueDelay (void)
{
  NS_LOG_FUNCTION (this);
  return m_qDelay;
}

int64_t
TlPieQueueDisc::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_uv->SetStream (stream);
  return 1;
}

bool
TlPieQueueDisc::DoEnqueue (Ptr<QueueDiscItem> item)
{
  NS_LOG_FUNCTION (this << item);

  QueueSize nQueued = GetCurrentSize();

  if (nQueued >= GetMaxSize())
    {
      // Drops due to queue limit: reactive
      DropBeforeEnqueue (item, FORCED_DROP);
      AddDrop(Simulator::Now().GetSeconds(), true);
      return false;
    }
  else if (DropEarly (item, nQueued.GetValue()))
    {
      // Early probability drop: proactive
      DropBeforeEnqueue (item, UNFORCED_DROP);
      AddDrop(Simulator::Now().GetSeconds(), true);
      return false;
    }

  // No drop
  AddDrop(Simulator::Now().GetSeconds(), false);
  bool retval = GetInternalQueue (0)->Enqueue (item);

  // If Queue::Enqueue fails, QueueDisc::DropBeforeEnqueue is called by the
  // internal queue because QueueDisc::AddInternalQueue sets the trace callback

  NS_LOG_LOGIC ("\t bytesInQueue  " << GetInternalQueue (0)->GetNBytes ());
  NS_LOG_LOGIC ("\t packetsInQueue  " << GetInternalQueue (0)->GetNPackets ());

  return retval;
}

void
TlPieQueueDisc::InitializeParams (void)
{
  // Initially queue is empty so variables are initialize to zero except m_dqCount
  m_inMeasurement = false;
  m_dqCount = DQCOUNT_INVALID;
  m_dropProb = 0;
  m_avgDqRate = 0.0;
  m_dqStart = 0;
  m_burstState = NO_BURST;
  m_qDelayOld = Time (Seconds (0));
  m_dropProbOld = 0;
}

bool TlPieQueueDisc::DropEarly (Ptr<QueueDiscItem> item, uint32_t qSize)
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

  if ((m_qDelayOld.GetSeconds () < (0.5 * m_qDelayRef.GetSeconds ())) && (m_dropProb < 0.2))
    {
      return false;
    }
  else if (GetMaxSize ().GetUnit () == QueueSizeUnit::BYTES && qSize <= 2 * m_meanPktSize)
    {
      return false;
    }
  else if (GetMaxSize ().GetUnit () == QueueSizeUnit::PACKETS && qSize <= 2)
    {
      return false;
    }


  /* change this! */
  PacketValueTag tag;
  item->GetPacket()->PeekPacketTag(tag);

  return tag.GetPacketValue() < m_thresholdValue;
}

void TlPieQueueDisc::CalculateP()
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

  p = (p > 0) ? p : 0;
  p = (1 > p) ? p : 1;

  m_dropProb = p;

  if (m_burstAllowance < m_tUpdate1)
    {
      m_burstAllowance =  Time (Seconds (0));
    }
  else
    {
      m_burstAllowance -= m_tUpdate1;
    }

  uint32_t burstResetLimit = BURST_RESET_TIMEOUT / m_tUpdate1.GetSeconds ();
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

  m_rtrsEvent_p = Simulator::Schedule (m_tUpdate1, &TlPieQueueDisc::CalculateP, this);

}

void TlPieQueueDisc::CalculateV()
{
  NS_LOG_FUNCTION (this);

  // m_dropProb
  double m_dropProbCurr = GetRealDropRate();

  double m_pDelta = m_dropProb - m_dropProbCurr;
  double m_pDeltaCurr = m_dropProbOld - m_dropProbCurr;
  int32_t m_thresholdValue_new = m_thresholdValue + m_f * m_pDelta * (m_vmax - m_vmin) + m_p * m_pDeltaCurr * (m_vmax - m_vmin);

  if ( m_thresholdValue_new > m_vmax )
  {
    m_thresholdValue = m_vmax;
  }
  else if ( m_thresholdValue_new < m_vmin )
  {
    m_thresholdValue = m_vmin;
  }
  else
  {
    m_thresholdValue = m_thresholdValue_new;
  }

  m_dropProbOld = m_dropProbCurr;

  if (m_dropProb == 0.0)
  {
    m_thresholdValue = 0;
  }

  m_rtrsEvent_v = Simulator::Schedule (m_tUpdate2, &TlPieQueueDisc::CalculateV, this);
}


Ptr<QueueDiscItem>
TlPieQueueDisc::DoDequeue ()
{
  NS_LOG_FUNCTION (this);

  if (GetInternalQueue (0)->IsEmpty ())
    {
      NS_LOG_LOGIC ("Queue empty");
      return 0;
    }

  Ptr<QueueDiscItem> item = GetInternalQueue (0)->Dequeue ();
  double now = Simulator::Now().GetSeconds ();
  uint32_t pktSize = item->GetSize();

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

bool
TlPieQueueDisc::CheckConfig (void)
{
  NS_LOG_FUNCTION (this);
  if (GetNQueueDiscClasses () > 0)
    {
      NS_LOG_ERROR ("TlPieQueueDisc cannot have classes");
      return false;
    }

  if (GetNPacketFilters () > 0)
    {
      NS_LOG_ERROR ("TlPieQueueDisc cannot have packet filters");
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
      NS_LOG_ERROR ("TlPieQueueDisc needs 1 internal queue");
      return false;
    }

  return true;
}


double TlPieQueueDisc::GetRealDropRate()
{
  RemoveOldValues();

  if ( m_droprecord.size() == 0)
    return 0;

  uint32_t dropCount = 0;
  for ( uint32_t i = 0; i < m_droprecord.size() ; ++i )
  {
    if ( m_droprecord[i].dropped )
    {
      dropCount++;
    }
  }
  return double(dropCount) / double(m_droprecord.size());
}

void TlPieQueueDisc::RemoveOldValues()
{
  double now = Simulator::Now().GetSeconds();
  double start = now - m_drop_timedelta;

  int threshold_index = 0;

  for (uint16_t i = 0; i < m_droprecord.size() ; ++i ) {
    if ( start > m_droprecord[i].receive_time)
    {
      threshold_index = i;
    }
  }

  m_droprecord.erase(m_droprecord.begin(),m_droprecord.begin()+threshold_index);
}

void TlPieQueueDisc::AddDrop(double time, bool dropped)
{
  m_droprecord.push_back(DropRecord(time, dropped));
}


} //namespace ns3
