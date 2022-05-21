/* Modification */
/*
 * tcp-advanced.h
 *
 *  Created on: Jan 9, 2021
 *      Author: vamsi
 */

#ifndef SRC_INTERNET_MODEL_TCP_ADVANCED_H_
#define SRC_INTERNET_MODEL_TCP_ADVANCED_H_


#include "tcp-congestion-ops.h"
#include "ns3/feedback-tag.h"

namespace ns3 {

class TcpSocketState;

class TcpAdvanced : public TcpNewReno
{
public:

  static TypeId GetTypeId (void);


  TcpAdvanced (void);

  /**
   * \brief Copy constructor
   * \param sock the object to copy
   */
  TcpAdvanced (const TcpAdvanced& sock);
  virtual ~TcpAdvanced (void);

  virtual std::string GetName () const;

  virtual void Init (Ptr<TcpSocketState> tcb);

  /**
   * \brief Compute RTTs needed to execute Vegas algorithm
   *
   * The function filters RTT samples from the last RTT to find
   * the current smallest propagation delay + queueing delay (minRtt).
   * We take the minimum to avoid the effects of delayed ACKs.
   *
   * The function also min-filters all RTT measurements seen to find the
   * propagation delay (baseRtt).
   *
   * \param tcb internal congestion state
   * \param segmentsAcked count of segments ACKed
   * \param rtt last RTT
   *
   */
//  virtual void PktsAcked (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked,
//                          const Time& rtt);

  /**
   * \brief Enable/disable Vegas algorithm depending on the congestion state
   *
   * We only start a Vegas cycle when we are in normal congestion state (CA_OPEN state).
   *
   * \param tcb internal congestion state
   * \param newState new congestion state to which the TCP is going to switch
   */
//  virtual void CongestionStateSet (Ptr<TcpSocketState> tcb,
//                                   const TcpSocketState::TcpCongState_t newState);

  /**
   * \brief Adjust cwnd following Vegas linear increase/decrease algorithm
   *
   * \param tcb internal congestion state
   * \param segmentsAcked count of segments ACKed
   */
  virtual void IncreaseWindow (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked);

  virtual void ReduceCwnd (Ptr<TcpSocketState> tcb);

  virtual void ProcessDcAck(Ptr<Packet> packet, const TcpHeader& tcpHeader, Ptr<TcpSocketState> tcb);

  void UpdateRateHpcc(Ptr<Packet> packet, const TcpHeader& tcpHeader, Ptr<TcpSocketState> tcb, FeedbackTag fb, bool fast_react);
  void FastReactHpcc(Ptr<Packet> packet, const TcpHeader& tcpHeader, Ptr<TcpSocketState> tcb, FeedbackTag fb);

  void UpdateRateTimely(Ptr<Packet> packet, const TcpHeader& tcpHeader, Ptr<TcpSocketState> tcb, FeedbackTag fb, bool fast_react);
  void FastReactTimely(Ptr<Packet> packet, const TcpHeader& tcpHeader, Ptr<TcpSocketState> tcb, FeedbackTag fb);

  void UpdateRatePowertcp(Ptr<Packet> packet, const TcpHeader& tcpHeader, Ptr<TcpSocketState> tcb, FeedbackTag fb, bool fast_react);
  void FastReactPowertcp(Ptr<Packet> packet, const TcpHeader& tcpHeader, Ptr<TcpSocketState> tcb, FeedbackTag fb);

  void UpdateRateThetaPowertcp(Ptr<Packet> packet, const TcpHeader& tcpHeader, Ptr<TcpSocketState> tcb, FeedbackTag fb, bool fast_react);
  void FastReactThetaPowertcp(Ptr<Packet> packet, const TcpHeader& tcpHeader, Ptr<TcpSocketState> tcb, FeedbackTag fb);



  void SetCCRate(Ptr<TcpSocketState> tcb, DataRate rate, DataRate prevRate);


  /**
   * \brief Get slow start threshold following Vegas principle
   *
   * \param tcb internal congestion state
   * \param bytesInFlight bytes in flight
   *
   * \return the slow start threshold value
   */
  virtual uint32_t GetSsThresh (Ptr<const TcpSocketState> tcb,
                                uint32_t bytesInFlight);

  virtual Ptr<TcpCongestionOps> Fork ();

protected:
private:


  void EnableVegas (Ptr<TcpSocketState> tcb);

  void DisableVegas ();

private:
  uint32_t m_alpha;                  //!< Alpha threshold, lower bound of packets in network
  uint32_t m_beta;                   //!< Beta threshold, upper bound of packets in network
  uint32_t m_gamma;                  //!< Gamma threshold, limit on increase
  Time m_baseRtt=Seconds(1);                    //!< Minimum of all Vegas RTT measurements seen during connection
  Time m_minRtt=Seconds(1);                     //!< Minimum of all RTT measurements within last RTT
  uint32_t m_cntRtt;                 //!< Number of RTT measurements during last RTT
  bool m_doingVegasNow;              //!< If true, do Vegas for this RTT
  SequenceNumber32 m_begSndNxt;      //!< Right edge during last RTT
  FeedbackTag FbPrev;

  uint32_t lastUpdatedSeq=0;
  uint32_t lastAckedSeq=0;
  uint32_t updateTimer=0;

  uint32_t QlenRTT;
uint32_t numTxBytes;

  double uOld[100]={0};
  double uAggregate=0;
  double uAggregateSimple=0;
  uint64_t OldBandwidth;
  uint64_t uCounter=0;
  uint64_t updaterCounter=0;
  uint64_t CCCounter=0;

  DataRate CCRateAgg;
  DataRate CCRateHop[10]; // 10 hops max.
  uint32_t CCWait=0;
  uint32_t incStageAgg;
  uint32_t incStageHop[10];
  bool hpcc = false;

  uint64_t lastRTT=0;
  uint64_t lastRTTdiff=0;
  uint64_t minRTT;
  uint64_t lastReceivedTime=0;
};

} // namespace ns3

#endif /* SRC_INTERNET_MODEL_TCP_ADVANCED_H_ */
/* Modification */