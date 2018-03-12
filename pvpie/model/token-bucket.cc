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
#include "token-bucket.h"

namespace ns3 {


NS_LOG_COMPONENT_DEFINE("TokenBucket");

NS_OBJECT_ENSURE_REGISTERED(TokenBucket);

TypeId TokenBucket::GetTypeId (void)
{
static TypeId tid = TypeId ("ns3::TokenBucket")
	.SetParent<Object>()
	.SetGroupName ("pvpie")
	.AddConstructor<TokenBucket>()
	;

	return tid;
}

TokenBucket::TokenBucket ()
{
	NS_LOG_FUNCTION (this);

}

TokenBucket::~TokenBucket()
{
	NS_LOG_FUNCTION (this);
}

void TokenBucket::DoDispose(void)
{
	NS_LOG_FUNCTION (this);
	Object::DoDispose ();
}


int32_t TokenBucket::GetSize(void)
{
	return m_size;
}

void TokenBucket::GenerateTokens()
{


}

void TokenBucket::RemoveTokens(int count)
{
	m_size -= count;
}

} /* namespace ns3 */