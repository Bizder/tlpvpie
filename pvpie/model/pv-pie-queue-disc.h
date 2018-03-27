/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef PVPIE_H
#define PVPIE_H

#include "ns3/queue-disc.h"
#include "ns3/nstime.h"
#include "ns3/boolean.h"
#include "ns3/data-rate.h"
#include "ns3/timer.h"
#include "ns3/event-id.h"
#include "ecdf.h"

#define BURST_RESET_TIMEOUT 1.5

namespace ns3 {

class PvPieQueueDisc : public QueueDisc {
	public:
		static TypeId GetTypeId(void);
		PvPieQueueDisc (void);
		virtual ~PvPieQueueDisc(void);

		enum BurstStateT
		{
			NO_BURST,
			IN_BURST,
			IN_BURST_PROTECTING,
		};

		uint32_t GetQueueSize(void);
		void SetQueueLimit (uint32_t lim);

		Time GetQueueDelay(void);

		static constexpr const char* UNFORCED_DROP = "Unforced drop";  //!< Early probability drops: proactive
		static constexpr const char* FORCED_DROP = "Forced drop";      //!< Drops due to queue limit: reactive

	protected:
		virtual void DoDispose(void);

	private:
		virtual bool DoEnqueue (Ptr<QueueDiscItem> item);
		virtual Ptr<QueueDiscItem> DoDequeue(void);
		virtual Ptr<const QueueDiscItem> DoPeek(void) const;
		virtual bool CheckConfig(void);

		virtual void InitializeParams(void);

		/**
		* \brief Check if a packet needs to be dropped due to probability drop
		* \param item queue item
		* \param qSize queue size
		* \returns 0 for no drop, 1 for drop
		*/
		bool DropEarly(Ptr<QueueDiscItem> item, uint32_t qSize, uint32_t packetValue);

		void CalculateP(void);

		// ** Variables supplied by user
		uint32_t m_queueLimit;                        //!< Queue limit in bytes
		Time m_tUpdate;                               //!< Time period after which CalculateP() is called
		Time m_qDelayRef;                             //!< Desired queue delay
		uint32_t m_meanPktSize;                       //!< Average packet size in bytes
		Time m_maxBurst;                              //!< Maximum burst allowed before random early dropping kicks in
		double m_a;                                   //!< Parameter to pie controller
		double m_b;                                   //!< Parameter to pie controller
		uint32_t m_dqThreshold;                       //!< Minimum queue size in bytes before dequeue rate is measured

		// ** Variables maintained by PIE
		TracedValue<double> m_thresholdValue;         //!< Filter value - V
		Time m_qDelayOld;                             //!< Old value of queue delay
		TracedValue<Time> m_qDelay;                   //!< Current value of queue delay
		Time m_burstAllowance;                        //!< Current max burst value in seconds that is allowed before random drops kick in
		uint32_t m_burstReset;                        //!< Used to reset value of burst allowance
		BurstStateT m_burstState;                     //!< Used to determine the current state of burst
		bool m_inMeasurement;                         //!< Indicates whether we are in a measurement cycle
		double m_avgDqRate;                           //!< Time averaged dequeue rate
		double m_dqStart;                             //!< Start timestamp of current measurement cycle
		uint32_t m_dqCount;                           //!< Number of bytes departed since current measurement cycle starts
		EventId m_rtrsEvent;                          //!< Event used to decide the decision of interval of drop probability calculation
		eCDF m_ecdf;                                  //!< eCDF function used to determine packet value

};

}

#endif /* PVPIE_H */

