/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef MARKER_H
#define MARKER_H

#include "ns3/queue-disc.h"
#include "ns3/random-variable-stream.h"
#include "ns3/timer.h"

namespace ns3 {

class PacketMarkerQueueDisc : public QueueDisc {
	public:
		static TypeId GetTypeId (void);
		PacketMarkerQueueDisc();
		virtual ~PacketMarkerQueueDisc();

		int64_t AssignStreams (int64_t stream);

	protected:
		virtual void DoDispose (void);

	private:
		virtual uint32_t throughputValueFunction(uint32_t);
		virtual uint8_t getPriority();

		virtual bool DoEnqueue (Ptr<QueueDiscItem> item);

		virtual Ptr<QueueDiscItem> DoDequeue (void);
		virtual Ptr<const QueueDiscItem> DoPeek (void) const;

		virtual bool CheckConfig (void);
		virtual void InitializeParams (void);

		// ** Variables defined by user
		double m_a;                                   //!< Value of alpha in EWMA calculation

		// ** Variables maintained by Marker
		TracedValue<uint32_t> m_transferRate;         //!< Transfer Rate calculated by EWMA (exponentioally waited moving average)
		Time m_lastSend;                              //!< Last time a packet was enqueued
		Ptr<UniformRandomVariable> m_uv;              //!< Rng stream

		TracedValue<uint32_t> m_lastpv;        //!< value of last PV
};

class GoldPacketMarkerQueueDisc : public PacketMarkerQueueDisc {
	public:
		static TypeId GetTypeId (void);
		GoldPacketMarkerQueueDisc() : PacketMarkerQueueDisc() {};
		virtual ~GoldPacketMarkerQueueDisc() {};

	private:
		uint32_t throughputValueFunction(uint32_t);
		uint8_t getPriority();
};

class SilverPacketMarkerQueueDisc : public PacketMarkerQueueDisc {
	public:
		static TypeId GetTypeId (void);
		SilverPacketMarkerQueueDisc() : PacketMarkerQueueDisc() {};
		virtual ~SilverPacketMarkerQueueDisc() {};

	private:
		uint32_t throughputValueFunction(uint32_t);
		uint8_t getPriority();
};

class BackgroundPacketMarkerQueueDisc : public PacketMarkerQueueDisc {
	public:
		static TypeId GetTypeId (void);
		BackgroundPacketMarkerQueueDisc() : PacketMarkerQueueDisc() {};
		virtual ~BackgroundPacketMarkerQueueDisc() {};

	private:
		uint32_t throughputValueFunction(uint32_t);
		uint8_t getPriority();
};


}

#endif /* MARKER_H */

