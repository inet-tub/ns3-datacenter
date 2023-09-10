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

#include "statsmode.h"
#include "flowmonstatsscene.h"
#include "statisticsconstants.h"
#include "textbubble.h"


namespace netanim
{

FlowMonStatsScene * pFlowMonStatsScene = 0;

FlowMonStatsScene::FlowMonStatsScene ():QGraphicsScene (100, 0, STATSSCENE_WIDTH_DEFAULT, STATSSCENE_HEIGHT_DEFAULT),
  m_flowProbeWidget (0)
{
  m_infoWidget = addWidget (new TextBubble ("Info:", "No data available\nDid you load the XML file?"));
  showInfoWidget ();

}

FlowMonStatsScene *
FlowMonStatsScene::getInstance ()
{
  if (!pFlowMonStatsScene)
    {
      pFlowMonStatsScene = new FlowMonStatsScene;
    }
  return pFlowMonStatsScene;
}

void
FlowMonStatsScene::systemReset ()
{
  m_lastX = 0;
  m_lastY = 0;
  m_bottomY = 0;
  clearProxyWidgetsMap ();
  m_flowIdFlowStats.clear ();
  m_flowIdIpv4Classifiers.clear ();
  showInfoWidget ();
}

void
FlowMonStatsScene::addFlowStat (uint32_t flowId, FlowStatsFlow_t flowStats)
{
  m_flowIdFlowStats[flowId] = flowStats;
}

void
FlowMonStatsScene::addIpv4Classifier (uint32_t flowId, Ipv4Classifier_t ipv4Classifier)
{
  m_flowIdIpv4Classifiers[flowId] = ipv4Classifier;
}

void
FlowMonStatsScene::addFlowProbes (FlowProbes_t flowProbes)
{
  m_flowProbes = flowProbes;
}

void
FlowMonStatsScene::showInfoWidget (bool show)
{
  m_infoWidget->setVisible (show);
  m_infoWidget->setPos (sceneRect ().width ()/2, sceneRect ().height ()/2);
}


void
FlowMonStatsScene::clearProxyWidgetsMap ()
{
  showInfoWidget ();
  for (FlowIdProxyWidgetMap_t::const_iterator i = m_flowIdProxyWidgets.begin ();
      i != m_flowIdProxyWidgets.end ();
      ++i)
    {
      removeItem (i->second);
      delete (i->second);
    }
  m_flowIdProxyWidgets.clear ();
  m_flowProbes.clear ();
  if (m_flowProbeWidget)
    {
      removeItem (m_flowProbeWidget);
      delete (m_flowProbeWidget);
      m_flowProbeWidget = 0;
    }
  m_flowIdFlowStats.clear ();
  m_flowIdIpv4Classifiers.clear ();
}

QString dropReasonToString (int reasonCode)
{
  switch (reasonCode)
    {
    case 0:
      return "No Route";
    case 1:
      return "TTL Expire";
    case 2:
      return "Bad Checksum";
    case 3:
      return "Queue";
    case 4:
      return "Interface Down";
    case 5:
      return "Route error";
    case 6:
      return "Fragment timeout";
    default:
      return "Unknown";
    }
}

QString
FlowMonStatsScene::flowStatsToString (FlowStatsFlow_t flowStats)
{
  QString str;
  int justify = 20;
  double denominator = (flowStats.timeLastTxPacket - flowStats.timeFirstTxPacket);
  if (!denominator)
    {
      denominator = -1;
    }
  double txBitRate = (8 * flowStats.txBytes) * 1e9 * 1e-3 / (denominator);
  denominator = (flowStats.timeLastRxPacket - flowStats.timeFirstRxPacket);
  if (!denominator)
    {
      denominator = -1;
    }
  double rxBitRate = (8 * flowStats.rxBytes) * 1e9 * 1e-3 / (denominator);
  double meanDelay = -1;
  if (flowStats.rxPackets)
    {
      meanDelay = (1000 * flowStats.delaySum/ (flowStats.rxPackets * 1e9));
    }
  uint64_t  sumPackets = flowStats.rxPackets + flowStats.lostPackets;
  double packetLostRatio = -1;
  if (sumPackets)
    {
      packetLostRatio = (double)flowStats.lostPackets/sumPackets;
    }

  str += "\nTx bitrate:" + QString::number (txBitRate) + "kbps\n";
  str += "Rx bitrate:" + QString::number (rxBitRate) + "kbps\n";
  str += "Mean delay:" + QString::number (meanDelay) + "ms\n";
  str += "Packet Loss ratio:" + QString::number (packetLostRatio * 100) + "%\n\n";


  str += QString ("timeFirstTxPacket= ").leftJustified (justify, ' ') + QString::number (flowStats.timeFirstTxPacket) + "ns\n";
  str += QString ("timeFirstRxPacket= ").leftJustified (justify, ' ') + QString::number (flowStats.timeFirstRxPacket) + "ns\n";
  str += QString ("timeLastTxPacket= ").leftJustified (justify, ' ') + QString::number (flowStats.timeLastTxPacket)   + "ns\n";
  str += QString ("timeLastRxPacket= ").leftJustified (justify, ' ') + QString::number (flowStats.timeLastRxPacket)   + "ns\n";
  str += QString ("delaySum= ").leftJustified (justify + 4, ' ') + QString::number (flowStats.delaySum)               + "ns\n";
  str += QString ("jitterSum= ").leftJustified (justify + 5, ' ') + QString::number (flowStats.jitterSum)             + "ns\n";
  str += QString ("lastDelay= ").leftJustified (justify + 5, ' ') + QString::number (flowStats.delaySum)              + "ns\n";
  str += QString ("txBytes= ").leftJustified (justify + 6, ' ') + QString::number (flowStats.txBytes)                 + "\n";
  str += QString ("rxBytes= ").leftJustified (justify + 6, ' ') + QString::number (flowStats.rxBytes)                 + "\n";
  str += QString ("txPackets= ").leftJustified (justify + 5, ' ') + QString::number (flowStats.txPackets)             + "\n";
  str += QString ("rxPackets= ").leftJustified (justify + 5, ' ') + QString::number (flowStats.rxPackets)             + "\n";
  str += QString ("lostPackets= ").leftJustified (justify + 5, ' ') + QString::number (flowStats.lostPackets)         + "\n";
  str += QString ("timesForwarded= ").leftJustified (justify, ' ') + QString::number (flowStats.timesForwarded)       + "\n";

  if (!flowStats.packetsDropped.empty ())
    {
      str += QString ("\nPackets Dropped:\n\n");
    }
  for (PacketsDroppedReasonVector_t::const_iterator i = flowStats.packetsDropped.begin ();
      i != flowStats.packetsDropped.end ();
      ++i)
    {
      Reason_t reason = *i;
      str += dropReasonToString (reason.reasonCode) + ":" + QString::number (reason.number) + "\n";
    }

  if (!flowStats.bytesDropped.empty ())
    {
      str += QString ("\nBytes Dropped:\n\n");
    }
  for (BytesDroppedReasonVector_t::const_iterator i = flowStats.bytesDropped.begin ();
      i != flowStats.bytesDropped.end ();
      ++i)
    {
      Reason_t reason = *i;
      str += dropReasonToString (reason.reasonCode) + ":" + QString::number (reason.number) + "\n";
    }

  for (HistogramVector_t::const_iterator i = flowStats.histograms.begin ();
      i != flowStats.histograms.end ();
      ++i)
    {
      Histogram_t histogram = *i;
      str += "\n" + histogram.name + " nBins:" + QString::number (histogram.nBins) + "\n";
      for (FlowBinVector_t::const_iterator j = histogram.bins.begin ();
          j != histogram.bins.end ();
          ++j)
        {
          FlowBin_t flowBin = *j;
          str += "Index:" + QString::number (flowBin.index) + " ";
          str += "Start:" + QString::number (flowBin.start) + " ";
          str += "Width:" + QString::number (flowBin.width) + " ";
          str += "Count:" + QString::number (flowBin.count) + "\n";
        }

    }


  return str;
}

QString
FlowMonStatsScene::ipv4ClassifierToString (Ipv4Classifier_t ipv4Classifier)
{
  QString str;
  QString protocol = "";
  if (ipv4Classifier.protocol == 6)
    {
      protocol += "TCP ";
    }
  else if (ipv4Classifier.protocol == 17)
    {
      protocol += "UDP ";
    }
  else
    {
      protocol += "Protocol:" + QString::number (ipv4Classifier.protocol) + " ";
    }
  str += protocol;
  str += ipv4Classifier.sourceAddress + "/" + QString::number (ipv4Classifier.sourcePort);
  str += "---->";
  str += ipv4Classifier.destinationAddress + "/" + QString::number (ipv4Classifier.destinationPort);
  return str;
}

void
FlowMonStatsScene::adjustRect ()
{
  QRectF currentRect = sceneRect ();
  QRectF newRect = QRectF (currentRect.topLeft (), QPointF (currentRect.bottomRight ().x (), m_bottomY));
  setSceneRect (newRect);
}


void
FlowMonStatsScene::addProxyWidgets ()
{
  for (FlowIdFlowStatsMap_t::const_iterator i = m_flowIdFlowStats.begin ();
      i != m_flowIdFlowStats.end ();
      ++i)
    {
      uint32_t flowId = i->first;
      FlowStatsFlow_t flowStats = i->second;
      Ipv4Classifier_t ipv4Classifer = m_flowIdIpv4Classifiers[flowId];
      QString content = ipv4ClassifierToString (ipv4Classifer) + "\n" + flowStatsToString (flowStats);
      TextBubble * tb = new TextBubble ("Flow Id:" + QString::number (flowId) + "\n======", content);
      m_flowIdProxyWidgets[flowId] = addWidget (tb);
      showInfoWidget (false);
    }
  if (m_flowProbes.empty ())
    {
      return;
    }

  QString str;
  uint32_t probeIndex = 0;
  for (FlowProbes_t::const_iterator i = m_flowProbes.begin ();
      i != m_flowProbes.end ();
      ++i, ++probeIndex)
    {
      str += "\nIndex:" + QString::number (probeIndex) + "\n";
      FlowProbe_t probe = *i;
      for (FlowProbe_t::const_iterator j = probe.begin ();
          j != probe.end ();
          ++j)
        {
          FlowProbeFlowStats_t flowStat = *j;
          str += " FlowId:" + QString::number (flowStat.flowId);
          str += " Packets:" + QString::number (flowStat.packets);
          str += " Bytes:" + QString::number (flowStat.bytes);
          str += " DelayFromFirstProbeSum:" + QString::number (flowStat.delayFromFirstProbeSum) + "ns";
          str += "\n";
        }

    }
  TextBubble * tb = new TextBubble ("Flow Probes:", str);
  m_flowProbeWidget = addWidget (tb);


}

void
FlowMonStatsScene::align ()
{
  m_lastX = 0;
  m_lastY = 0;
  m_bottomY = 0;
  qreal currentMaxHeight = 0;

  for (FlowIdProxyWidgetMap_t::const_iterator i = m_flowIdProxyWidgets.begin ();
      i != m_flowIdProxyWidgets.end ();
      ++i)
    {
      QGraphicsProxyWidget * pw = i->second;
      bool flowIsActive = StatsMode::getInstance ()->isNodeActive (i->first);
      pw->setVisible (flowIsActive);
      if (flowIsActive)
        {
          TextBubble * tb = (TextBubble *) pw->widget ();
          QFont f (tb->font ());
          f.setPointSizeF (StatsMode::getInstance ()->getCurrentFontSize ());
          tb->setFont (f);

          QFontMetrics fm (f);
          pw->setMaximumHeight (fm.height () * tb->text ().count ("\n"));
          pw->adjustSize ();

          qreal newX = m_lastX + pw->size ().width ();
          currentMaxHeight = qMax (currentMaxHeight, pw->size ().height ());
          if (newX >= sceneRect ().right ())
            {
              m_lastX = 0;
              m_lastY += currentMaxHeight + INTERSTATS_SPACE;
              currentMaxHeight = 0;
            }
          pw->setPos (m_lastX, m_lastY);
          m_lastX = pw->pos ().x () + pw->size ().width () + INTERSTATS_SPACE;
          m_lastY = pw->pos ().y ();
          m_bottomY = m_lastY + currentMaxHeight;
          adjustRect ();
        }
    }
  if (m_flowProbeWidget)
    {
      QGraphicsProxyWidget * pw = m_flowProbeWidget;
      TextBubble * tb = (TextBubble *) pw->widget ();
      QFont f (tb->font ());
      f.setPointSizeF (StatsMode::getInstance ()->getCurrentFontSize ());
      tb->setFont (f);

      QFontMetrics fm (f);
      pw->setMaximumHeight (fm.height () * tb->text ().count ("\n"));
      pw->adjustSize ();

      qreal newX = m_lastX + pw->size ().width ();
      currentMaxHeight = qMax (currentMaxHeight, pw->size ().height ());
      if (newX >= sceneRect ().right ())
        {
          m_lastX = 0;
          m_lastY += currentMaxHeight + INTERSTATS_SPACE;
          currentMaxHeight = 0;
        }
      pw->setPos (m_lastX, m_lastY);
      m_lastX = pw->pos ().x () + pw->size ().width () + INTERSTATS_SPACE;
      m_lastY = pw->pos ().y ();
      m_bottomY = m_lastY + currentMaxHeight;
      adjustRect ();

    }

}

void
FlowMonStatsScene::reloadContent (bool force)
{
  Q_UNUSED (force);
  if (m_flowIdProxyWidgets.empty ())
    {
      addProxyWidgets ();
    }
  if (m_flowIdProxyWidgets.empty ())
    {
      return;
    }
  align ();
}

uint32_t
FlowMonStatsScene::getNodeCount ()
{
  if (!m_flowIdIpv4Classifiers.empty ())
    {
      uint32_t lastFlowId = m_flowIdIpv4Classifiers.rbegin ()->first + 1;
      return lastFlowId;
    }
  else
    {
      return 0;
    }
}

} // namespace netanim
