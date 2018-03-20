/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef MARKER_H
#define MARKER_H

#include <vector>
#include "ns3/queue-disc.h"
#include "ns3/ipv4-queue-disc-item.h"
#include "token-bucket.h"

namespace ns3 {

class PacketMarkerQueueDisc : public QueueDisc {
	public:
		static TypeId GetTypeId (void);
		PacketMarkerQueueDisc();
		virtual ~PacketMarkerQueueDisc();

		enum QueueDiscMode
		{
			QUEUE_DISC_MODE_PACKETS,
			QUEUE_DISC_MODE_BYTES,
		};

	protected:
		virtual void DoDispose (void);

	private:
		virtual uint16_t throughputValueFunction(uint32_t);

		virtual bool DoEnqueue (Ptr<QueueDiscItem> item);

		virtual Ptr<QueueDiscItem> DoDequeue (void);
		virtual Ptr<const QueueDiscItem> DoPeek (void) const;

		virtual bool CheckConfig (void);
		virtual void InitializeParams (void);

		// ** Variables maintained by Marker
		std::vector<TokenBucket> m_tokenBuckets;
		int32_t m_granularity;

};

class GoldPacketMarkerQueueDisc : public PacketMarkerQueueDisc {
	public:
		static TypeId GetTypeId (void);
		GoldPacketMarkerQueueDisc() : PacketMarkerQueueDisc() {};
		virtual ~GoldPacketMarkerQueueDisc() {};

	private:
		uint16_t throughputValueFunction(uint32_t);
};

class SilverPacketMarkerQueueDisc : public PacketMarkerQueueDisc {
	public:
		static TypeId GetTypeId (void);
		SilverPacketMarkerQueueDisc() : PacketMarkerQueueDisc() {};
		virtual ~SilverPacketMarkerQueueDisc() {};

	private:
		uint16_t throughputValueFunction(uint32_t);

};



}

#endif /* MARKER_H */

