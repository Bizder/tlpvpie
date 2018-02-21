/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "valuedapp.h"

namespace ns3 {

ValuedApp::ValuedApp(): m_socket(0), m_connected(false), m_packetSize(512), m_packetValue(0)
{
}

ValuedApp::~ValuedApp()
{
	m_socket = 0;
}

void ValuedApp::Setup(ns3::Ptr<ns3::Socket> socket, ns3::Address address, uint32_t packetSize, uint8_t packetValue)
{
	m_socket = socket;
	m_peer = address;
	m_packetSize = packetSize;
	m_packetValue = packetValue;

	// Fatal error if socket type is not NS3_SOCK_STREAM or NS3_SOCK_SEQPACKET
	if (m_socket->GetSocketType() != ns3::Socket::NS3_SOCK_STREAM &&
		m_socket->GetSocketType() != ns3::Socket::NS3_SOCK_SEQPACKET)
	{
		NS_FATAL_ERROR
			("Using ValuedApp with an incompatible socket type. "
			 "ValuedApp requires SOCK_STREAM or SOCK_SEQPACKET. "
			 "In other words, use TCP instead of UDP.");
	}
}

void ValuedApp::StartApplication()
{
	m_socket->Bind();
	m_socket->Connect(m_peer);
	m_socket->SetIpTos(m_packetValue);
	m_socket->ShutdownRecv();
	m_socket->SetConnectCallback(ns3::MakeCallback(&ValuedApp::ConnectionSucceeded, this), ns3::MakeCallback(&ValuedApp::ConnectionFailed, this));
	m_socket->SetSendCallback(ns3::MakeCallback(&ValuedApp::DataSend, this));
	if (m_connected)
	{
		SendData();
	}
}

void ValuedApp::StopApplication()
{
	if (m_socket != 0) {
		m_socket->Close();
		m_connected = false;
	}
}

void ValuedApp::SendData(void)
{
	// We exit this loop when actual<toSend as the send side
	// buffer is full. The "DataSend" callback will pop when
	// some buffer space has freed ip.
	for (;;) {
		ns3::Ptr<ns3::Packet> packet = ns3::Create<ns3::Packet>(m_packetSize);
		int actual = m_socket->Send(packet);
		if ((unsigned) actual != m_packetSize)
		{
			break;
		}
	}
}

void ValuedApp::ConnectionSucceeded(ns3::Ptr<ns3::Socket> socket)
{
	m_connected = true;
	SendData();
}

void ValuedApp::ConnectionFailed(ns3::Ptr<ns3::Socket> socket)
{
}

void ValuedApp::DataSend(ns3::Ptr<ns3::Socket>, uint32_t)
{
	if (m_connected) {
		ns3::Simulator::ScheduleNow(&ValuedApp::SendData, this);
	}
}

}
