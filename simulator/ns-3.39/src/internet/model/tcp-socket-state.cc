/*
 * Copyright (c) 2018 Natale Patriciello <natale.patriciello@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include "tcp-socket-state.h"

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(TcpSocketState);

TypeId
TcpSocketState::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::TcpSocketState")
            .SetParent<Object>()
            .SetGroupName("Internet")
            .AddConstructor<TcpSocketState>()

      /*Modification*/
    //////////////////////////////////////////////// For PowerTCP (called as Tcp wien in the code), HPCC ////////////////////////////////////////////////////////////////////////////////////////////////
    .AddAttribute("initCCRate"," initial rate that will be used by Tcp wien", DataRateValue(DataRate("1Gbps")), MakeDataRateAccessor (&TcpSocketState::CCRate),MakeDataRateChecker())
    .AddAttribute("minCCRate"," minimum rate that will be used by Tcp wien for each flow", DataRateValue(DataRate("100Mbps")), MakeDataRateAccessor (&TcpSocketState::minCCRate),MakeDataRateChecker())
    .AddAttribute("maxCCRate"," maximum rate that will be used by Tcp wien for each flow", DataRateValue(DataRate("10Gbps")), MakeDataRateAccessor (&TcpSocketState::maxCCRate),MakeDataRateChecker())
    .AddAttribute("AI"," additive increase Tcp wien", DataRateValue(DataRate("50Mbps")), MakeDataRateAccessor (&TcpSocketState::CCAddInc),MakeDataRateChecker())
    .AddAttribute("HighAI"," high additive increase used by Timely", DataRateValue(DataRate("150Mbps")), MakeDataRateAccessor (&TcpSocketState::CCAddIncHigh),MakeDataRateChecker())
    .AddAttribute ("mThreshHpcc", "threshold for the use for the use of MI in HPCC", UintegerValue (5), MakeUintegerAccessor (&TcpSocketState::m_miThresh), MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("fastReactHpcc", "fast react enable/disable for HPCC", BooleanValue (true), MakeBooleanAccessor (&TcpSocketState::m_fast_react), MakeBooleanChecker ())
    .AddAttribute ("sampleFeedbackHpcc", "sample feedback for HPCC", BooleanValue (false), MakeBooleanAccessor (&TcpSocketState::m_sampleFeedback), MakeBooleanChecker ())
    .AddAttribute ("useHpcc", "enable HPCC", BooleanValue (false), MakeBooleanAccessor (&TcpSocketState::useHpcc), MakeBooleanChecker ())
    .AddAttribute ("useTimely", "enable Timely", BooleanValue (false), MakeBooleanAccessor (&TcpSocketState::useTimely), MakeBooleanChecker ())
    .AddAttribute ("useThetaPowerTcp", "enable ThetaPowerTcp", BooleanValue (false), MakeBooleanAccessor (&TcpSocketState::useThetaPower), MakeBooleanChecker ())
    .AddAttribute ("usePowerTcp", "enable ThetaPowerTcp", BooleanValue (false), MakeBooleanAccessor (&TcpSocketState::usePower), MakeBooleanChecker ())
    .AddAttribute ("multipleRateHpcc", "whether to consider aggregate of all hops or each hop in HPCC", BooleanValue (false), MakeBooleanAccessor (&TcpSocketState::m_multipleRate), MakeBooleanChecker ())
    .AddAttribute ("targetUtil", "target utilization for TCP Wien and HPCC", DoubleValue (0.95), MakeDoubleAccessor (&TcpSocketState::m_targetUtil), MakeDoubleChecker<double> ())
    .AddAttribute ("baseRtt", "baseRtt in the topology", TimeValue (MicroSeconds (60)), MakeTimeAccessor (&TcpSocketState::CCInitRtt), MakeTimeChecker ())
    .AddAttribute ("m_useSS", "use slowstart or not", BooleanValue (true), MakeBooleanAccessor (&TcpSocketState::m_useSS), MakeBooleanChecker ())
    .AddAttribute ("TimelyAlpha", "timely alpha", DoubleValue (0.875), MakeDoubleAccessor (&TcpSocketState::m_tmly_alpha), MakeDoubleChecker<double> ())
    .AddAttribute ("TimelyBeta", "timely beta", DoubleValue (0.8), MakeDoubleAccessor (&TcpSocketState::m_tmly_beta), MakeDoubleChecker<double> ())
    .AddAttribute ("TimelyThigh", "high threshold for timely in NANOSECONDS", UintegerValue (500000), MakeUintegerAccessor (&TcpSocketState::m_tmly_THigh), MakeUintegerChecker<uint64_t> ())
    .AddAttribute ("TimelyTlow", "low threshold for timely in NANOSECONDS", UintegerValue (200000), MakeUintegerAccessor (&TcpSocketState::m_tmly_TLow), MakeUintegerChecker<uint64_t> ())
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /*Modification*/

            .AddAttribute("EnablePacing",
                          "Enable Pacing",
                          BooleanValue(false),
                          MakeBooleanAccessor(&TcpSocketState::m_pacing),
                          MakeBooleanChecker())
            .AddAttribute("MaxPacingRate",
                          "Set Max Pacing Rate",
                          DataRateValue(DataRate("4Gb/s")),
                          MakeDataRateAccessor(&TcpSocketState::m_maxPacingRate),
                          MakeDataRateChecker())
            .AddAttribute("PacingSsRatio",
                          "Percent pacing rate increase for slow start conditions",
                          UintegerValue(200),
                          MakeUintegerAccessor(&TcpSocketState::m_pacingSsRatio),
                          MakeUintegerChecker<uint16_t>())
            .AddAttribute("PacingCaRatio",
                          "Percent pacing rate increase for congestion avoidance conditions",
                          UintegerValue(120),
                          MakeUintegerAccessor(&TcpSocketState::m_pacingCaRatio),
                          MakeUintegerChecker<uint16_t>())
            .AddAttribute("PaceInitialWindow",
                          "Perform pacing for initial window of data",
                          BooleanValue(false),
                          MakeBooleanAccessor(&TcpSocketState::m_paceInitialWindow),
                          MakeBooleanChecker())
            .AddTraceSource("PacingRate",
                            "The current TCP pacing rate",
                            MakeTraceSourceAccessor(&TcpSocketState::m_pacingRate),
                            "ns3::TracedValueCallback::DataRate")
            .AddTraceSource("CongestionWindow",
                            "The TCP connection's congestion window",
                            MakeTraceSourceAccessor(&TcpSocketState::m_cWnd),
                            "ns3::TracedValueCallback::Uint32")
            .AddTraceSource("CongestionWindowInflated",
                            "The TCP connection's inflated congestion window",
                            MakeTraceSourceAccessor(&TcpSocketState::m_cWndInfl),
                            "ns3::TracedValueCallback::Uint32")
            .AddTraceSource("SlowStartThreshold",
                            "TCP slow start threshold (bytes)",
                            MakeTraceSourceAccessor(&TcpSocketState::m_ssThresh),
                            "ns3::TracedValueCallback::Uint32")
            .AddTraceSource("CongState",
                            "TCP Congestion machine state",
                            MakeTraceSourceAccessor(&TcpSocketState::m_congState),
                            "ns3::TracedValueCallback::TcpCongState")
            .AddTraceSource("EcnState",
                            "Trace ECN state change of socket",
                            MakeTraceSourceAccessor(&TcpSocketState::m_ecnState),
                            "ns3::TracedValueCallback::EcnState")
            .AddTraceSource("HighestSequence",
                            "Highest sequence number received from peer",
                            MakeTraceSourceAccessor(&TcpSocketState::m_highTxMark),
                            "ns3::TracedValueCallback::SequenceNumber32")
            .AddTraceSource("NextTxSequence",
                            "Next sequence number to send (SND.NXT)",
                            MakeTraceSourceAccessor(&TcpSocketState::m_nextTxSequence),
                            "ns3::TracedValueCallback::SequenceNumber32")
            .AddTraceSource("BytesInFlight",
                            "The TCP connection's congestion window",
                            MakeTraceSourceAccessor(&TcpSocketState::m_bytesInFlight),
                            "ns3::TracedValueCallback::Uint32")
            .AddTraceSource("RTT",
                            "Last RTT sample",
                            MakeTraceSourceAccessor(&TcpSocketState::m_lastRtt),
                            "ns3::TracedValueCallback::Time");
    return tid;
}

TcpSocketState::TcpSocketState(const TcpSocketState& other)
    : Object(other),
      m_cWnd(other.m_cWnd),
      m_ssThresh(other.m_ssThresh),
      m_initialCWnd(other.m_initialCWnd),
      m_initialSsThresh(other.m_initialSsThresh),
      m_segmentSize(other.m_segmentSize),
      m_lastAckedSeq(other.m_lastAckedSeq),
      m_congState(other.m_congState),
      m_ecnState(other.m_ecnState),
      m_highTxMark(other.m_highTxMark),
      m_nextTxSequence(other.m_nextTxSequence),
      m_rcvTimestampValue(other.m_rcvTimestampValue),
      m_rcvTimestampEchoReply(other.m_rcvTimestampEchoReply),
      m_pacing(other.m_pacing),
      m_maxPacingRate(other.m_maxPacingRate),
      m_pacingRate(other.m_pacingRate),
      m_pacingSsRatio(other.m_pacingSsRatio),
      m_pacingCaRatio(other.m_pacingCaRatio),
      m_paceInitialWindow(other.m_paceInitialWindow),
      m_minRtt(other.m_minRtt),
      m_bytesInFlight(other.m_bytesInFlight),
      m_lastRtt(other.m_lastRtt),
      m_ecnMode(other.m_ecnMode),
      m_useEcn(other.m_useEcn),
      m_ectCodePoint(other.m_ectCodePoint),
      m_lastAckedSackedBytes(other.m_lastAckedSackedBytes)

{
}

const char* const TcpSocketState::TcpCongStateName[TcpSocketState::CA_LAST_STATE] = {
    "CA_OPEN",
    "CA_DISORDER",
    "CA_CWR",
    "CA_RECOVERY",
    "CA_LOSS",
};

const char* const TcpSocketState::EcnStateName[TcpSocketState::ECN_CWR_SENT + 1] = {
    "ECN_DISABLED",
    "ECN_IDLE",
    "ECN_CE_RCVD",
    "ECN_SENDING_ECE",
    "ECN_ECE_RCVD",
    "ECN_CWR_SENT",
};

} // namespace ns3
