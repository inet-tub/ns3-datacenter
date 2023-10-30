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
#ifndef ROUTINGSTATSSCENE_H
#define ROUTINGSTATSSCENE_H

#include "common.h"
#include "timevalue.h"
#include "routingxmlparser.h"

namespace netanim
{


struct NodeIdDest_t
{
  uint32_t fromNodeId;
  QString destination;

  bool operator< (const NodeIdDest_t & rhs ) const
  {

    return ( fromNodeId < rhs.fromNodeId) ||
         ( fromNodeId == rhs.fromNodeId &&
         ( GET_ASCII (destination) < GET_ASCII (rhs.destination) ) );
  }
} ;

typedef struct
{
  NodeIdDest_t nodeIdDest;
  RoutePathElementsVector_t elements;
} RoutePath_t;

typedef std::vector <RoutePath_t> RoutePathVector_t;

class RoutingStatsScene : public QGraphicsScene
{
public:
  static RoutingStatsScene * getInstance ();
  void test ();
  void add (uint32_t nodeId, qreal time, QString rt);
  void addRp (uint32_t nodeId, QString destination, qreal time, RoutePathElementsVector_t elements);
  void adjustRect ();
  void systemReset ();
  void reloadContent (bool force = false);
  uint32_t getNodeCount ();
  RoutePathVector_t getRoutePaths (qreal currentTime);
private:
  typedef std::map <uint32_t, QGraphicsProxyWidget *> NodeIdProxyWidgetMap_t;
  typedef std::map <uint32_t, TimeValue <QString> > NodeIdTimeValueMap_t;
  typedef std::map <NodeIdDest_t, TimeValue <RoutePathElementsVector_t> > NodeIdDestRPMap_t;
  RoutingStatsScene ();
  void addToProxyWidgetsMap (uint32_t nodeId, QString title, QString content);
  void clearProxyWidgetsMap ();
  void clearNodeIdTimeValues ();
  void showInfoWidget (bool show = true);
  void updateContent (uint32_t nodeId, QGraphicsProxyWidget * pw);
  qreal m_lastX;
  qreal m_lastY;
  qreal m_bottomY;
  qreal m_lastTime;
  QGraphicsProxyWidget * m_infoWidget;
  NodeIdProxyWidgetMap_t m_nodeIdProxyWidgets;
  NodeIdTimeValueMap_t m_nodeIdTimeValues;
  NodeIdDestRPMap_t m_rps;

};


} // namespace netanim

#endif // ROUTINGSTATSSCENE_H
