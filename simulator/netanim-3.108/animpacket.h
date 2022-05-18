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

#ifndef ANIMPACKET_H
#define ANIMPACKET_H
#include "common.h"
#include "animatorconstants.h"
#include "common.h"
#include "timevalue.h"
#include "animevent.h"
namespace netanim
{


class AnimWirelessCircles : public QObject, public QGraphicsEllipseItem
{
  Q_OBJECT
  Q_PROPERTY (QRectF rect READ rect WRITE setRect)

};


struct ArpInfo
{
  ArpInfo ()
  {
    type = "null";
    sourceMac = "null";
    sourceIpv4 = "null";
    destMac = "ff:ff:ff:ff:ff:ff";
    destIpv4 = "null";
  }
  QString toString ()
  {
    return  " Arp "     + type       +
            " SMac: "   + sourceMac  +
            " DMac: "   + destMac    +
            " SrcIp : " + sourceIpv4 +
            " DstIp : " + destIpv4;
  }
  QString toShortString ()
  {
    return QString ("Arp:") + type + " DstIP=" + destIpv4;
  }
  QString type;
  QString sourceMac;
  QString sourceIpv4;
  QString destMac;
  QString destIpv4;
};

struct PppInfo
{
  QString toString ()
  {
    return " PPP";
  }
  QString toShortString ()
  {
    return "PPP";
  }

};

struct EthernetInfo
{
  EthernetInfo ()
  {
    sourceMac = "null";
    destMac = "null";
  }
  QString toString ()
  {
    return  " Ethernet SMac: " + sourceMac +
            " DMac: "          + destMac;
  }
  QString toShortString ()
  {
    return "Ethernet:" + sourceMac + " > " + destMac;
  }
  QString sourceMac;
  QString destMac;
};


struct WifiMacInfo
{
  WifiMacInfo ()
  {
    type = "null";
    toDs = "null";
    fromDs = "null";
    Da = "null";
    Sa = "null";
    Bssid = "null";
    Ra = "null";
    SSid = "null";
    assocResponseStatus = "null";

  }

  QString toString ()
  {
    if (type == "CTL_ACK")
      return " Wifi CTL_ACK RA:" + Ra;
    if (type == "CTL_RTS")
      return " Wifi CTL_RTS RA:" + Ra + " TA:" + Sa;
    if (type == "CTL_CTS")
      return " Wifi CTL_CTS RA:" + Ra;
    QString temp = " Wifi " + type +
                   " FromDS: " + fromDs +
                   " toDS: " + toDs +
                   " DA: " + Da +
                   " SA: " + Sa +
                   " BSSId: " + Bssid;
    if (type == "MGT_ASSOCIATION_REQUEST")
      temp += " SSid: " + SSid;

    if (type == "MGT_ASSOCIATION_RESPONSE")
      temp += " status: " + assocResponseStatus;
    return temp;
  }

  QString toShortString ()
  {
    QString s = "";
    if (type == "CTL_RTS")
      s = "Wifi:CTL_RTS RA:" + Ra + " TA:" + Sa;
    if (type == "CTL_CTS")
      s = "Wifi:CTL_CTS RA:" + Ra;
    if (type == "MGT_BEACON")
      s =  "Wifi:BEACON ssid" + SSid;
    if (type == "MGT_ASSOCIATION_REQUEST")
      s =  "Wifi:ASSOC_REQ ssid" + SSid;
    if (type == "CTL_ACK")
      s = "Wifi:CTL_ACK RA:" + Ra;
    else
      s = "Wifi:" + type;
    return s;


  }
  QString type;
  QString toDs;
  QString fromDs;
  QString Da;
  QString Sa;
  QString Bssid;
  QString Ra;
  QString SSid;
  QString assocResponseStatus;
};


struct Ipv4Info
{
  Ipv4Info ()
  {

  }
  QString toString ()
  {
    return  " Ipv4 Proto:" + protocol +
            " SrcIp: " + SrcIp +
            " DstIp: " + DstIp;
  }
  QString toShortString ()
  {
    return "IPv4:" + SrcIp + " > " + DstIp;
  }
  QString tos;
  QString Dscp;
  QString Ecn;
  QString Ttl;
  QString Id;
  QString protocol;
  QString length;
  QString SrcIp;
  QString DstIp;

};



struct Ipv6Info
{
  Ipv6Info ()
  {

  }
  QString toString ()
  {
    return  " Ipv6";
  }
  QString toShortString ()
  {
    return "IPv6";
  }

};


struct IcmpInfo
{
  IcmpInfo ()
  {

  }
  QString toString ()
  {
    QString temp;
    temp += "ICMP type: " + type +
            "code: " + code;
    if (type == "3" && code == "3")
      temp += " DstUnreachable";
    return temp;

  }
  QString toShortString ()
  {
    if ( (type == "3") & (code == "3"))
      {
        return "ICMP: Dst Unreachable";
      }
    return "ICMP: type=" + type + " code="+ code;
  }
  QString type;
  QString code;
};


struct UdpInfo
{
  UdpInfo ()
  {

  }
  QString toString ()
  {
    return " UDP " + SPort + " > " + DPort;

  }
  QString toShortString ()
  {
    return "UDP:" + SPort + " > " + DPort;

  }
  QString length;
  QString SPort;
  QString DPort;

};

struct TcpInfo
{
  TcpInfo ()
  {

  }
  QString toString ()
  {
    return " TCP " + SPort + " > " + DPort +
           " " + flags +  " Seq=" + seq   +
           " Ack=" + ack + " Win=" + window;

  }
  QString toShortString ()
  {
    return  "TCP:[" + flags + "]" + " S=" + seq +
        " A=" + ack;
  }
  QString SPort;
  QString DPort;
  QString flags;
  QString seq;
  QString ack;
  QString window;

};

struct AodvInfo
{
  AodvInfo ()
  {

  }
  QString toString ()
  {
    if (type == "RERR")
      {
        return "RERR:" + rerrInfo + " " + destination;
      }
    return "AODV:" + type + " D=" + destination + " S=" + source + " Seq=" + seq;
  }
  QString toShortString ()
  {
    if (type == "RERR")
      {
        return "RERR:" + rerrInfo + " " + destination;
      }
    return "AODV:" + type + " D=" + destination + " S=" + source + " Seq=" + seq;
  }
  QString type;
  QString destination;
  QString source;
  QString seq;
  QString rerrInfo;
};

struct DsdvInfo
{
  DsdvInfo ()
  {

  }
  QString toString ()
  {
    return "DSDV";
  }
  QString toShortString ()
  {
    return "DSDV";
  }
};

struct OlsrInfo
{
  OlsrInfo ()
  {

  }
  QString toString ()
  {
    return "OLSR";
  }
  QString toShortString ()
  {
    return "OLSR";
  }
};



class AnimPacket : public QGraphicsObject
{
  Q_OBJECT
  Q_PROPERTY (QPointF pos READ pos WRITE setPos)
public:
  AnimPacket (uint32_t fromNodeId,
             uint32_t toNodeId,
             qreal firstBitTx,
             qreal firstBitRx,
             qreal lastBitTx,
             qreal lastBitRx,
             bool isWPacket,
             QString metaInfo,
             bool showMetaInfo,
             uint8_t numWirelessSlots);
  ~AnimPacket ();

  typedef enum {
    ALL= 0 << 0,
    TCP= 1 << 0,
    UDP= 1 << 1,
    AODV= 1 << 2,
    OLSR= 1 << 3,
    DSDV= 1 << 4,
    IPV4= 1 << 5,
    WIFI= 1 << 6,
    ETHERNET= 1 << 7,
    PPP= 1 << 8,
    ICMP= 1 << 9,
    ARP= 1 << 10,
    IPV6 = 1 << 11
  } FilterType_t;
  enum { Type = ANIMPACKET_TYPE };
  int type () const
  {
    return Type;
  }
  qreal getFirstBitTx ();
  qreal getFirstBitRx ();
  qreal getLastBitRx ();
  qreal getLastBitTx ();
  uint32_t getFromNodeId ();
  uint32_t getToNodeId ();
  QPointF getFromPos ();
  QPointF getToPos ();
  void update (qreal t);
  virtual QRectF boundingRect () const;
  void paint (QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
  QPointF getHead ();
  QGraphicsSimpleTextItem * getInfoTextItem ();
  bool getIsWPacket ();
  static QString getMeta (QString metaInfo, bool shortString = true);
  static QString getMeta (QString metaInfo, int filter, bool & result, bool shortString = true);
  bool packetExpired ();
  qreal getRadius ();


private:
  uint32_t m_fromNodeId;
  uint32_t m_toNodeId;
  qreal m_firstBitTx;
  qreal m_firstBitRx;
  qreal m_lastBitTx;
  qreal m_lastBitRx;
  qreal m_velocity;
  qreal m_isWPacket;
  qreal m_distanceTraveled;
  QLineF m_line;
  qreal m_cos;
  qreal m_sin;
  QPointF m_fromPos;
  QPointF m_toPos;
  QPointF m_head;
  QRectF m_boundingRect;
  QGraphicsSimpleTextItem * m_infoText;
  qreal m_currentTime;
  uint8_t m_numWirelessSlots;
  uint8_t m_currentWirelessSlot;


  static ArpInfo parseArp (QString metaInfo, bool & result);
  static PppInfo parsePpp (QString metaInfo, bool & result);
  static EthernetInfo parseEthernet (QString metaInfo, bool & result);
  static WifiMacInfo parseWifi (QString metaInfo, bool & result);
  static Ipv4Info parseIpv4 (QString metaInfo, bool & result);
  static Ipv6Info parseIpv6 (QString metaInfo, bool & result);
  static IcmpInfo parseIcmp (QString metaInfo, bool & result);
  static UdpInfo parseUdp (QString metaInfo, bool & result);
  static TcpInfo parseTcp (QString metaInfo, bool & result);
  static AodvInfo parseAodv (QString metaInfo, bool & result);
  static DsdvInfo parseDsdv (QString metaInfo, bool & result);
  static OlsrInfo parseOlsr (QString metaInfo, bool & result);

};

class AnimPacketMgr
{
public:
  static AnimPacketMgr * getInstance ();
  AnimPacket * add (uint32_t fromId, uint32_t toId, qreal fbTx, qreal fbRx, qreal lbTx, qreal lbRx, bool isWPacket, QString metaInfo, bool showMetaInfo, uint8_t numWirelessSlots);
private:
  AnimPacketMgr ();


};

}
#endif // ANIMPACKET_H
