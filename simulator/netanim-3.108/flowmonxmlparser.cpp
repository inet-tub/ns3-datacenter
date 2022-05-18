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
 * Contributions: Makhtar Diouf <makhtar.diouf@gmail.com>
 */

#include "common.h"
#include "log.h"
#include "flowmonxmlparser.h"
#include "animatormode.h"
#include "flowmonstatsscene.h"
#include <exception>

NS_LOG_COMPONENT_DEFINE ("FlowMonXmlParser");

namespace netanim
{

FlowMonXmlparser::FlowMonXmlparser (QString traceFileName):
  m_traceFileName (traceFileName),
  m_parsingComplete (false),
  m_reader (0),
  m_fileIsValid (true),
  m_state (INIT)
{
  if (m_traceFileName == "")
    return;

  m_traceFile = new QFile (m_traceFileName);
  try
    {
      if ((m_traceFile->size () <= 0) || !m_traceFile->open (QIODevice::ReadOnly | QIODevice::Text))
        {
          m_fileIsValid = false;
          return;
        }
    m_reader = new QXmlStreamReader (m_traceFile);
    }
  catch (std::exception& e)
    {
      NS_LOG_DEBUG ("Unable to load flowmonitor file:" << e.what ());
      m_fileIsValid = false;
    }
}

FlowMonXmlparser::~FlowMonXmlparser ()
{
  if (m_traceFile)
    delete m_traceFile;
  if (m_reader)
    delete m_reader;
}


uint64_t
FlowMonXmlparser::getFlowCount ()
{
  uint64_t count = 0;
  QFile * f = new QFile (m_traceFileName);
  if (f->open (QIODevice::ReadOnly | QIODevice::Text))
    {
      QString allContent = QString (f->readAll ());
      int j = 0;
      QString searchString = "flow=";
      while ( (j = allContent.indexOf (searchString, j)) != -1)
        {
          ++j;
          ++count;
        }
      f->close ();
      delete f;
      //qDebug (QString::number (count));
      return count;
    }
  return count;
}

bool
FlowMonXmlparser::isFileValid ()
{
  return m_fileIsValid;
}

bool
FlowMonXmlparser::isParsingComplete ()
{
  return m_parsingComplete;
}


void
FlowMonXmlparser::doParse ()
{
  uint64_t parsedElementCount = 0;
  while (!isParsingComplete ())
    {
      if (AnimatorMode::getInstance ()->keepAppResponsive ())
        {
          //AnimatorMode::getInstance ()->setParsingCount (parsedElementCount);

        }
      FlowMonParsedElement parsedElement = parseNext ();
      switch (parsedElement.type)
        {
        case FlowMonParsedElement::XML_FLOWMONITOR:
        {
          break;
        }
        case FlowMonParsedElement::XML_FLOWSTATS:
        {
          break;
        }
        case FlowMonParsedElement::XML_FLOWSTATSFLOW:
        {
          ++parsedElementCount;
          FlowMonStatsScene::getInstance ()->addFlowStat (parsedElement.flowStats.flowId, parsedElement.flowStats);
          break;
        }
        case FlowMonParsedElement::XML_IPV4CLASSFLOW:
        {
          ++parsedElementCount;
          FlowMonStatsScene::getInstance ()->addIpv4Classifier (parsedElement.ipv4Classifier.flowId, parsedElement.ipv4Classifier);
          break;
        }
        case FlowMonParsedElement::XML_FLOWPROBES:
        {
          ++parsedElementCount;
          FlowMonStatsScene::getInstance ()->addFlowProbes (parsedElement.flowProbes);
          break;

        }
        case FlowMonParsedElement::XML_INVALID:
        default:
        {
          //qDebug ("Invalid XML element");
        }
        } //switch
    } // while loop
}

FlowMonParsedElement
FlowMonXmlparser::parseNext ()
{
  FlowMonParsedElement parsedElement;
  parsedElement.type = FlowMonParsedElement::XML_INVALID;

  if (m_reader->atEnd () || m_reader->hasError ())
    {
      m_parsingComplete = true;
      m_traceFile->close ();
      return parsedElement;
    }
  //qDebug ("T" + m_reader->text ().toString ())  ;




  QXmlStreamReader::TokenType token =  m_reader->readNext ();
  if (token == QXmlStreamReader::StartDocument)
    {
      return parsedElement;
    }

  if (token == QXmlStreamReader::StartElement)
    {
      if (m_reader->name () == "FlowMonitor")
        {
          //QDEBUG ("FlowMonitor");
        }
      if (m_reader->name () == "FlowStats")
        {
          parsedElement = parseFlowStats ();
        }
      if (m_reader->name () == "Ipv4FlowClassifier")
        {
          parsedElement = parseIpv4Classifier ();
        }
      if (m_reader->name () == "FlowProbes")
        {
          parsedElement = parseFlowProbes ();
        }

      if (m_reader->name () == "Flow")
        {
          if (m_state == FLOWSTATS)
            {
              parsedElement = parseFlowStatsFlow ();
            }
          else if (m_state == IPV4CLASSIFIER)
            {
              parsedElement = parseIpv4ClassifierFlow ();
            }
        }



    }

  //qDebug (m_reader->errorString ());
  //qDebug (m_reader->tokenString ());

  if (m_reader->atEnd ())
    {
      m_parsingComplete = true;
      m_traceFile->close ();
    }
  return parsedElement;
}

FlowMonParsedElement
FlowMonXmlparser::parseFlowMonitor ()
{
  FlowMonParsedElement parsedElement;
  parsedElement.type = FlowMonParsedElement::XML_FLOWMONITOR;
  m_state = FLOWMONITOR;
  return parsedElement;
}


FlowMonParsedElement
FlowMonXmlparser::parseFlowStats ()
{
  FlowMonParsedElement parsedElement;
  parsedElement.type = FlowMonParsedElement::XML_FLOWSTATS;
  m_state = FLOWSTATS;
  return parsedElement;
}


FlowProbeFlowStats_t
FlowMonXmlparser::parseFlowProbeFlowStats ()
{
  FlowProbeFlowStats_t flowStats;
  flowStats.flowId = m_reader->attributes ().value ("flowId").toString ().toUInt ();
  flowStats.bytes = m_reader->attributes ().value ("bytes").toString ().toULong ();
  flowStats.delayFromFirstProbeSum = m_reader->attributes ().value ("delayFromFirstProbeSum").toString ().remove ("ns").toDouble ();
  flowStats.packets = m_reader->attributes ().value ("packets").toString ().toULong ();
  return flowStats;
}

FlowProbe_t
FlowMonXmlparser::parseFlowProbe ()
{

  FlowProbe_t probe;
  QXmlStreamReader::TokenType token = m_reader->readNext ();

  while ( (m_reader->name () != "FlowProbe"))
    {

      if ((m_reader->name () == "FlowStats") && (token != QXmlStreamReader::EndElement))
        {
          if (m_state != FLOWPROBE)
            {
            }
          //QDEBUG (m_reader->name ().toString ());
          //QDEBUG (m_reader->tokenString ());
          probe.push_back (parseFlowProbeFlowStats ());

        }
      token = m_reader->readNext ();
    }
  return probe;
}

FlowMonParsedElement
FlowMonXmlparser::parseFlowProbes ()
{
  FlowMonParsedElement parsedElement;
  parsedElement.type = FlowMonParsedElement::XML_FLOWPROBES;
  m_state = FLOWPROBE;
  QXmlStreamReader::TokenType token = m_reader->readNext ();
  while ( (m_reader->name () != "FlowProbes"))
    {

      if ((m_reader->name () == "FlowProbe") && (token != QXmlStreamReader::EndElement))
        {
          if (m_state != FLOWPROBE)
            {
            }
          //QDEBUG (m_reader->name ().toString ());
          //QDEBUG (m_reader->tokenString ());
          parsedElement.flowProbes.push_back (parseFlowProbe ());

        }
      token = m_reader->readNext ();
    }
  return parsedElement;
}

Histogram_t
FlowMonXmlparser::parseHistogram (QString name)
{
  Histogram_t histogram;
  histogram.name = name;
  histogram.nBins = m_reader->attributes ().value ("nBins").toString ().toUInt ();
  QXmlStreamReader::TokenType token = m_reader->readNext ();
  while (m_reader->name () != name)
    {
      if (m_reader->name () == "bin" && (token != QXmlStreamReader::EndElement))
        {
          FlowBin_t flowBin;
          flowBin.index = m_reader->attributes ().value ("index").toString ().toUInt ();
          flowBin.start = m_reader->attributes ().value ("start").toString ().toDouble ();
          flowBin.width = m_reader->attributes ().value ("width").toString ().toDouble ();
          flowBin.count = m_reader->attributes ().value ("count").toString ().toUInt ();
          histogram.bins.push_back (flowBin);
        }
      token = m_reader->readNext ();
    }
  return histogram;
}

FlowMonParsedElement
FlowMonXmlparser::parseFlowStatsFlow ()
{
  FlowMonParsedElement parsedElement;
  parsedElement.type = FlowMonParsedElement::XML_FLOWSTATSFLOW;
  FlowStatsFlow_t flow;
  flow.flowId = m_reader->attributes ().value ("flowId").toString ().toUInt ();
  //QDEBUG ("Flow:" + m_reader->attributes ().value ("flowId").toString ());
  flow.timeFirstTxPacket = m_reader->attributes ().value ("timeFirstTxPacket").toString ().remove ("ns").toDouble ();
  flow.timeFirstRxPacket = m_reader->attributes ().value ("timeFirstRxPacket").toString ().remove ("ns").toDouble ();
  flow.timeLastTxPacket = m_reader->attributes ().value ("timeLastTxPacket").toString ().remove ("ns").toDouble ();
  flow.timeLastRxPacket = m_reader->attributes ().value ("timeLastRxPacket").toString ().remove ("ns").toDouble ();
  flow.delaySum = m_reader->attributes ().value ("delaySum").toString ().remove ("ns").toDouble ();
  flow.jitterSum = m_reader->attributes ().value ("jitterSum").toString ().remove ("ns").toDouble ();
  flow.lastDelay = m_reader->attributes ().value ("lastDelay").toString ().remove ("ns").toDouble ();
  flow.txBytes = m_reader->attributes ().value ("txBytes").toString ().toULong ();
  flow.rxBytes = m_reader->attributes ().value ("rxBytes").toString ().toULong ();
  flow.txPackets = m_reader->attributes ().value ("txPackets").toString ().toULong ();
  flow.rxPackets = m_reader->attributes ().value ("rxPackets").toString ().toULong ();
  flow.lostPackets = m_reader->attributes ().value ("lostPackets").toString ().toULong ();
  flow.timesForwarded = m_reader->attributes ().value ("timesForwarded").toString ().toULong ();

  QXmlStreamReader::TokenType token = m_reader->readNext ();
  while (m_reader->name () != "Flow")
    {
      if ((m_reader->name () == "packetsDropped") && (token != QXmlStreamReader::EndElement))
        {
          if (m_state != FLOWSTATS)
            {
            }
          //QDEBUG (m_reader->name ().toString ());
          //QDEBUG (m_reader->tokenString ());

          parsePacketsDropped (flow);
        }
      if ((m_reader->name () == "bytesDropped") && (token != QXmlStreamReader::EndElement))
        {

          if (m_state != FLOWSTATS)
            {
            }
          //QDEBUG (m_reader->name ().toString ());
          //QDEBUG (m_reader->tokenString ());

          parseBytesDropped (flow);
        }
      if (m_reader->name () == "delayHistogram")
        {
          flow.histograms.push_back (parseHistogram ("delayHistogram"));
        }
      else if (m_reader->name () == "jitterHistogram")
        {
          flow.histograms.push_back (parseHistogram ("jitterHistogram"));
        }
      else if (m_reader->name () == "flowInterruptionsHistogram")
        {
          flow.histograms.push_back (parseHistogram ("flowInterruptionsHistogram"));
        }
      else if (m_reader->name () == "packetSizeHistogram")
        {
          flow.histograms.push_back (parseHistogram ("packetSizeHistogram"));
        }
      token = m_reader->readNext ();
    }


  parsedElement.flowStats = flow;
  return parsedElement;
}


FlowMonParsedElement
FlowMonXmlparser::parseIpv4Classifier ()
{
  FlowMonParsedElement parsedElement;
  parsedElement.type = FlowMonParsedElement::XML_IPV4CLASSIFIER;
  m_state = IPV4CLASSIFIER;
  return parsedElement;
}


FlowMonParsedElement
FlowMonXmlparser::parseIpv4ClassifierFlow ()
{
  FlowMonParsedElement parsedElement;
  parsedElement.type = FlowMonParsedElement::XML_IPV4CLASSFLOW;
  Ipv4Classifier_t classifier;
  classifier.flowId = m_reader->attributes ().value ("flowId").toString ().toUInt ();
  classifier.protocol = m_reader->attributes ().value ("protocol").toString ().toUShort ();
  classifier.sourcePort = m_reader->attributes ().value ("sourcePort").toString ().toUShort ();
  classifier.destinationPort = m_reader->attributes ().value ("destinationPort").toString ().toUShort ();
  classifier.sourceAddress = m_reader->attributes ().value ("sourceAddress").toString ();
  classifier.destinationAddress = m_reader->attributes ().value ("destinationAddress").toString ();
  parsedElement.ipv4Classifier = classifier;
  return parsedElement;
}

void
FlowMonXmlparser::parsePacketsDropped (FlowStatsFlow_t & flow)
{
  Reason_t reason;
  reason.reasonCode = m_reader->attributes ().value ("reasonCode").toString ().toUInt ();
  reason.number = m_reader->attributes ().value ("number").toString ().toUInt ();
  flow.packetsDropped.push_back (reason);

}

void
FlowMonXmlparser::parseBytesDropped (FlowStatsFlow_t & flow)
{
  Reason_t reason;
  reason.reasonCode = m_reader->attributes ().value ("reasonCode").toString ().toUInt ();
  reason.number = m_reader->attributes ().value ("bytes").toString ().toUInt ();
  flow.bytesDropped.push_back (reason);

}

} // namespace netanim
