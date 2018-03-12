/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

/*
Quantize data: b[i] = 10^(1+1/30*i) kbps , i e [0, 149] [10 kbps, 1Gbps]
			   V[i] = V(b[i])

Token buckets:
		l_max[i] = b[i]*d (averaging delay)
		l[i] = l_max[i] <- intialized
		filled continuously with b[i] speed


		DoEnqueue:
			packet size: s bytes

			set packet value to V[k]

			min([1,149] | l[i] >= s)
			tokenbuckets: remove tokens l[j] = l[j] - s if j > k

			PV: V[k] -> k =
*/

#ifndef TOKENBUCKET_H
#define TOKENBUCKET_H

#include "ns3/object.h"

namespace ns3 {

class TokenBucket : public Object {
	public:
		static TypeId GetTypeId (void);
		TokenBucket();
		virtual ~TokenBucket();

	protected:
		virtual void DoDispose (void);

	public:
		void RemoveTokens(int);
		int32_t GetSize(void);

	private:
		void GenerateTokens();

		/* user variables */
		uint32_t m_maxSize;

		/* local variables */
		uint32_t m_tokenGenerationRate;
		int32_t m_size;


};

}

#endif /* TOKENBUCKET_H */

