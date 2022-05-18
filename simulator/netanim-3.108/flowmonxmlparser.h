/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 *
 * Author: John Abraham <john.abraham.in@gmail.com>
 */

#ifndef FLOWMONXMLPARSER_H
#define FLOWMONXMLPARSER_H

#include "common.h"

namespace netanim
{


typedef struct
{
  uint32_t index;
  double start;
  double width;
  uint32_t count;
} FlowBin_t;

typedef std::vector <FlowBin_t> FlowBinVector_t;

typedef struct
{
  uint32_t flowId;
  QString sourceAddress;
  QString destinationAddress;
  uint16_t protocol;
  uint16_t sourcePort;
  uint16_t destinationPort;

} Ipv4Classifier_t;

typedef struct
{
  QString name;
  uint32_t nBins;
  FlowBinVector_t bins;
} Histogram_t;

typedef std::vector <Histogram_t> HistogramVector_t;
typedef struct
{
  uint8_t reasonCode;
  uint64_t number;
} Reason_t;
typedef std::vector<Reason_t> BytesDroppedReasonVector_t;
typedef std::vector<Reason_t> PacketsDroppedReasonVector_t;

typedef struct
{
  uint32_t flowId;
  double timeFirstTxPacket;
  double timeFirstRxPacket;
  double timeLastTxPacket;
  double timeLastRxPacket;
  double delaySum;
  double jitterSum;
  double lastDelay;
  uint64_t txBytes;
  uint64_t rxBytes;
  uint64_t txPackets;
  uint64_t rxPackets;
  uint64_t lostPackets;
  uint64_t timesForwarded;
  BytesDroppedReasonVector_t bytesDropped;
  PacketsDroppedReasonVector_t packetsDropped;
  HistogramVector_t histograms;
} FlowStatsFlow_t;


typedef struct
{
  uint32_t flowId;
  uint64_t packets;
  uint64_t bytes;
  double delayFromFirstProbeSum;

} FlowProbeFlowStats_t;

typedef std::vector<FlowProbeFlowStats_t> FlowProbe_t;
typedef std::vector<FlowProbe_t> FlowProbes_t;



struct FlowMonParsedElement
{
  enum FlowMonParsedElementType
  {
    XML_INVALID,
    XML_FLOWMONITOR,
    XML_FLOWSTATS,
    XML_IPV4CLASSIFIER,
    XML_FLOWSTATSFLOW,
    XML_IPV4CLASSFLOW,
    XML_FLOWPROBES,
    XML_FLOWPROBE,
    XML_FLOWPROBEFLOW
  };
  FlowMonParsedElementType type;
  Ipv4Classifier_t ipv4Classifier;
  FlowStatsFlow_t flowStats;
  FlowProbes_t flowProbes;

};

class FlowMonXmlparser
{
public:
  FlowMonXmlparser (QString traceFileName);
  ~FlowMonXmlparser ();
  FlowMonParsedElement parseNext ();
  bool isParsingComplete ();
  bool isFileValid ();
  uint64_t getFlowCount ();
  void doParse ();
  FlowMonParsedElement parseFlowMonitor ();
  FlowMonParsedElement parseFlowStats ();
  FlowMonParsedElement parseFlowProbes ();
  FlowProbe_t parseFlowProbe ();
  FlowProbeFlowStats_t parseFlowProbeFlowStats ();
  FlowProbeFlowStats_t parseFlowProbeFlowStat ();
  FlowMonParsedElement parseFlowStatsFlow ();
  FlowMonParsedElement parseIpv4Classifier ();
  FlowMonParsedElement parseIpv4ClassifierFlow ();
  Histogram_t parseHistogram (QString name);
  void parsePacketsDropped (FlowStatsFlow_t &);
  void parseBytesDropped (FlowStatsFlow_t &);




private:
  enum state
  {
    INIT,
    FLOWMONITOR,
    FLOWSTATS,
    IPV4CLASSIFIER,
    FLOWPROBES,
    FLOWPROBE
  };
  typedef std::map<uint32_t, Ipv4Classifier_t> FlowIdIpv4ClassifierMap_t;


  QString m_traceFileName;
  bool m_parsingComplete;
  QXmlStreamReader * m_reader;
  bool m_fileIsValid;
  QFile * m_traceFile;
  state m_state;
  FlowIdIpv4ClassifierMap_t m_flowIdIpv4Classifiers;

};



} // namespace netanim

#endif // FLOWMONXMLPARSER_H
