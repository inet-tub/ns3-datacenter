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
 * Contributions: Dmitrii Shakshin <d.shakshin@gmail.com> (Open Source and Linux Laboratory http://dev.osll.ru/)
 */

#ifndef ANIMEVENT_H
#define ANIMEVENT_H

#include "common.h"
namespace netanim
{

class AnimEvent
{

public:
  typedef enum
  {
    PACKET_FBTX_EVENT,
    PACKET_LBRX_EVENT,
    ADD_NODE_EVENT,
    UPDATE_NODE_POS_EVENT,
    UPDATE_NODE_COLOR_EVENT,
    UPDATE_NODE_DESCRIPTION_EVENT,
    UPDATE_NODE_SIZE_EVENT,
    UPDATE_NODE_IMAGE_EVENT,
    UPDATE_NODE_SYSID_EVENT,
    ADD_LINK_EVENT,
    UPDATE_LINK_EVENT,
    WIRED_PACKET_UPDATE_EVENT,
    UPDATE_NODE_COUNTER_EVENT,
    CREATE_NODE_COUNTER_EVENT,
    IP_EVENT,
    IPV6_EVENT
  } AnimEventType_h;
  AnimEventType_h m_type;
  AnimEvent (AnimEventType_h type): m_type (type)
  {
  }
};


class AnimNodeAddEvent: public AnimEvent
{
public:
  AnimNodeAddEvent (uint32_t nodeId, uint32_t nodeSysId, qreal x, qreal y, QString nodeDescription,
                    uint8_t r, uint8_t g, uint8_t b): AnimEvent (ADD_NODE_EVENT),
    m_nodeId (nodeId),
    m_nodeSysId (nodeSysId),
    m_x (x),
    m_y (y),
    m_nodeDescription (nodeDescription),
    m_r (r),
    m_g (g),
    m_b (b)
  {
  }
  uint32_t m_nodeId;
  uint32_t m_nodeSysId;
  qreal m_x;
  qreal m_y;
  QString m_nodeDescription;
  uint8_t m_r;
  uint8_t m_g;
  uint8_t m_b;


};


class AnimCreateNodeCounterEvent: public AnimEvent
{
public:
  // Node Counter
  typedef enum {
    UINT32_COUNTER,
    DOUBLE_COUNTER
  } NodeCounterType_t;
  AnimCreateNodeCounterEvent (uint32_t counterId, QString counterName, NodeCounterType_t counterType):
    AnimEvent (CREATE_NODE_COUNTER_EVENT), m_counterId (counterId), m_counterName (counterName), m_counterType (counterType)
  {
  }
  uint32_t m_counterId;
  QString m_counterName;
  NodeCounterType_t m_counterType;


};

class AnimIpEvent: public AnimEvent
{
public:
  AnimIpEvent (uint32_t nodeId,  QVector<QString> ipv4Addresses):
      AnimEvent (IP_EVENT),
      m_nodeId (nodeId),
      m_ipv4Addresses (ipv4Addresses)
  {
  }
  uint32_t m_nodeId;
  QVector<QString> m_ipv4Addresses;
};

class AnimIpv6Event: public AnimEvent
{
public:
  AnimIpv6Event (uint32_t nodeId,  QVector<QString> ipv6Addresses):
      AnimEvent (IPV6_EVENT),
      m_nodeId (nodeId),
      m_ipv6Addresses (ipv6Addresses)
  {
  }
  uint32_t m_nodeId;
  QVector<QString> m_ipv6Addresses;
};

class AnimNodeCounterUpdateEvent: public AnimEvent
{
public:
  AnimNodeCounterUpdateEvent (uint32_t counterId, uint32_t nodeId, qreal counterValue):
    AnimEvent (UPDATE_NODE_COUNTER_EVENT), m_counterId (counterId), m_nodeId (nodeId), m_counterValue (counterValue)
  {
  }
  uint32_t m_counterId;
  uint32_t m_nodeId;
  qreal m_counterValue;
};


class AnimLinkAddEvent: public AnimEvent
{

public:
  AnimLinkAddEvent (uint32_t fromNodeId, uint32_t toNodeId, QString linkDescription, QString fromNodeDescription, QString toNodeDescription,
                   bool p2p=true):
    AnimEvent (ADD_LINK_EVENT),
    m_fromNodeId (fromNodeId),
    m_toNodeId (toNodeId),
    m_linkDescription (linkDescription),
    m_fromNodeDescription (fromNodeDescription),
    m_toNodeDescription (toNodeDescription),
    m_p2p (p2p)
  {
  }
  uint32_t m_fromNodeId;
  uint32_t m_toNodeId;
  QString m_linkDescription;
  QString m_fromNodeDescription;
  QString m_toNodeDescription;
  bool m_p2p;
};

class AnimLinkUpdateEvent: public AnimEvent
{

public:
  AnimLinkUpdateEvent (uint32_t fromNodeId, uint32_t toNodeId, QString linkDescription):
    AnimEvent (UPDATE_LINK_EVENT),
    m_fromNodeId (fromNodeId),
    m_toNodeId (toNodeId),
    m_linkDescription (linkDescription)
  {
  }
  uint32_t m_fromNodeId;
  uint32_t m_toNodeId;
  QString m_linkDescription;
};



class AnimWiredPacketUpdateEvent: public AnimEvent
{
public:
  AnimWiredPacketUpdateEvent ():
    AnimEvent (WIRED_PACKET_UPDATE_EVENT)
  {
  }
};


class AnimNodePositionUpdateEvent: public AnimEvent
{
public:
  AnimNodePositionUpdateEvent (uint32_t nodeId, qreal x, qreal y):
    AnimEvent (UPDATE_NODE_POS_EVENT),
    m_nodeId (nodeId),
    m_x (x),
    m_y (y)
  {
  }
  uint32_t m_nodeId;
  qreal m_x;
  qreal m_y;
};


class AnimNodeColorUpdateEvent: public AnimEvent
{
public:
  AnimNodeColorUpdateEvent (uint32_t nodeId, uint8_t r, uint8_t g, uint8_t b):
    AnimEvent (UPDATE_NODE_COLOR_EVENT),
    m_nodeId (nodeId),
    m_r (r),
    m_g (g),
    m_b (b)
  {
  }
  uint32_t m_nodeId;
  uint8_t m_r;
  uint8_t m_g;
  uint8_t m_b;

};


class AnimNodeSizeUpdateEvent: public AnimEvent
{
public:
  AnimNodeSizeUpdateEvent (uint32_t nodeId, qreal width, qreal height):
    AnimEvent (UPDATE_NODE_SIZE_EVENT),
    m_nodeId (nodeId),
    m_width (width),
    m_height (height)
  {
  }
  uint32_t m_nodeId;
  qreal m_width;
  qreal m_height;

};

class AnimNodeDescriptionUpdateEvent: public AnimEvent
{
public:
  AnimNodeDescriptionUpdateEvent (uint32_t nodeId, QString description):
    AnimEvent (UPDATE_NODE_DESCRIPTION_EVENT),
    m_nodeId (nodeId),
    m_description (description)
  {
  }
  uint32_t m_nodeId;
  QString m_description;

};



class AnimNodeImageUpdateEvent: public AnimEvent
{
public:
  AnimNodeImageUpdateEvent (uint32_t nodeId, uint32_t resourceId):
    AnimEvent (UPDATE_NODE_IMAGE_EVENT),
    m_nodeId (nodeId),
    m_resourceId (resourceId)
  {
  }
  uint32_t m_nodeId;
  uint32_t m_resourceId;

};

class AnimNodeSysIdUpdateEvent : public AnimEvent
{
public:
  AnimNodeSysIdUpdateEvent (uint32_t nodeId, uint32_t systemId) :
      AnimEvent (UPDATE_NODE_SYSID_EVENT),
      m_nodeId (nodeId),
      m_nodeSysId (systemId)
  {
  }
  uint32_t m_nodeId;
  uint32_t m_nodeSysId;
};

class AnimPacketLbRxEvent: public AnimEvent
{
public:
  AnimPacketLbRxEvent (void * p):
    AnimEvent (PACKET_LBRX_EVENT),
    m_pkt (p),
    m_valid (true)
  {
  }
  void * m_pkt;
  bool m_valid;

};


class AnimPacketEvent: public AnimEvent
{
public:
  AnimPacketEvent (uint32_t fromId,
                   uint32_t toId,
                   qreal fbTx,
                   qreal fbRx,
                   qreal lbTx,
                   qreal lbRx,
                   bool isWPacket,
                   QString metaInfo,
                   uint8_t numSlots):
    AnimEvent (PACKET_FBTX_EVENT),
    m_fromId (fromId),
    m_toId (toId),
    m_fbTx (fbTx),
    m_fbRx (fbRx),
    m_lbTx (lbTx),
    m_lbRx (lbRx),
    m_isWPacket (isWPacket),
    m_metaInfo (metaInfo),
    m_numSlots (numSlots)
  {
  }
  uint32_t m_fromId;
  uint32_t m_toId;
  qreal m_fbTx;
  qreal m_fbRx;
  qreal m_lbTx;
  qreal m_lbRx;
  bool m_isWPacket;
  QString m_metaInfo;
  uint8_t m_numSlots;


};




} // namespace netanim
#endif // ANIMEVENT_H
