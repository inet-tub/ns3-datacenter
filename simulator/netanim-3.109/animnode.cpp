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

#include "animnode.h"
#include "animresource.h"
#include "animatorview.h"
#include <QColor>

NS_LOG_COMPONENT_DEFINE ("AnimNode");
namespace netanim
{

AnimNodeMgr * pAnimNodeMgr = 0;

AnimNode::AnimNode (uint32_t nodeId, uint32_t nodeSysId, qreal x, qreal y, QString nodeDescription):
  m_nodeDescription (0),
  m_nodeId (nodeId),
  m_nodeSysId (nodeSysId),
  m_x (x),
  m_y (y),
  m_showNodeId (true),
  m_showNodeSysId (false),
  m_resourceId (-1),
  m_showNodeTrajectory (false),
  m_showBatteryCapcity (false)
{
  //setVisible (false);
  setZValue (ANIMNODE_ZVALUE);
  m_r = 255;
  m_g = 0;
  m_b = 0;
  if (nodeDescription == "")
    {
      nodeDescription = QString::number (nodeId);
    }
  m_nodeDescription = new QGraphicsTextItem (nodeDescription);
  m_nodeDescription->setFlag (QGraphicsItem::ItemIgnoresTransformations);
  setFlag (QGraphicsItem::ItemIsSelectable);
}

AnimNode::~AnimNode ()
{
  if (m_nodeDescription)
    {
      delete m_nodeDescription;
    }
}

void
AnimNode::showNodeId (bool show)
{
  m_showNodeId = show;
  m_nodeDescription->setVisible (m_showNodeId);
}

QColor
generateColor (size_t index, uint8_t alpha = 0)
{

  static const size_t colors[] =
    { Qt::blue, Qt::magenta, Qt::darkCyan, Qt::darkYellow, Qt::darkRed, Qt::darkMagenta, Qt::darkGreen, Qt::darkBlue,
	Qt::black, Qt::darkGray, Qt::lightGray };
  static const size_t COUNT = sizeof (colors) / sizeof (size_t);
  QColor result;

  if (index < COUNT)
    result = QColor (Qt::GlobalColor (colors[index]));
  else
    {
      result = QColor (Qt::GlobalColor (colors[index % COUNT]));
      const int step = 256 * 3 % COUNT;

      result.setRed ((result.red () + step * index) % 255);
      result.setGreen ((result.blue () + step * index) % 255);
      result.setBlue (((int)result.green () - step * index) % 255);
    }
  if (alpha)
    result.setAlpha (alpha);

  return result;
}


void
AnimNode::showNodeSysId (bool show)
{
  if (show)
    {
      m_lastColor = this->getColor ();
      int r, g, b, a;
      m_lastColor.getRgb(&r, &g, &b, &a);
      const QColor &color = generateColor (m_nodeSysId, a);
      color.getRgb (&r, &g, &b, &a);
      setColor (static_cast<uint8_t> (r), static_cast<uint8_t> (g), static_cast<uint8_t> (b), static_cast<uint8_t> (a));
    }
  else
    {
      if(m_showNodeSysId)
	{
	  int r, g, b, a;
	  m_lastColor.getRgb (&r, &g, &b, &a);
	  setColor (r, g, b, a);
	}
    }
  m_showNodeSysId = show;
  m_nodeDescription->setPlainText (QString::number (m_nodeId) + (m_showNodeSysId ? QString(" SysId:") +
  QString::number (m_nodeSysId): QString()));
}

bool
AnimNode::isVisibleNodeSysId() const
{
  return m_showNodeSysId;
}

void
AnimNode::updateNodeSysId (uint32_t sysId, bool show)
{
  m_nodeSysId = sysId;
  //m_nodeSysIdDescription->setPlainText ("sysId: " + QString::number (sysId));
  showNodeSysId (show);
}

void
AnimNode::updateBatteryCapacityImage (bool show)
{

  m_showBatteryCapcity = show;
  QString batteryCapacityImagePath(":/resources/battery_icon_");
  bool result = false;
  CounterType_t counterType;
  uint32_t counterId = AnimNodeMgr::getInstance ()->getCounterIdForName ("RemainingEnergy", result, counterType);
  if (!result)
    {
      return;
    }

  result = false;
  qreal capacity = getDoubleCounterValue (counterId, result);
  if (!result)
    {
      return;
    }

  if (capacity > 0.75) batteryCapacityImagePath += "4";
  else if (capacity > 0.5) batteryCapacityImagePath += "3";
  else if (capacity > 0.25) batteryCapacityImagePath += "2";
  else if (capacity >= 0) batteryCapacityImagePath += "1";
  else batteryCapacityImagePath += "0";
 // NS_LOG_DEBUG ("Capacity:" << batteryCapacityImagePath.toAscii().data());

  batteryCapacityImagePath += ".png";
  if(show)
    {
      m_batteryPixmap = QPixmap (batteryCapacityImagePath);
    }
  if (!show)
    {
      m_batteryPixmap = QPixmap ();
    }

}




uint32_t
AnimNode::getUint32CounterValue (uint32_t counterId, bool & result)
{
  result = false;
  if (m_counterIdToValuesUint32.find (counterId) == m_counterIdToValuesUint32.end ())
    return -1;
  result = true;
  return m_counterIdToValuesUint32[counterId];
}




qreal
AnimNode::getDoubleCounterValue (uint32_t counterId, bool & result)
{
  result = false;
  if (m_counterIdToValuesDouble.find (counterId) == m_counterIdToValuesDouble.end ())
    return -1;
  result = true;
  return m_counterIdToValuesDouble[counterId];
}

void
AnimNode::updateCounter (uint32_t counterId, qreal counterValue, CounterType_t counterType)
{
  if (counterType == DOUBLE_COUNTER)
    {
      m_counterIdToValuesDouble[counterId] = counterValue;
    }

  if (counterType == UINT32_COUNTER)
    {
      m_counterIdToValuesUint32[counterId] = counterValue;
    }
}



int
AnimNode::getResourceId ()
{
  return m_resourceId;
}


void
AnimNode::setResource (int resourceId)
{
  m_resourceId = resourceId;
  QString resourcePath = AnimResourceManager::getInstance ()->get (resourceId);
  //NS_LOG_DEBUG ("Res:" << resourcePath.toAscii ().data ());
  QPixmap pix;
  if (resourcePath.endsWith (".png"))
    pix = QPixmap (resourcePath, "png");
  else if (resourcePath.endsWith (".svg"))
    pix = QPixmap (resourcePath, "svg");
  setPixmap (pix);
  update ();
}

void
AnimNode::setColor (uint8_t r, uint8_t g, uint8_t b, uint8_t alpha)
{
  m_r = r;
  m_g = g;
  m_b = b;
  m_alpha = alpha;
  ResizeableItem::setColor (r, g, b, alpha);
  update ();
}

QColor
AnimNode::getColor ()
{
  QColor c (m_r, m_g, m_b, 255);
  return c;
}

qreal
AnimNode::getWidth ()
{
  return m_width;
}



qreal
AnimNode::getX ()
{
  return m_x;
}

qreal
AnimNode::getY ()
{
  return m_y;
}


void
AnimNode::setPos (qreal x, qreal y)
{
  m_x = x;
  m_y = y;
  QGraphicsItem::setPos (x, y);
}

void
AnimNode::setX (qreal x)
{
  m_x = x;
}


bool
AnimNode::getShowNodeTrajectory ()
{
  return m_showNodeTrajectory;
}

void
AnimNode::setShowNodeTrajectory (bool showNodeTrajectory)
{
  m_showNodeTrajectory = showNodeTrajectory;
}


void
AnimNode::setY (qreal y)
{
  m_y = y;
}

uint32_t
AnimNode::getNodeId ()
{
  return m_nodeId;
}

uint32_t
AnimNode::getNodeSysId ()
{
  return m_nodeSysId;
}

QGraphicsTextItem *
AnimNode::getDescription ()
{
  return m_nodeDescription;
}

QPointF AnimNode::getCenter ()
{
  //return sceneBoundingRect ().center ();
  return QPointF (m_x, m_y);
}
void AnimNode::setNodeDescription (QString description)
{
  m_nodeDescription->setPlainText (description);
}

void AnimNode::paint (QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
  ResizeableItem::paint (painter, option, widget);
  if (!m_batteryPixmap.isNull ())
    {
      updateBatteryCapacityImage (m_showBatteryCapcity);
      QPointF bottomLeft = sceneBoundingRect ().bottomLeft ();
      //NS_LOG_DEBUG ("Pix Width:" << m_batteryPixmap->width());
      bottomLeft = QPointF (-1, 1);
      painter->save ();
      painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing | QPainter::Antialiasing, true);      painter->scale (0.5, 1);
      painter->drawPixmap (bottomLeft.x (), bottomLeft.y (), 5, 5, m_batteryPixmap);

      painter->restore ();
    }
}


void AnimNode::mouseMoveEvent (QGraphicsSceneMouseEvent *event)
{
  ResizeableItem::mouseMoveEvent (event);
  if (m_nodeDescription)
    {
      m_nodeDescription->setPos (sceneBoundingRect ().bottomRight ());
      update ();
    }
}

AnimNode::Ipv4Set_t
AnimNode::getIpv4Addresses ()
{
  return m_ipv4Set;
}

AnimNode::Ipv6Set_t
AnimNode::getIpv6Addresses ()
{
  return m_ipv6Set;
}

AnimNode::MacVector_t
AnimNode::getMacAddresses ()
{
  return m_macVector;
}

void
AnimNode::addIpv4Address (QString ip)
{
  m_ipv4Set.insert (ip);
}

void
AnimNode::addIpv6Address (QString ip)
{
  m_ipv6Set.insert (ip);
}

void
AnimNode::addMacAddress (QString mac)
{
  m_macVector.push_back (mac);
}

bool
AnimNode::hasIpv4 (QString ip)
{
  bool result = false;
  QStringList quads = ip.split (".");
  if (quads.count () == 4)
    {
      if (quads.at (3) == "255")
        return true;
      for (Ipv4Set_t::const_iterator i = m_ipv4Set.begin ();
          i != m_ipv4Set.end ();
          ++i)
        {
          if (*i == ip)
            {
              //QDEBUG (ip);
              return true;
            }
        }
    }

  return result;
}


bool
AnimNode::hasMac (QString mac)
{
  bool result = false;
  QStringList bytes = mac.split (":");
  if (bytes.count () == 6)
    {
      for (MacVector_t::const_iterator i = m_macVector.begin ();
          i != m_macVector.end ();
          ++i)
        {
          if (*i == mac)
            {
              return true;
            }
        }
    }

  return result;
}


AnimNodeMgr::AnimNodeMgr ():
  m_minX (0),
  m_minY (0),
  m_maxX (0),
  m_maxY (0)
{

}

AnimNodeMgr * AnimNodeMgr::getInstance ()
{
  if (!pAnimNodeMgr)
    {
      pAnimNodeMgr = new AnimNodeMgr;
    }
  return pAnimNodeMgr;
}


AnimNode * AnimNodeMgr::add (uint32_t nodeId, uint32_t nodeSysId, qreal x, qreal y, QString nodeDescription)
{
  if (m_nodes.find (nodeId) != m_nodes.end ())
    {
      //NS_FATAL_ERROR ("NodeId:" << nodeId << " Already exists");
    }
  QPixmap pix (":/resources/ns3logo2.png","png");
  AnimNode * node = new AnimNode (nodeId, nodeSysId, x, y, nodeDescription);
  node->setPos (x, y);
  //node->setPixmap (pix);
  m_nodes[nodeId] = node;
  m_minX = qMin (m_minX, x);
  m_minY = qMin (m_minY, y);
  m_maxX = qMax (m_maxX, x);
  m_maxY = qMax (m_maxY, y);

  return node;
}


void
AnimNodeMgr::setSize (qreal width, qreal height)
{
  for (NodeIdAnimNodeMap_t::const_iterator i = m_nodes.begin ();
      i != m_nodes.end ();
      ++i)
    {
      AnimNode * animNode = i->second;
      animNode->setSize (width, height);
    }
}


void
AnimNodeMgr::showRemainingBatteryCapacity (bool show)
{
  for (NodeIdAnimNodeMap_t::const_iterator i = m_nodes.begin ();
      i != m_nodes.end ();
      ++i)
    {
      AnimNode * animNode = i->second;
      animNode->updateBatteryCapacityImage (show);
    }
}

AnimNode * AnimNodeMgr::getNode (uint32_t nodeId)
{
  return m_nodes[nodeId];
}

uint32_t
AnimNodeMgr::getCount ()
{
  return m_nodes.size ();
}


QPointF
AnimNodeMgr::getMinPoint ()
{
  return QPointF (m_minX, m_minY);
}

QPointF
AnimNodeMgr::getMaxPoint ()
{
  qreal m = qMax (m_maxX, m_maxY);
  return QPointF (m, m);
}


void
AnimNodeMgr::systemReset ()
{
  m_nodes.clear ();
  m_minX = 0;
  m_minY = 0;
  m_maxX = 0;
  m_maxY = 0;
  m_counterIdToNamesDouble.clear ();
  m_counterIdToNamesUint32.clear ();
}


void
AnimNodeMgr::addIpv4Address (uint32_t nodeId, QString ip)
{
  getNode (nodeId)->addIpv4Address (ip);
}

void
AnimNodeMgr::addIpv6Address (uint32_t nodeId, QString ip)
{
  getNode (nodeId)->addIpv6Address (ip);
}

void
AnimNodeMgr::addMacAddress (uint32_t nodeId, QString mac)
{
  getNode (nodeId)->addMacAddress (mac);
}

void
AnimNodeMgr::showNodeId (bool show)
{
  for (NodeIdAnimNodeMap_t::const_iterator i = m_nodes.begin ();
      i != m_nodes.end ();
      ++i)
    {
      AnimNode * animNode = i->second;
      animNode->showNodeId (show);
    }

}

void
AnimNodeMgr::showNodeSysId (bool show)
{
  for (NodeIdAnimNodeMap_t::const_iterator i = m_nodes.begin ();
      i != m_nodes.end ();
      ++i)
    {
      AnimNode * animNode = i->second;
      animNode->showNodeSysId (show);
    }
}


AnimNodeMgr::TimePosVector_t
AnimNodeMgr::getPositions (uint32_t nodeId)
{
  return m_nodePositions[nodeId];
}

void
AnimNodeMgr::addAPosition (uint32_t nodeId, qreal t, QPointF pos)
{
  if (m_nodePositions.find (nodeId) == m_nodePositions.end ())
    {
      TimePosVector_t posVector;
      m_nodePositions[nodeId] = posVector;
    }
  TimePosVector_t & pv = m_nodePositions[nodeId];
  TimePosition_t tp;
  tp.p = pos;
  tp.t = t;
  pv.push_back (tp);
}


void
AnimNodeMgr::addNodeCounterUint32 (uint32_t counterId, QString counterName)
{
  m_counterIdToNamesUint32[counterId] = counterName;
}

void
AnimNodeMgr::addNodeCounterDouble (uint32_t counterId, QString counterName)
{
  m_counterIdToNamesDouble[counterId] = counterName;
}

void
AnimNodeMgr::updateNodeCounter (uint32_t nodeId, uint32_t counterId, qreal counterValue)
{
  AnimNode * animNode = getNode (nodeId);
  AnimNode::CounterType_t ct;
  bool counterFound = false;
  for (CounterIdName_t::const_iterator i = m_counterIdToNamesDouble.begin ();
       i != m_counterIdToNamesDouble.end ();
       ++i)
    {
      if (counterId == i->first)
        {
          ct = AnimNode::DOUBLE_COUNTER;
          counterFound = true;
          break;
        }
    }
  if (!counterFound)
    {
    for (CounterIdName_t::const_iterator i = m_counterIdToNamesUint32.begin ();
         i != m_counterIdToNamesUint32.end ();
         ++i)
      {
        if (counterId == i->first)
          {
            ct = AnimNode::UINT32_COUNTER;
            counterFound = true;
            break;
          }
      }
    }
  animNode->updateCounter (counterId, counterValue, ct);
}


AnimNodeMgr::CounterIdName_t
AnimNodeMgr::getUint32CounterNames ()
{
  return m_counterIdToNamesUint32;
}

AnimNodeMgr::CounterIdName_t
AnimNodeMgr::getDoubleCounterNames ()
{
  return m_counterIdToNamesDouble;
}



uint32_t
AnimNodeMgr::getCounterIdForName (QString counterName, bool &result, AnimNode::CounterType_t & counterType)
{
  result = false;
  for (CounterIdName_t::const_iterator i = m_counterIdToNamesDouble.begin ();
       i != m_counterIdToNamesDouble.end ();
       ++i)
    {
      QString n = i->second;
      if (n == counterName)
        {
          result = true;
          counterType = AnimNode::DOUBLE_COUNTER;
          return i->first;
        }
    }
  for (CounterIdName_t::const_iterator i = m_counterIdToNamesUint32.begin ();
       i != m_counterIdToNamesUint32.end ();
       ++i)
    {
      QString n = i->second;
      if (n == counterName)
        {
          result = true;
          counterType = AnimNode::UINT32_COUNTER;
          return i->first;
        }
    }
  return -1;
}



}

