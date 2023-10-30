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

#include "animpacket.h"
#include "animnode.h"
#include "animatorview.h"
#include "logqt.h"

#define PI 3.14159265
NS_LOG_COMPONENT_DEFINE ("AnimPacket");

namespace netanim
{
AnimPacketMgr * pAnimPacketMgr = 0;

AnimPacket::AnimPacket (uint32_t fromNodeId,
                        uint32_t toNodeId,
                        qreal firstBitTx,
                        qreal firstBitRx,
                        qreal lastBitTx,
                        qreal lastBitRx,
                        bool isWPacket,
                        QString metaInfo,
                        bool showMetaInfo,
                        uint8_t numWirelessSlots):
  m_fromNodeId (fromNodeId),
  m_toNodeId (toNodeId),
  m_firstBitTx (firstBitTx),
  m_firstBitRx (firstBitRx),
  m_lastBitTx (lastBitTx),
  m_lastBitRx (lastBitRx),
  m_isWPacket (isWPacket),
  m_infoText (0),
  m_numWirelessSlots (numWirelessSlots),
  m_currentWirelessSlot (0)
{
  m_fromPos = AnimNodeMgr::getInstance ()->getNode (fromNodeId)->getCenter ();
  m_toPos = AnimNodeMgr::getInstance ()->getNode (toNodeId)->getCenter ();
  //NS_LOG_DEBUG ("FromPos:" << m_fromPos);
  //NS_LOG_DEBUG ("ToPos:" << m_toPos);
  m_line = QLineF (m_fromPos, m_toPos);
  qreal propDelay = m_firstBitRx - m_firstBitTx;
  m_velocity = m_line.length ()/propDelay;
  m_cos = cos ((360 - m_line.angle ()) * PI/180);
  m_sin = sin ((360 - m_line.angle ()) * PI/180);
  setVisible(false);
  setZValue(ANIMPACKET_ZVAVLUE);

  m_infoText = new QGraphicsSimpleTextItem (this);
  if(showMetaInfo)
    {
      m_infoText->setText ("p");
      m_infoText->setFlag (QGraphicsItem::ItemIgnoresTransformations);


      qreal textAngle = m_line.angle ();
      if(textAngle < 90)
        {
          textAngle = 360-textAngle;
        }
      else if (textAngle > 270)
        {
          textAngle = 360-textAngle;
        }
      else
        {
          textAngle = 180-textAngle;
        }
      m_infoText->setTransform (QTransform().rotate (textAngle));
      m_infoText->setText (getMeta(metaInfo));
    }

}

AnimPacket::~AnimPacket ()
{
  if(m_infoText)
    {
      delete m_infoText;
    }
}

QString
AnimPacket::getMeta (QString metaInfo, int filter, bool & result, bool shortString)
{
  result = false;
  QString metaInfoString = GET_DATA (metaInfo);

  bool aodvResult = false;
  AodvInfo aodvInfo = parseAodv (metaInfoString, aodvResult);
  bool olsrResult = false;
  OlsrInfo olsrInfo = parseOlsr (metaInfoString, olsrResult);
  bool dsdvResult = false;
  DsdvInfo dsdvInfo = parseDsdv (metaInfoString, dsdvResult);
  bool tcpResult = false;
  TcpInfo tcpInfo = parseTcp (metaInfoString, tcpResult);
  bool udpResult = false;
  UdpInfo udpInfo = parseUdp (metaInfoString, udpResult);
  bool arpResult = false;
  ArpInfo arpInfo = parseArp (metaInfoString, arpResult);
  bool icmpResult = false;
  IcmpInfo icmpInfo = parseIcmp (metaInfoString, icmpResult);
  bool ipv4Result = false;
  Ipv4Info ipv4Info = parseIpv4 (metaInfoString, ipv4Result);
  bool ipv6Result = false;
  Ipv6Info ipv6Info = parseIpv6 (metaInfoString, ipv6Result);

  bool wifiResult = false;
  WifiMacInfo wifiMacInfo = parseWifi (metaInfoString, wifiResult);
  bool pppResult = false;
  PppInfo pppInfo = parsePpp (metaInfoString, pppResult);
  bool ethernetResult = false;
  EthernetInfo ethernetInfo = parseEthernet (metaInfoString, ethernetResult);

  QString finalString = "";
  if (filter == AnimPacket::ALL)
    {
      if (shortString)
        {
          finalString += aodvInfo.toShortString () +
                         olsrInfo.toShortString () +
                         dsdvInfo.toShortString () +
                         tcpInfo.toShortString () +
                         udpInfo.toShortString () +
                         icmpInfo.toShortString () +
                         ipv4Info.toShortString () +
                         ipv6Info.toShortString () +
                         arpInfo.toShortString () +
                         wifiMacInfo.toShortString () +
                         pppInfo.toShortString () +
                         ethernetInfo.toShortString ();
        }
      else
        {
          finalString += aodvInfo.toString() +
                         olsrInfo.toString () +
                         dsdvInfo.toString () +
                         tcpInfo.toString () +
                         udpInfo.toString () +
                         icmpInfo.toString () +
                         ipv4Info.toString () +
                         ipv6Info.toString () +
                         arpInfo.toString () +
                         wifiMacInfo.toString () +
                         pppInfo.toString () +
                         ethernetInfo.toString ();
        }
      result = true;
    }
  else
    {
      if (filter & AnimPacket::AODV)
        if (aodvResult)
          finalString += (shortString)?aodvInfo.toShortString ():aodvInfo.toString ();
      if (filter & AnimPacket::OLSR)
        if (olsrResult)
          finalString += (shortString)?olsrInfo.toShortString ():olsrInfo.toString ();
      if (filter & AnimPacket::DSDV)
        if (dsdvResult)
          finalString += (shortString)?dsdvInfo.toShortString ():dsdvInfo.toString ();
      if (filter & AnimPacket::TCP)
        if (tcpResult)
          finalString += (shortString)?tcpInfo.toShortString ():tcpInfo.toString ();
      if (filter & AnimPacket::UDP)
        if (udpResult)
          finalString += (shortString)?udpInfo.toShortString ():udpInfo.toString ();
      if (filter & AnimPacket::ICMP)
        if (icmpResult)
          finalString += (shortString)?icmpInfo.toShortString ():icmpInfo.toString ();
      if (filter & AnimPacket::IPV4)
        if (ipv4Result)
          finalString += (shortString)?ipv4Info.toShortString ():ipv4Info.toString ();
      if (filter & AnimPacket::IPV6)
        if (ipv6Result)
          finalString += (shortString)?ipv6Info.toShortString ():ipv6Info.toString ();
      if (filter & AnimPacket::ARP)
        if (arpResult)
          finalString += (shortString)?arpInfo.toShortString ():arpInfo.toString ();
      if (filter & AnimPacket::WIFI)
        if (wifiResult)
          finalString += (shortString)?wifiMacInfo.toShortString ():wifiMacInfo.toString ();
      if (filter & AnimPacket::PPP)
        if (pppResult)
          finalString += (shortString)?pppInfo.toShortString ():pppInfo.toString ();
      if (filter & AnimPacket::ETHERNET)
        if (ethernetResult)
          finalString += (shortString)?ethernetInfo.toShortString ():ethernetInfo.toString ();
      if (finalString != "")
        result = true;

    }
  return finalString;

}

QString
AnimPacket::getMeta (QString metaInfo, bool shortString)
{
  bool result = false;
  QString metaInfoString = GET_DATA (metaInfo);


  AodvInfo aodvInfo = parseAodv (metaInfoString, result);
  if (result)
    {
      return (shortString)?aodvInfo.toShortString ():aodvInfo.toString ();
    }


  result = false;
  OlsrInfo olsrInfo = parseOlsr (metaInfoString, result);
  if(result)
    {
      return (shortString)?olsrInfo.toShortString ():olsrInfo.toString ();
    }


  result = false;
  DsdvInfo dsdvInfo = parseDsdv (metaInfoString, result);
  if(result)
    {
      return (shortString)?dsdvInfo.toShortString ():dsdvInfo.toString ();
    }



  result = false;
  TcpInfo tcpInfo = parseTcp (metaInfoString, result);
  if(result)
    {
      return (shortString)?tcpInfo.toShortString ():tcpInfo.toString ();
    }



  result = false;
  UdpInfo udpInfo = parseUdp (metaInfoString, result);
  if(result)
    {
      return (shortString)?udpInfo.toShortString ():udpInfo.toString ();
    }



  result = false;
  ArpInfo arpInfo = parseArp (metaInfoString, result);
  if(result)
    {
      return (shortString)?arpInfo.toShortString ():arpInfo.toString ();
    }




  result = false;
  IcmpInfo icmpInfo = parseIcmp (metaInfoString, result);
  if(result)
    {
      return (shortString)?icmpInfo.toShortString ():icmpInfo.toString ();

    }


  result = false;
  Ipv4Info ipv4Info = parseIpv4 (metaInfoString, result);
  if(result)
    {
      return (shortString)?ipv4Info.toShortString ():ipv4Info.toString ();
    }


  result = false;
  WifiMacInfo wifiMacInfo = parseWifi (metaInfoString, result);
  if(result)
    {
      return (shortString)?wifiMacInfo.toShortString ():wifiMacInfo.toString ();
    }


  result = false;
  PppInfo pppInfo = parsePpp (metaInfoString, result);
  if(result)
    {
      return (shortString)?pppInfo.toShortString ():pppInfo.toString ();
    }

  result = false;
  EthernetInfo ethernetInfo = parseEthernet (metaInfoString, result);
  if(result)
    {
      return (shortString)?ethernetInfo.toShortString ():ethernetInfo.toString ();
    }
  return "";

}

uint32_t
AnimPacket::getFromNodeId ()
{
  return m_fromNodeId;
}

uint32_t
AnimPacket::getToNodeId ()
{
  return m_toNodeId;
}

qreal
AnimPacket::getFirstBitTx ()
{
  return m_firstBitTx;
}

qreal
AnimPacket::getFirstBitRx ()
{
  return m_firstBitRx;
}


qreal
AnimPacket::getLastBitRx ()
{
  return m_lastBitRx;
}

qreal
AnimPacket::getLastBitTx ()
{
  return m_lastBitTx;
}

bool
AnimPacket::getIsWPacket ()
{
  return m_isWPacket;
}

qreal
AnimPacket::getRadius ()
{
  QLineF l (m_fromPos, m_toPos);
  return l.length ();
}

bool
AnimPacket::packetExpired ()
{
  return m_currentWirelessSlot >= m_numWirelessSlots;
}

QGraphicsSimpleTextItem *
AnimPacket::getInfoTextItem ()
{
  return m_infoText;
}



PppInfo
AnimPacket::parsePpp(QString metaInfo, bool & result)
{
  PppInfo pppInfo;
  QRegularExpression rx("ns3::PppHeader.*");
  QRegularExpressionMatch match = rx.match(metaInfo);
  result = match.hasMatch();
  return pppInfo;
}


ArpInfo
AnimPacket::parseArp (QString metaInfo, bool & result)
{
  ArpInfo arpInfo;

  QRegularExpression rx ("ns3::ArpHeader\\s+\\((request|reply) source mac: ..-..-(..:..:..:..:..:..) source ipv4: (\\S+) (?:dest mac: ..-..-)?(..:..:..:..:..:.. )?dest ipv4: (\\S+)\\)");
  QRegularExpressionMatch match = rx.match(metaInfo);
  if (!match.hasMatch())
    {
      result = false;
      return arpInfo;
    }

  arpInfo.type = GET_DATA (match.captured (1));
  arpInfo.sourceMac = GET_DATA (match.captured (2));
  arpInfo.sourceIpv4 = GET_DATA (match.captured (3));
  if(QString (GET_DATA (match.captured (4))) != "")
    arpInfo.destMac = GET_DATA (match.captured (4));
  arpInfo.destIpv4  = GET_DATA (match.captured (5));
  result = true;
  return arpInfo;
  /*qDebug (" Type:" + arpInfo->type +
          " SMac:" + arpInfo->sourceMac +
          " SIp:"  + arpInfo->sourceIpv4 +
          " DMac:" + arpInfo->destMac +
          " DIp:"  + arpInfo->destIpv4);*/


}

EthernetInfo
AnimPacket::parseEthernet (QString metaInfo, bool & result)
{
  EthernetInfo ethernetInfo;
  QRegularExpression rx ("ns3::EthernetHeader \\( length/type\\S+ source=(..:..:..:..:..:..), destination=(..:..:..:..:..:..)\\)");
  QRegularExpressionMatch match = rx.match(metaInfo);
  if (!match.hasMatch())
    {
      result = false;
      return ethernetInfo;
    }

  ethernetInfo.sourceMac = GET_DATA (match.captured (1));
  ethernetInfo.destMac = GET_DATA (match.captured (2));

  result = true;
  return ethernetInfo;
}


IcmpInfo
AnimPacket::parseIcmp (QString metaInfo, bool & result)
{
  IcmpInfo icmpInfo;

  QRegularExpression rx ("ns3::Icmpv4Header \\(type=(.*), code=([^\\)]*)");
  QRegularExpressionMatch match = rx.match(metaInfo);
  if (!match.hasMatch())
    {
      result = false;
      return icmpInfo;
    }

  icmpInfo.type =  GET_DATA (match.captured (1));
  icmpInfo.code =  GET_DATA (match.captured (2));
  result = true;
  return icmpInfo;

}

UdpInfo
AnimPacket::parseUdp (QString metaInfo, bool & result)
{
  UdpInfo udpInfo;

  QRegularExpression rx ("ns3::UdpHeader \\(length: (\\S+) (\\S+) > (\\S+)\\)");
  QRegularExpressionMatch match = rx.match(metaInfo);
  if (!match.hasMatch())
    {
      result = false;
      return udpInfo;
    }

  udpInfo.length = GET_DATA (match.captured (1));
  udpInfo.SPort = GET_DATA (match.captured (2));
  udpInfo.DPort = GET_DATA (match.captured (3));
  result = true;
  return udpInfo;
}

Ipv6Info
AnimPacket::parseIpv6 (QString metaInfo, bool & result)
{
  Ipv6Info ipv6Info;
  QRegularExpression rx ("ns3::Ipv6Header");
  QRegularExpressionMatch match = rx.match(metaInfo);
  result = match.hasMatch();
  return ipv6Info;
}

Ipv4Info
AnimPacket::parseIpv4 (QString metaInfo, bool & result)
{
  Ipv4Info ipv4Info;

  QRegularExpression rx ("ns3::Ipv4Header \\(tos (\\S+) DSCP (\\S+) ECN (\\S+) ttl (\\d+) id (\\d+) protocol (\\d+) .* length: (\\d+) (\\S+) > (\\S+)\\)");
  QRegularExpressionMatch match = rx.match(metaInfo);
  if (!match.hasMatch())
    {
      result = false;
      return ipv4Info;
    }
  ipv4Info.tos = GET_DATA (match.captured (1));
  ipv4Info.Dscp = GET_DATA (match.captured (2));
  ipv4Info.Ecn = GET_DATA (match.captured (3));
  ipv4Info.Ttl = GET_DATA (match.captured (4));
  ipv4Info.Id = GET_DATA (match.captured (5));
  ipv4Info.protocol = GET_DATA (match.captured (6));
  ipv4Info.length = GET_DATA (match.captured (7));
  ipv4Info.SrcIp = GET_DATA (match.captured (8));
  ipv4Info.DstIp = GET_DATA (match.captured (9));
  result = true;
  return ipv4Info;
}

TcpInfo
AnimPacket::parseTcp (QString metaInfo, bool & result)
{
  TcpInfo tcpInfo;

  QRegularExpression rx ("ns3::TcpHeader \\((\\d+) > (\\d+) \\[([^\\]]*)\\] Seq=(\\d+) Ack=(\\d+) Win=(\\d+)");
  QRegularExpressionMatch match = rx.match(metaInfo);
  if (!match.hasMatch())
    {
      result = false;
      return tcpInfo;
    }
  tcpInfo.SPort = GET_DATA (match.captured (1));
  tcpInfo.DPort = GET_DATA (match.captured (2));
  tcpInfo.flags = GET_DATA (match.captured (3));
  tcpInfo.seq = GET_DATA (match.captured (4));
  tcpInfo.ack = GET_DATA (match.captured (5));
  tcpInfo.window = GET_DATA (match.captured (6));
  result = true;
  return tcpInfo;
}

WifiMacInfo
AnimPacket::parseWifi (QString metaInfo, bool & result)
{
  QRegularExpression rxCTL_ACK ("ns3::WifiMacHeader \\(CTL_ACK .*RA=(..:..:..:..:..:..)");
  QRegularExpressionMatch match_ACK = rxCTL_ACK.match(metaInfo);
  WifiMacInfo wifiMacInfo;
  if (match_ACK.hasMatch())
    {
      wifiMacInfo.type = "CTL_ACK";
      wifiMacInfo.Ra = GET_DATA (match_ACK.captured(1));
      result = true;
      return wifiMacInfo;

    }
  QRegularExpression rxCTL_RTS ("ns3::WifiMacHeader \\(CTL_RTS .*RA=(..:..:..:..:..:..), TA=(..:..:..:..:..:..)");
  QRegularExpressionMatch match_RTS = rxCTL_RTS.match(metaInfo);
  if (match_RTS.hasMatch())
    {
      wifiMacInfo.type = "CTL_RTS";
      wifiMacInfo.Ra = GET_DATA (match_RTS.captured (1));
      wifiMacInfo.Sa = GET_DATA (match_RTS.captured (2));
      result = true;
      return wifiMacInfo;

    }

  QRegularExpression rxCTL_CTS ("ns3::WifiMacHeader \\(CTL_CTS .*RA=(..:..:..:..:..:..)");
  QRegularExpressionMatch match_CTS = rxCTL_CTS.match(metaInfo);
  if (match_CTS.hasMatch())
    {
      wifiMacInfo.type = "CTL_CTS";
      wifiMacInfo.Ra = GET_DATA (match_CTS.captured (1));
      result = true;
      return wifiMacInfo;

    }

  QRegularExpression rx ("ns3::WifiMacHeader \\((\\S+) ToDS=(0|1), FromDS=(0|1), .*DA=(..:..:..:..:..:..), SA=(..:..:..:..:..:..), BSSID=(..:..:..:..:..:..)");
  QRegularExpressionMatch match = rx.match(metaInfo);
  if (!match.hasMatch())
    {
      result = false;
      return wifiMacInfo;
    }
  wifiMacInfo.type = GET_DATA (match.captured (1));
  wifiMacInfo.toDs = GET_DATA (match.captured (2));
  wifiMacInfo.fromDs = GET_DATA (match.captured (3));
  wifiMacInfo.Da = GET_DATA (match.captured (4));
  wifiMacInfo.Sa = GET_DATA (match.captured (5));
  wifiMacInfo.Bssid = GET_DATA (match.captured (6));

  if(wifiMacInfo.type == "MGT_ASSOCIATION_REQUEST")
    {
      QRegularExpression rx ("ns3::MgtAssocRequestHeader \\(ssid=(\\S+),");
      QRegularExpressionMatch match = rx.match(metaInfo);
      if (!match.hasMatch())
        {
          result = false;
          return wifiMacInfo;
        }
      wifiMacInfo.SSid = GET_DATA (match.captured (1));
    }
  if(wifiMacInfo.type == "MGT_ASSOCIATION_RESPONSE")
    {
      QRegularExpression rx ("ns3::MgtAssocResponseHeader \\(status code=(\\S+), rates");
      QRegularExpressionMatch match = rx.match(metaInfo);
      if (!match.hasMatch())
        {
          result = false;
          return wifiMacInfo;
        }
      wifiMacInfo.assocResponseStatus = GET_DATA (match.captured (1));
    }
  result = true;
  return wifiMacInfo;
}

AodvInfo
AnimPacket::parseAodv (QString metaInfo, bool & result)
{
  AodvInfo aodvInfo;

  QRegularExpression rx ("ns3::aodv::TypeHeader \\((\\S+)\\) ");
  QRegularExpressionMatch match = rx.match(metaInfo);
  if (!match.hasMatch())
    {
      result = false;
      return aodvInfo;
    }
  aodvInfo.type = GET_DATA (match.captured (1));
  if(aodvInfo.type == "RREQ")
    {
      QRegularExpression rx ("ns3::aodv::RreqHeader \\(RREQ ID \\d+ destination: ipv4 (\\S+) sequence number (\\d+) source: ipv4 (\\S+) sequence number \\d+");
      QRegularExpressionMatch match = rx.match(metaInfo);
      if (!match.hasMatch())
        {
          result = false;
          return aodvInfo;
        }
      aodvInfo.destination = GET_DATA (match.captured (1));
      aodvInfo.seq = GET_DATA (match.captured (2));
      aodvInfo.source = GET_DATA (match.captured (3));

    }
  if(aodvInfo.type == "RREP")
    {
      QRegularExpression rx ("ns3::aodv::RrepHeader \\(destination: ipv4 (\\S+) sequence number (\\d+) source ipv4 (\\S+) ");
      QRegularExpressionMatch match = rx.match(metaInfo);
      if (!match.hasMatch())
        {
          result = false;
          return aodvInfo;
        }
      aodvInfo.destination = GET_DATA (match.captured (1));
      aodvInfo.seq = GET_DATA (match.captured (2));
      aodvInfo.source = GET_DATA (match.captured (3));
    }
  if(aodvInfo.type == "RERR")
    {
      QRegularExpression rx ("ns3::aodv::RerrHeader \\(([^\\)]+) \\(ipv4 address, seq. number):(\\S+) ");
      QRegularExpressionMatch match = rx.match(metaInfo);
      if (!match.hasMatch())
        {
          result = false;
          return aodvInfo;
        }
      aodvInfo.rerrInfo = GET_DATA (match.captured (1));
      aodvInfo.destination = GET_DATA (match.captured (2));
    }
  result = true;
  return aodvInfo;

}

DsdvInfo
AnimPacket::parseDsdv (QString metaInfo, bool & result)
{
  DsdvInfo dsdvInfo;

  QRegularExpression rx ("ns3::dsdv::DsdvHeader");
  QRegularExpressionMatch match = rx.match(metaInfo);

  result = match.hasMatch();
  return dsdvInfo;

}

OlsrInfo
AnimPacket::parseOlsr (QString metaInfo, bool & result)
{
  OlsrInfo olsrInfo;

  QRegularExpression rx ("ns3::olsr::MessageHeader");
  QRegularExpressionMatch match = rx.match(metaInfo);

  result = match.hasMatch();
  return olsrInfo;
}







#if 0
void
AnimPacket::update (qreal t)
{
  qreal timeElapsed = t - getFirstBitTx ();
  qreal distanceTravelled = m_velocity * timeElapsed;
  m_distanceTraveled = distanceTravelled;
  qreal x = m_distanceTraveled * m_cos;
  qreal y = m_distanceTraveled * m_sin;
  m_head = QPointF (m_fromPos.x () + x,  m_fromPos.y () + y);
  //NS_LOG_DEBUG ("Upd Time:" << t << " Head:" << m_head << " Distance traveled:" << m_distanceTraveled << " time elapsed:" << timeElapsed  << " velocity:" << m_velocity);
}

#else
void
AnimPacket::update (qreal t)
{
  //NS_LOG_DEBUG ("Updating");
  m_currentTime = t;
  //qreal midPointX = (m_toPos.x () + m_fromPos.x ())/2;
  //qreal midPointY = (m_toPos.y () + m_fromPos.y ())/2;
  if (m_isWPacket)
    {
      //m_head = QPointF (midPointX, midPointY);
      m_head = m_toPos;
    }
  else
    {
      if (t > getFirstBitRx ())
        {
          m_head = m_toPos;
        }
      else
        {
          qreal timeElapsed = t - getFirstBitTx ();
          qreal distanceTravelled = m_velocity * timeElapsed;
          qreal x = distanceTravelled * m_cos;
          qreal y = distanceTravelled * m_sin;
          m_head = QPointF (m_fromPos.x () + x, m_fromPos.y () + y);
        }

    }
  //m_head = QPointF (100,100);
  //NS_LOG_DEBUG ("m_head:" << m_head);
}

#endif
QRectF
AnimPacket::boundingRect () const
{
  return m_boundingRect;
  //QRectF r = QRectF(-m_boundingRect.width(),  -m_boundingRect.height(), m_boundingRect.width(), m_boundingRect.height());
  //return r;
}


void
AnimPacket::paint (QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
  Q_UNUSED(option)
  Q_UNUSED(widget)
  //NS_LOG_DEBUG ("Packet Transform:" << transform());
  //NS_LOG_DEBUG ("Device Transform:" << painter->deviceTransform());
  //NS_LOG_DEBUG ("Scene Transform:" << sceneTransform());
  QPen p;
  QTransform viewTransform = AnimatorView::getInstance ()->transform ();
  painter->save ();
  QPainterPath arrowTailPath;
  arrowTailPath.moveTo (0, 0);
  qreal mag = getRadius ();

  qreal transformedMag = 2 * (10/viewTransform.m22 ());

  if (!m_isWPacket)
    {
      qreal timeElapsed = m_currentTime - getFirstBitTx ();
      mag = m_velocity * timeElapsed;
      qreal magLbTx = 0;
      //NS_LOG_DEBUG ("First Bit Mag:" << mag);
      if (m_currentTime > getLastBitTx ())
        {
          timeElapsed = m_currentTime - getLastBitTx ();
          magLbTx = m_velocity * timeElapsed;
        }
      if (m_currentTime > getFirstBitRx ())
        {
          qreal fullDistance = m_velocity * (getFirstBitRx () - getFirstBitTx ());
          mag = fullDistance;

        }
      mag -= magLbTx;
      //NS_LOG_DEBUG ("Mag:" << mag << " MagLbTx:" << magLbTx);
      arrowTailPath.lineTo (-mag, 0);
      arrowTailPath.addEllipse( QPointF (-mag, 0), transformedMag/10, transformedMag/10);
    }
  else
    {
      arrowTailPath.lineTo (-mag , 0);

    }
  //arrowTailPath.addText (-mag, 0, f, "HOLA");
  p.setColor (Qt::blue);
  //p.setWidthF (0.75);
  p.setCosmetic (true);
  painter->setPen (p);
  painter->rotate (360 - m_line.angle ());
  painter->drawPath (arrowTailPath);
  painter->restore ();



  QPolygonF arrowHeadPolygon;

  QPainterPath arrowHeadPath;
  qreal arrowHeadLength = transformedMag;

  arrowHeadPolygon << QPointF (0, 0)
                   << QPointF (-arrowHeadLength * cos (PI/10), -arrowHeadLength * sin (PI/10))
                   << QPointF (-(arrowHeadLength/2) * cos (PI/10), 0)
                   << QPointF (-arrowHeadLength * cos (PI/10), arrowHeadLength * sin (PI/10));

  arrowHeadPath.lineTo (-arrowHeadLength * cos (PI/10), -arrowHeadLength * sin (PI/10));
  arrowHeadPath.moveTo (0, 0);
  arrowHeadPath.lineTo (-arrowHeadLength * cos (PI/10), arrowHeadLength * sin (PI/10));
  arrowHeadPath.moveTo (0, 0);

  arrowHeadPath.moveTo (0, 0);
  painter->save();
  QPen arrowHeadPen;
  arrowHeadPen.setCosmetic (true);
  //arrowHeadPen.setWidthF (0.75);

  QColor black (0, 0, 5, 130);
  arrowHeadPen.setColor (black);

  painter->setPen(arrowHeadPen);
  painter->rotate (360 - m_line.angle ());
  QBrush brush;
  brush.setColor (black);
  brush.setStyle (Qt::SolidPattern);
  painter->setBrush (brush);
  painter->drawPolygon (arrowHeadPolygon);
  painter->restore ();




  QPainterPath path;
  path.moveTo (0, 0);
  path.addPath (arrowHeadPath);
  //path.moveTo (0, 0);
  path.addPath (arrowTailPath);


  m_boundingRect = path.boundingRect ();
  QTransform t;
  t.rotate(360 - m_line.angle ());
  m_boundingRect = t.mapRect (m_boundingRect);





  painter->save ();
  QTransform textTransform;
  qreal textAngle = m_line.angle ();
  if(textAngle < 90)
    {
      textAngle = 360-textAngle;
    }
  else if (textAngle > 270)
    {
      textAngle = 360-textAngle;
    }
  else
    {
      textAngle = 180-textAngle;
    }
  textTransform.rotate (textAngle);
  QPainterPath textPath;


  QPen p1 = painter->pen ();
  p1.setColor (Qt::red);
  p1.setStyle (Qt::SolidLine);
  QBrush brush2 = painter->brush ();
  brush2.setStyle (Qt::SolidPattern);
  brush2.setColor (Qt::black);
  p1.setWidthF (1/viewTransform.m22 ());
  p1.setBrush (brush2);
  painter->setBrush (brush2);
  painter->setPen (p1);
  painter->setTransform (m_infoText->transform ());
  QRectF textBoundingRect = textTransform.mapRect (textPath.boundingRect ());

  painter->restore ();
  m_boundingRect = QRectF (QPointF(qMin (m_boundingRect.left (), textBoundingRect.left ()),
                                  qMin (m_boundingRect.top (), textBoundingRect.top ())),
                          QPointF(qMax (m_boundingRect.right (), textBoundingRect.right ()),
                                  qMax (m_boundingRect.bottom (), textBoundingRect.bottom ())));





  QPointF p_1;// = mapFromScene(m_line.p1());
  QPointF p_2;// = mapFromScene(m_line.p2());
  p_1 = mapFromScene(m_line.p1());
  p_2 = mapFromScene(m_line.p2());

  bool leftToRight = false;
  if (p_1.x () < p_2.x ())
    {
      leftToRight = true;

    }

  if (m_isWPacket)
    {
      if (leftToRight)
        {
          QTransform tempTransform;
          tempTransform.rotate(360 - m_line.angle());
          m_infoText->setPos(tempTransform.map(QPointF(-mag * 3/4, 0)));
        }
      else
        {
          QTransform tempTransform;
          tempTransform.rotate(360 - m_line.angle());

          m_infoText->setPos(tempTransform.map(QPointF(-mag / 4, 0)));
        }
    }
  else
    {
      if (leftToRight)
        {
          QTransform tempTransform;
          tempTransform.rotate(360 - m_line.angle());
          m_infoText->setPos(tempTransform.map(QPointF(-mag, 0)));
        }
      else
        {
          QTransform tempTransform;
          tempTransform.rotate(360 - m_line.angle());

          m_infoText->setPos(tempTransform.map(QPointF(0, 0)));
        }
    }


}


QPointF
AnimPacket::getHead ()
{
  return m_head;
}

QPointF
AnimPacket::getFromPos ()
{
  return m_fromPos;
}

QPointF
AnimPacket::getToPos ()
{
  return m_toPos;
}

AnimPacketMgr::AnimPacketMgr ()
{
}
AnimPacketMgr *
AnimPacketMgr::getInstance ()
{
  if(!pAnimPacketMgr)
    {
      pAnimPacketMgr = new AnimPacketMgr;
    }
  return pAnimPacketMgr;
}

AnimPacket *
AnimPacketMgr::add (uint32_t fromId,
                    uint32_t toId,
                    qreal fbTx,
                    qreal fbRx,
                    qreal lbTx,
                    qreal lbRx,
                    bool isWPacket,
                    QString metaInfo,
                    bool showMetaInfo,
                    uint8_t numWirelessSlots)
{
  AnimPacket * pkt = new AnimPacket (fromId, toId, fbTx, fbRx, lbTx, lbRx, isWPacket, metaInfo, showMetaInfo, numWirelessSlots);
  return pkt;
}



}

