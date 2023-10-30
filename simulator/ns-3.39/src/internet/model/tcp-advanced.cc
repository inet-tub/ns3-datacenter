/* Modification */
/*
 * tcp-advanced.cc
 *
 *  Created on: Jan 9, 2021
 *      Author: vamsi
 */

#include "tcp-socket-state.h"
#include "tcp-advanced.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("TcpAdvanced");
NS_OBJECT_ENSURE_REGISTERED (TcpAdvanced);

TypeId
TcpAdvanced::GetTypeId (void)
{
	static TypeId tid = TypeId ("ns3::TcpAdvanced")
	                    .SetParent<TcpNewReno> ()
	                    .AddConstructor<TcpAdvanced> ()
	                    .SetGroupName ("Internet")
	                    .AddAttribute ("Alpha", "Lower bound of packets in network",
	                                   UintegerValue (2),
	                                   MakeUintegerAccessor (&TcpAdvanced::m_alpha),
	                                   MakeUintegerChecker<uint32_t> ())
	                    .AddAttribute ("Beta", "Upper bound of packets in network",
	                                   UintegerValue (4),
	                                   MakeUintegerAccessor (&TcpAdvanced::m_beta),
	                                   MakeUintegerChecker<uint32_t> ())
	                    .AddAttribute ("Gamma", "Limit on increase",
	                                   UintegerValue (1),
	                                   MakeUintegerAccessor (&TcpAdvanced::m_gamma),
	                                   MakeUintegerChecker<uint32_t> ())
	                    ;
	return tid;
}

TcpAdvanced::TcpAdvanced (void)
	: TcpNewReno (),
	  m_baseRtt (MicroSeconds(60)),
	  m_minRtt (MicroSeconds(60))
{
	setDc();
	NS_LOG_FUNCTION (this);
}

TcpAdvanced::TcpAdvanced (const TcpAdvanced& sock)
	: TcpNewReno (sock),
	  m_baseRtt (sock.m_baseRtt),
	  m_minRtt (sock.m_minRtt)
{
	setDc();
	NS_LOG_FUNCTION (this);
}

TcpAdvanced::~TcpAdvanced (void)
{
	NS_LOG_FUNCTION (this);
}

void
TcpAdvanced::Init(Ptr<TcpSocketState> tcb) {
	NS_LOG_FUNCTION (this << tcb);
	NS_LOG_INFO (this << "TcpAdvanced");

	hpcc = tcb->useHpcc;
	if (!tcb->useTimely) {
		tcb->m_initialCWnd = (tcb->maxCCRate.GetBitRate() * tcb->CCInitRtt.GetSeconds()) / (8 * tcb->m_segmentSize);
	}
	else {
		tcb->m_initialCWnd = (UINT32_MAX - 1e3);
	}
	tcb->CCRate = tcb->maxCCRate;

	m_baseRtt = tcb->CCInitRtt;

	if (!tcb->useTimely) {
		tcb->m_setCCRateCallback(tcb, tcb->CCRate, tcb->CCRate, m_baseRtt, true);
	}
	else {
		tcb->m_setCCRateCallback(tcb, tcb->CCRate, tcb->CCRate, m_baseRtt, false);
	}

	// the below are used only by HPCC if enabled
	CCRateAgg = tcb->CCRate;
	for (int i = 0; i < 10; i++)
		CCRateHop[i] = tcb->CCRate;

}

Ptr<TcpCongestionOps>
TcpAdvanced::Fork (void)
{
	return CopyObject<TcpAdvanced> (this);
}

void
TcpAdvanced::SetCCRate(Ptr<TcpSocketState> tcb, DataRate rate, DataRate prevRate) {

	tcb->CCRate = rate;
	if (CC_pacingTimer.IsRunning()) {

		Time left = CC_pacingTimer.GetDelayLeft();

		Time prevRateTime = prevRate.CalculateBytesTxTime(tcb->m_segmentSize);
//		Time prevSched = Simulator::Now() - (prevRateTime - left);

		CC_pacingTimer.Suspend();

		Time toSend = tcb->CCRate.CalculateBytesTxTime(tcb->m_segmentSize);

		CC_pacingTimer.Schedule(toSend + left - prevRateTime);
	}
}

uint64_t GetTimeDelta(uint64_t a, uint64_t b) {
	if (a > b) {
		return a - b;
	}
	else {
		return b - a;
	}
}

uint64_t GetBytesDelta(uint64_t a, uint64_t b) {
	if (a > b) {
		return a - b;
	}
	else
		return b - a;
}

double GetUDelta (double a, double b) {
	if (a > b)
		return a - b;
	if (b > a)
		return b - a;
}

void TcpAdvanced::ProcessDcAck(Ptr<Packet> packet, const TcpHeader& tcpHeader, Ptr<TcpSocketState> tcb) {

	bool found;
	FeedbackTag fb;
	found = packet->PeekPacketTag(fb);

	uint32_t ackNum = tcpHeader.GetAckNumber().GetValue();

	if (found) {
		if (tcb->useHpcc) {
			if (ackNum > lastUpdatedSeq) {

				UpdateRateHpcc(packet, tcpHeader, tcb, fb, false);
			} else {
				FastReactHpcc(packet, tcpHeader, tcb, fb);
			}
		}
		else if (tcb->useTimely) {
			// std::cout << tcb->m_cWnd.Get() << std::endl;
			if (ackNum > lastUpdatedSeq) {

				UpdateRateTimely(packet, tcpHeader, tcb, fb, false);
			} else {
				FastReactTimely(packet, tcpHeader, tcb, fb);
			}
		}
		else if (tcb->useThetaPower) {
			if (ackNum > lastUpdatedSeq) {

				UpdateRateThetaPowertcp(packet, tcpHeader, tcb, fb, false);
			} else {
				FastReactThetaPowertcp(packet, tcpHeader, tcb, fb);
			}
		}
		else if (tcb->usePower) {
			if (ackNum > lastUpdatedSeq) {
				UpdateRatePowertcp(packet, tcpHeader, tcb, fb, false);
			} else {
				FastReactPowertcp(packet, tcpHeader, tcb, fb);
			}
		}
	}
}



void TcpAdvanced::UpdateRatePowertcp(Ptr<Packet> packet, const TcpHeader& tcpHeader, Ptr<TcpSocketState> tcb, FeedbackTag fb, bool fast_react) {
	uint32_t next_seq = tcb->m_nextTxSequence.Get().GetValue();
	uint32_t ackNum = tcpHeader.GetAckNumber().GetValue();


	if (lastUpdatedSeq == 0) {
		lastUpdatedSeq = next_seq;
		lastAckedSeq = ackNum;

		NS_ASSERT(fb.getHopCount() <= fb.getMaxHops());
		FbPrev = fb;
	} else {

		if (fb.getHopCount() <= fb.getMaxHops()) {
			double max_c = 0;
			bool inStable = false;

			double U = 0;
			uint64_t dt = 0;
			bool updated[fb.getMaxHops()] = {false}, updated_any = false;

			NS_ASSERT(fb.getHopCount() <= fb.getMaxHops());

			for (uint32_t i = 0; i < fb.getHopCount(); i++) {
				FeedbackTag::telemetry tmt = fb.getFeedback(i);
				FeedbackTag::telemetry tmtPrev = FbPrev.getFeedback(i);
				if (tcb->m_sampleFeedback) {
					if (tmt.qlenDeq == 0 and fast_react)
						continue;
				}
				updated[i] = updated_any = true;

				double tau = GetTimeDelta(tmt.tsDeq, tmtPrev.tsDeq);
				double duration = tau * 1e-9;
				double txRate = GetBytesDelta(tmt.txBytes, tmtPrev.txBytes) * 8 / duration;
				double rxRate = txRate + (double(tmt.qlenDeq) - double(tmtPrev.qlenDeq)) * 8 / duration;
				double u;
//

				double A = rxRate;
				if (A < 0.5 * tmt.bandwidth && tmt.bandwidth == tmtPrev.bandwidth ) {
					A = 0.5 * tmt.bandwidth;
				}
				else
					A = rxRate;

				u = ( ( A ) * ( m_baseRtt.GetSeconds() * tmt.bandwidth + double(tmt.qlenDeq * 8) ) ) / ( tmt.bandwidth * (tmt.bandwidth * m_baseRtt.GetSeconds()));

				if (u > U) {
					U = u;
					dt = tau;
					if (tmt.bandwidth != tmtPrev.bandwidth)
						uAggregate = U;
				}
			}
			FbPrev = fb;
			lastAckedSeq = ackNum;



			DataRate new_rate;
			int32_t new_incStage;
			DataRate new_rate_per_hop[fb.getMaxHops()];
			int32_t new_incStage_per_hop[fb.getMaxHops()];

			if (updated_any) {
				if (dt > m_baseRtt.GetNanoSeconds())
					dt = m_baseRtt.GetNanoSeconds();

				uAggregate = (uAggregate * (m_baseRtt.GetNanoSeconds() - dt) + U * dt) / (double(m_baseRtt.GetNanoSeconds()));

				max_c = uAggregate / tcb->m_targetUtil;
				// std::cout << "U " << uAggregate << " time " << Simulator::Now().GetSeconds() << std::endl;

				if (max_c <= 0.5 )
					new_rate = DataRate(CCRateAgg.GetBitRate() / max_c + tcb->CCAddInc.GetBitRate()) ;
				else
					new_rate =  DataRate( 0.9 * (CCRateAgg.GetBitRate() / max_c + tcb->CCAddInc.GetBitRate()) + 0.1 * CCRateAgg.GetBitRate() ) ;

				if (new_rate < tcb->minCCRate)
					new_rate = tcb->minCCRate;
				if (new_rate > tcb->maxCCRate)
					new_rate = tcb->maxCCRate;
			}
			if (updated_any) {
				tcb->m_setCCRateCallback(tcb, new_rate, tcb->CCRate, m_baseRtt, true);
			}
			if (!fast_react) {
				if (updated_any) {
					CCRateAgg = new_rate;
					incStageAgg = new_incStage;
				}
			}
		}
		if (!fast_react) {
			if (next_seq > lastUpdatedSeq)
				lastUpdatedSeq = next_seq;
		}

	}
}



void TcpAdvanced::FastReactPowertcp(Ptr<Packet> packet, const TcpHeader& tcpHeader, Ptr<TcpSocketState> tcb, FeedbackTag fb) {
	if (tcb->m_fast_react)
		UpdateRatePowertcp(packet, tcpHeader, tcb, fb, true);
}


void TcpAdvanced::UpdateRateThetaPowertcp(Ptr<Packet> packet, const TcpHeader& tcpHeader, Ptr<TcpSocketState> tcb, FeedbackTag fb, bool fast_react) {
	uint32_t next_seq = tcb->m_nextTxSequence.Get().GetValue();
	uint32_t ackNum = tcpHeader.GetAckNumber().GetValue();

	if (lastUpdatedSeq == 0) {
		lastUpdatedSeq = next_seq;
		lastAckedSeq = ackNum;
		double rtt = Simulator::Now().GetNanoSeconds() - fb.getPktTimestamp();
		lastRTT = rtt;
		lastReceivedTime = Simulator::Now().GetNanoSeconds();
	} else {

		double max_c = 0;

		bool updated_any = true;

		uint64_t dt = Simulator::Now().GetNanoSeconds() - lastReceivedTime;

		lastReceivedTime = Simulator::Now().GetNanoSeconds();

		double rtt = Simulator::Now().GetNanoSeconds() - fb.getPktTimestamp();

		if (dt > m_baseRtt.GetNanoSeconds())
			dt = m_baseRtt.GetNanoSeconds();

		double A = double(rtt - lastRTT) / dt + 1;
		if (A < 0.5)
			A = 0.5;

		double U = double(rtt * (A)) / (m_baseRtt.GetNanoSeconds());

		lastRTT = rtt;
		lastAckedSeq = ackNum;

		DataRate new_rate;

		if (updated_any) {
			if (dt > m_baseRtt.GetNanoSeconds())
				dt = m_baseRtt.GetNanoSeconds();

			uAggregate = (uAggregate * (m_baseRtt.GetNanoSeconds() - dt) + U * dt) / (double(m_baseRtt.GetNanoSeconds()));

			max_c = uAggregate;
			// std::cout << "U " << uAggregate << " time " << Simulator::Now().GetSeconds() << std::endl;
			if (max_c < 0.5) {
				new_rate = DataRate( 0.9 * (CCRateAgg.GetBitRate() / max_c + tcb->CCAddInc.GetBitRate()) + 0.1 * CCRateAgg.GetBitRate() ) ;
			}
			else {
				new_rate =  DataRate( 0.7 * (CCRateAgg.GetBitRate() / max_c + tcb->CCAddInc.GetBitRate()) + 0.3 * CCRateAgg.GetBitRate() ) ;
			}

			if (new_rate < tcb->minCCRate)
				new_rate = tcb->minCCRate;
			if (new_rate > tcb->maxCCRate)
				new_rate = tcb->maxCCRate;
		}
		if (updated_any) {
			tcb->m_setCCRateCallback(tcb, new_rate, tcb->CCRate, m_baseRtt, true);
		}
		if (!fast_react) {
			if (updated_any) {
				CCRateAgg = new_rate;
			}
		}

		if (!fast_react) {
			if (next_seq > lastUpdatedSeq)
				lastUpdatedSeq = next_seq;
		}

	}
}



void TcpAdvanced::FastReactThetaPowertcp(Ptr<Packet> packet, const TcpHeader& tcpHeader, Ptr<TcpSocketState> tcb, FeedbackTag fb) {
	if (tcb->m_fast_react) {
		// UpdateRateThetaPowertcp(packet,tcpHeader, tcb,fb, true);
	}
}


void TcpAdvanced::UpdateRateHpcc(Ptr<Packet> packet, const TcpHeader& tcpHeader, Ptr<TcpSocketState> tcb, FeedbackTag fb, bool fast_react) {

	uint32_t next_seq = tcb->m_nextTxSequence.Get().GetValue();


	if (lastUpdatedSeq == 0) {
		lastUpdatedSeq = next_seq;

		NS_ASSERT(fb.getHopCount() <= fb.getMaxHops());
		FbPrev = fb;
	} else {

		if (fb.getHopCount() <= fb.getMaxHops()) {
			double max_c = 0;
			// check each hop
			double U = 0;
			uint64_t dt = 0;
			bool updated[fb.getMaxHops()] = {false}, updated_any = false;
			NS_ASSERT(fb.getHopCount() <= fb.getMaxHops());

			for (uint32_t i = 0; i < fb.getHopCount(); i++) {
				FeedbackTag::telemetry tmt = fb.getFeedback(i);
				FeedbackTag::telemetry tmtPrev = FbPrev.getFeedback(i);
				if (tcb->m_sampleFeedback) {
					if (tmt.qlenDeq == 0 and fast_react)
						continue;
				}
				updated[i] = updated_any = true;

				double tau = GetTimeDelta(tmt.tsDeq, tmtPrev.tsDeq);
				double duration = tau * 1e-9;
				double txRate = GetBytesDelta(tmt.txBytes, tmtPrev.txBytes) * 8 / duration;

				double	u = txRate / tmt.bandwidth + (double)std::min(tmt.qlenDeq, tmtPrev.qlenDeq) * 8.0 / (tmt.bandwidth * m_baseRtt.GetSeconds() );


				if (!tcb->m_multipleRate) {
					if (u > U) {
						U = u;
						dt = tau;
					}
				} else {
					if (tau > m_baseRtt.GetNanoSeconds())
						tau = m_baseRtt.GetNanoSeconds();
					uOld[i] = (uOld[i] * (m_baseRtt.GetNanoSeconds() - tau) + u * tau) / double(m_baseRtt.GetNanoSeconds());
				}
			}
			FbPrev = fb;

			DataRate new_rate;
			int32_t new_incStage;
			DataRate new_rate_per_hop[fb.getMaxHops()];
			int32_t new_incStage_per_hop[fb.getMaxHops()];
			if (!tcb->m_multipleRate) {
				if (updated_any) {
					if (dt > m_baseRtt.GetNanoSeconds())
						dt = m_baseRtt.GetNanoSeconds();
					uAggregate = (uAggregate * (m_baseRtt.GetNanoSeconds() - dt) + U * dt) / double(m_baseRtt.GetNanoSeconds());
					max_c = uAggregate / tcb->m_targetUtil;
					if (max_c >= 1 || incStageAgg >= tcb->m_miThresh) {
						new_rate = DataRate(CCRateAgg.GetBitRate() / max_c + tcb->CCAddInc.GetBitRate());
						new_incStage = 0;
					} else {
						new_rate = DataRate(CCRateAgg.GetBitRate() + tcb->CCAddInc.GetBitRate());
						new_incStage = incStageAgg + 1;
					}
					if (new_rate < tcb->minCCRate)
						new_rate = tcb->minCCRate;
					if (new_rate > tcb->maxCCRate)
						new_rate = tcb->maxCCRate;

				}
			} else {
				new_rate = tcb->maxCCRate;
				for (uint32_t i = 0; i < fb.getHopCount(); i++) {
					if (updated[i]) {
						double c = uOld[i] / tcb->m_targetUtil;
						if (c >= 1 || incStageHop[i] >= tcb->m_miThresh) {
							new_rate_per_hop[i] = DataRate(CCRateHop[i].GetBitRate() / c + tcb->CCAddInc.GetBitRate());
							new_incStage_per_hop[i] = 0;
						} else {
							new_rate_per_hop[i] = DataRate(CCRateHop[i].GetBitRate() + tcb->CCAddInc.GetBitRate());
							new_incStage_per_hop[i] = incStageHop[i] + 1;
						}
						if (new_rate_per_hop[i] < tcb->minCCRate)
							new_rate_per_hop[i] = tcb->minCCRate;
						if (new_rate_per_hop[i] > tcb->maxCCRate)
							new_rate_per_hop[i] = tcb->maxCCRate;
						if (new_rate_per_hop[i] < new_rate)
							new_rate = new_rate_per_hop[i];

					} else {
						if (CCRateHop[i] < new_rate)
							new_rate = CCRateHop[i];
					}
				}

			}
			if (updated_any)
				tcb->m_setCCRateCallback(tcb, new_rate, tcb->CCRate, m_baseRtt, true);

			if (!fast_react) {
				if (updated_any) {
					CCRateAgg = new_rate;
					incStageAgg = new_incStage;
				}
				if (tcb->m_multipleRate) {
					for (uint32_t i = 0; i < fb.getHopCount(); i++) {
						if (updated[i]) {
							CCRateHop[i] = new_rate_per_hop[i];
							incStageHop[i] = new_incStage_per_hop[i];
						}
					}
				}
			}
		}
		if (!fast_react) {
			if (next_seq > lastUpdatedSeq)
				lastUpdatedSeq = next_seq;
		}
	}
}

void TcpAdvanced::FastReactHpcc(Ptr<Packet> packet, const TcpHeader& tcpHeader, Ptr<TcpSocketState> tcb, FeedbackTag fb) {
	if (tcb->m_fast_react)
		UpdateRateHpcc(packet, tcpHeader, tcb, fb, true);
}


/*Timely*/
void TcpAdvanced::UpdateRateTimely(Ptr<Packet> packet, const TcpHeader& tcpHeader, Ptr<TcpSocketState> tcb, FeedbackTag fb, bool fast_react) {
	uint32_t next_seq = tcb->m_nextTxSequence.Get().GetValue();

	uint64_t rtt = Simulator::Now().GetNanoSeconds() - fb.getPktTimestamp();
	if (lastUpdatedSeq != 0) {
		int64_t new_rtt_diff = (int64_t)rtt - (int64_t)lastRTT;
		double rtt_diff = (1 - tcb->m_tmly_alpha) * lastRTTdiff + tcb->m_tmly_alpha * new_rtt_diff;
		double gradient = rtt_diff / m_baseRtt.GetNanoSeconds();
		bool inc = false;
		double c = 0;
		if (rtt < tcb->m_tmly_TLow) {
			inc = true;
		} else if (rtt > tcb->m_tmly_THigh) {
			c = 1 - tcb->m_tmly_beta * (1 - (double)tcb->m_tmly_THigh / rtt);
			inc = false;
		} else if (gradient <= 0) {
			inc = true;
		} else {
			c = 1 - tcb->m_tmly_beta * gradient;
			if (c < 0)
				c = 0;
			inc = false;
		}
		DataRate new_rate;
		if (inc) {
			if (incStageAgg < 5) {
				new_rate = CCRateAgg + tcb->CCAddInc;
			} else {
				new_rate = CCRateAgg + tcb->CCAddIncHigh;
			}
			if (new_rate < tcb->minCCRate)
				new_rate = tcb->minCCRate;
			if (new_rate > tcb->maxCCRate)
				new_rate = tcb->maxCCRate;

			if (!fast_react) {
				CCRateAgg = new_rate;
				incStageAgg++;
				lastRTTdiff = rtt_diff;
				tcb->m_setCCRateCallback(tcb, new_rate, tcb->CCRate, m_baseRtt, false);
			}
		}
		else {
			new_rate = std::max(tcb->minCCRate, CCRateAgg * c);
			if (!fast_react) {
				CCRateAgg = new_rate;
				incStageAgg = 0;
				lastRTTdiff = rtt_diff;
				tcb->m_setCCRateCallback(tcb, new_rate, tcb->CCRate, m_baseRtt, false);
			}
		}
	}
	if (!fast_react && next_seq > lastUpdatedSeq) {
		lastUpdatedSeq = next_seq;
		lastRTT = rtt;
	}
}

void TcpAdvanced::FastReactTimely(Ptr<Packet> packet, const TcpHeader& tcpHeader, Ptr<TcpSocketState> tcb, FeedbackTag fb) {
}




void
TcpAdvanced::IncreaseWindow (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked)
{
	NS_LOG_FUNCTION (this << tcb << segmentsAcked);
}

void
TcpAdvanced::ReduceCwnd (Ptr<TcpSocketState> tcb)
{
	NS_LOG_FUNCTION (this << tcb);
//  tcb->m_cWnd = std::max (tcb->m_cWnd.Get () / 2, tcb->m_segmentSize);

}



std::string
TcpAdvanced::GetName () const
{
	return "TcpAdvanced";
}

uint32_t
TcpAdvanced::GetSsThresh (Ptr<const TcpSocketState> tcb,
                          uint32_t bytesInFlight)
{
	NS_LOG_FUNCTION (this << tcb << bytesInFlight);
	return UINT32_MAX;
//  return std::max (std::min (tcb->m_ssThresh.Get (), tcb->m_cWnd.Get () - tcb->m_segmentSize), 2 * tcb->m_segmentSize);
}

} // namespace ns3
/* Modification */