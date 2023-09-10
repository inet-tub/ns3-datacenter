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
 * Contributions: Eugene Kalishenko <ydginster@gmail.com> (Open Source and Linux Laboratory http://dev.osll.ru/)
 * 		  Dmitrii Shakshin <d.shakshin@gmail.com> (Open Source and Linux Laboratory http://dev.osll.ru/)
 */
#ifndef ANIMNODE_H
#define ANIMNODE_H

#include "common.h"
#include "resizeableitem.h"

namespace netanim
{

typedef
struct {
  qreal t;
  QPointF p;
} TimePosition_t;

class AnimNode: public ResizeableItem
{
public:
  typedef QSet <QString> Ipv4Set_t;
  typedef QSet <QString> Ipv6Set_t;

  typedef QVector <QString> MacVector_t;
  typedef std::map <uint32_t, uint32_t> CounterIdValueUint32_t;
  typedef std::map <uint32_t, double> CounterIdValueDouble_t;

  typedef enum {
    UINT32_COUNTER,
    DOUBLE_COUNTER
  } CounterType_t;

  AnimNode (uint32_t nodeId, uint32_t nodeSysId, qreal x, qreal y, QString nodeDescription);
  ~AnimNode ();
  void paint (QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
  void mouseMoveEvent (QGraphicsSceneMouseEvent *event);
  void setNodeDescription (QString description);
  QPointF getCenter ();
  QGraphicsTextItem * getDescription ();
  qreal getX ();
  qreal getY ();
  void setX (qreal x);
  void setY (qreal y);
  bool getShowNodeTrajectory ();
  QColor getColor ();
  uint32_t getNodeId ();
  uint32_t getNodeSysId ();
  qreal getWidth ();
  int getResourceId ();
  Ipv4Set_t getIpv4Addresses ();
  Ipv6Set_t getIpv6Addresses ();
  MacVector_t getMacAddresses ();
  void setWidth (qreal width);
  void setHeight (qreal height);
  void setColor (uint8_t r, uint8_t g, uint8_t b, uint8_t alpha = 255);
  void setResource (int resourceId);
  void setPos (qreal x, qreal y);
  void setShowNodeTrajectory (bool showNodeTrajectory);
  void addIpv4Address (QString ip);
  void addIpv6Address (QString ip);
  void addMacAddress (QString mac);
  bool hasIpv4 (QString ip);
  bool hasMac (QString mac);
  void showNodeId (bool show);
  void showNodeSysId (bool show);
  bool isVisibleNodeSysId () const;
  void updateCounter (uint32_t counterId, qreal counterValue, CounterType_t counterType);

  qreal getDoubleCounterValue (uint32_t counterId, bool & result);
  uint32_t getUint32CounterValue (uint32_t counterId, bool & result);
  void updateBatteryCapacityImage (bool show);
  void updateNodeSysId (uint32_t nodeSysId, bool show);

private:
  QGraphicsTextItem * m_nodeDescription;
  uint32_t m_nodeId;
  uint32_t m_nodeSysId;
  qreal m_x;
  qreal m_y;
  bool m_showNodeId;
  bool m_showNodeSysId;
  Ipv4Set_t m_ipv4Set;
  Ipv6Set_t m_ipv6Set;
  MacVector_t m_macVector;
  int m_resourceId;
  bool m_showNodeTrajectory;
  QPixmap m_batteryPixmap; //!< Battery image
  bool m_showBatteryCapcity;

  QColor m_lastColor;

  CounterIdValueUint32_t m_counterIdToValuesUint32;
  CounterIdValueDouble_t m_counterIdToValuesDouble;

};


class AnimNodeMgr
{
public:
  typedef std::map <uint32_t, AnimNode *> NodeIdAnimNodeMap_t;
  typedef QVector <TimePosition_t> TimePosVector_t;
  typedef std::map <uint32_t, TimePosVector_t> NodeIdPositionMap_t;
  typedef std::map <uint32_t, QString> CounterIdName_t;

  static AnimNodeMgr * getInstance ();
  AnimNode * getNode (uint32_t nodeId);
  AnimNode * add (uint32_t nodeId, uint32_t nodeSysId, qreal x, qreal y, QString nodeDescription);
  uint32_t getCount ();
  QPointF getMinPoint ();
  QPointF getMaxPoint ();
  void systemReset ();
  void addIpv4Address (uint32_t nodeId, QString ip);
  void addIpv6Address (uint32_t nodeId, QString ip);
  void addMacAddress (uint32_t nodeId, QString mac);
  void setSize (qreal width, qreal height);
  void showNodeId (bool show);
  void showNodeSysId (bool show);
  TimePosVector_t getPositions (uint32_t nodeId);
  void addAPosition (uint32_t nodeId, qreal t, QPointF pos);
  void showRemainingBatteryCapacity (bool show);

  void addNodeCounterUint32 (uint32_t counterId, QString counterName);
  void addNodeCounterDouble (uint32_t counterId, QString counterName);
  void updateNodeCounter (uint32_t nodeId, uint32_t counterId, qreal counterValue);

  CounterIdName_t getUint32CounterNames ();
  CounterIdName_t getDoubleCounterNames ();
  uint32_t getCounterIdForName (QString counterName, bool & result, AnimNode::CounterType_t & counterType);

private:
  AnimNodeMgr ();
  NodeIdAnimNodeMap_t m_nodes;
  qreal m_minX;
  qreal m_minY;
  qreal m_maxX;
  qreal m_maxY;
  NodeIdPositionMap_t m_nodePositions;
  CounterIdName_t m_counterIdToNamesUint32;
  CounterIdName_t m_counterIdToNamesDouble;

};


}
#endif // ANIMNODE_H
