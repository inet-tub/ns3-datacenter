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

#ifndef COUNTERTABLESSCENE_H
#define COUNTERTABLESSCENE_H

#include "common.h"
#include "table.h"
#include "qcustomplot.h"

namespace netanim {

class CounterTablesScene : public QGraphicsScene
{

public:
  static CounterTablesScene * getInstance ();
  void setCurrentCounterName (QString Name);
  void reloadContent (bool force = false);
  void setAllowedNodesVector (QVector <uint32_t> allowedNodes);
  void showChart (bool show);

private:
  CounterTablesScene ();
  QString m_currentCounterName;
  Table * m_table;
  QGraphicsProxyWidget * m_tableItem;
  QVector <uint32_t> m_allowedNodes;
  bool isAllowedNode (uint32_t);
  uint32_t getIndexForNode (uint32_t nodeId);
  QCustomPlot * m_plot;
  QGraphicsProxyWidget * m_plotItem;
  bool m_showChart;


};

} // namespace netanim




#endif // COUNTERTABLESSCENE_H
