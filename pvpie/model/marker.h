/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef MARKER_H
#define MARKER_H

namespace ns3 {

class PacketMarkerQueueDisc : public QueueDisc {
	public:
		static TypeId GetTypeId (void);
		PacketMarkerQueueDisc();
		virtual ~PacketMarkerQueueDisc();

	protected:
		virtual void DoDispose (void);

	private:
		virtual bool DoEnqueue (Ptr<QueueDiscItem> item);

		virtual bool CheckConfig (void);
		virtual void InitializeParams (void);

}

#endif /* MARKER_H */

