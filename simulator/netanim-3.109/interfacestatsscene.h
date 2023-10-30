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

#ifndef INTERFACESTATSSCENE_H
#define INTERFACESTATSSCENE_H

#include "common.h"

namespace netanim
{

class InterfaceStatsScene : public QGraphicsScene
{

public:
  static InterfaceStatsScene * getInstance ();
  void test ();
  void add (uint32_t nodeId, QString pointADescription, uint32_t otherNodeId, QString pointBDescription, QString linkDescription);
  void adjustRect ();
  void systemReset ();
  void reloadContent (bool force = false);
private:
  typedef struct
  {
    uint32_t toId;
    QString pointADescription;
    QString pointBDescription;
    QString linkDescription;
  } LinkProperty_t;

  typedef std::vector <QGraphicsProxyWidget *> ProxyWidgetVector_t;
  typedef std::map <uint32_t, ProxyWidgetVector_t> NodeIdProxyWidgetVectorMap_t;
  InterfaceStatsScene ();
  void addToProxyWidgetsMap (uint32_t nodeId, QGraphicsProxyWidget *);
  void clearProxyWidgetsMap ();
  void showInfoWidget (bool show = true);
  qreal m_lastX;
  qreal m_lastY;
  qreal m_bottomY;
  bool m_dirty;
  QGraphicsProxyWidget * m_infoWidget;
  NodeIdProxyWidgetVectorMap_t m_nodeIdProxyWidgets;
  qreal m_currentMaxHeight;

};

} // namespace netanim

#endif // INTERFACESTATSSCENE_H
