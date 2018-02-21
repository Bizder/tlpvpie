/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef VALUEDAPP_H
#define VALUEDAPP_H

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"

namespace ns3 {

class ValuedApp : public Application
{
	public:
		ValuedApp();
		virtual ~ValuedApp();

		void Setup(Ptr<Socket> socket, Address address, uint32_t packetSize, uint8_t packetValue);

	private:
		virtual void StartApplication(void);
		virtual void StopApplication(void);

		void SendData();

		Ptr<Socket> m_socket;
		Address m_peer;
		bool m_connected;
		uint32_t m_packetSize;
		uint8_t m_packetValue;

		void ConnectionSucceeded(Ptr<Socket> socket);
		void ConnectionFailed(Ptr<Socket> socket);
		void DataSend(Ptr<Socket>, uint32_t);
		void Ignore(Ptr<Socket> socket);
};

}

#endif /* VALUEDAPP_H */

