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

#include "countertablesscene.h"
#include "statisticsconstants.h"
#include "animnode.h"
#include "animatormode.h"
#include "statsview.h"

namespace netanim
{

NS_LOG_COMPONENT_DEFINE ("CounterTablesScene");
CounterTablesScene * pCounterTablesScene = 0;

CounterTablesScene::CounterTablesScene ():
  QGraphicsScene (100, 0, STATSSCENE_WIDTH_DEFAULT, 6 * STATSSCENE_HEIGHT_DEFAULT),
  m_plot (0),
  m_plotItem (0),
  m_showChart (true)
{
  m_table = new Table ();
  m_tableItem = new QGraphicsProxyWidget;
  m_tableItem->setWidget (m_table);
  addItem (m_tableItem);

}

CounterTablesScene *
CounterTablesScene::getInstance ()
{
  if (!pCounterTablesScene)
    {
      pCounterTablesScene = new CounterTablesScene;
    }
  return pCounterTablesScene;
}


void
CounterTablesScene::setCurrentCounterName (QString name)
{
  m_currentCounterName = name;
  reloadContent ();
}

uint32_t
CounterTablesScene::getIndexForNode (uint32_t nodeId)
{
  int index = 0;
  for (int i = 0; i < m_allowedNodes.count (); ++i)
    {
      if (i == static_cast <int> (nodeId))
        return index;
      ++index;
    }
  return index;
}

bool
CounterTablesScene::isAllowedNode (uint32_t nodeId)
{
  for (int i = 0; i < m_allowedNodes.count (); ++i)
    {
      if (m_allowedNodes[i] == nodeId)
        return true;
    }
  return false;
}

void
CounterTablesScene::reloadContent (bool force)
{
  Q_UNUSED (force);

  if (m_plotItem)
    {
      removeItem (m_plotItem);
      delete m_plot;
      m_plot = 0;
      m_plotItem = 0;
    }

  typedef QVector<double> valueVector_t;
  typedef std::map <uint32_t, valueVector_t> nodeValueMap_t;
  nodeValueMap_t nodeTimes;
  nodeValueMap_t nodeCounterValues;
  qreal minCounter = uint32_t(-1);
  qreal maxCounter = 0;
  qreal maxTime = 0;


      m_table->clear ();
      QStringList headerList;
      headerList << "Time";
      for (QVector <uint32_t>::const_iterator i = m_allowedNodes.begin ();
           i != m_allowedNodes.end ();
           ++i)
        {
          headerList << QString::number (*i);
        }
      m_table->setHeaderList (headerList);

      bool result = false;
      AnimNode::CounterType_t counterType;
      uint32_t counterId = AnimNodeMgr::getInstance ()->getCounterIdForName (m_currentCounterName, result, counterType);
      if (!result)
        return;



      TimeValue <AnimEvent *> * events = AnimatorMode::getInstance ()->getEvents ();
      for (TimeValue<AnimEvent *>::TimeValue_t::const_iterator i = events->Begin ();
          i != events->End ();
          ++i)
        {
          AnimEvent * ev = i->second;
          if (ev->m_type == AnimEvent::UPDATE_NODE_COUNTER_EVENT)
            {
              AnimNodeCounterUpdateEvent * updateEvent = static_cast<AnimNodeCounterUpdateEvent *> (ev);

              if (!isAllowedNode (updateEvent->m_nodeId))
                continue;
              if (updateEvent->m_counterId != counterId)
                continue;
              m_table->incrRowCount ();

              if (nodeTimes.find (updateEvent->m_nodeId) == nodeTimes.end ())
                {
                  valueVector_t newVec;
                  nodeTimes[updateEvent->m_nodeId] = newVec;
                  nodeCounterValues[updateEvent->m_nodeId] = newVec;
                }
              valueVector_t & timeVec = nodeTimes[updateEvent->m_nodeId];
              timeVec.push_back (i->first);
              maxTime = qMax (maxTime, i->first);
              //NS_LOG_DEBUG ("TimeVec Count:" << timeVec.count());

              m_table->addCell (0, QString::number (i->first));
              //NS_LOG_DEBUG ("T:" << i->first);
              if (counterType == AnimNode::DOUBLE_COUNTER)
                {
                  qreal value = updateEvent->m_counterValue;
                  m_table->addCell (getIndexForNode (updateEvent->m_nodeId)+1, QString::number (value));
                  //NS_LOG_DEBUG ("Val:" << value);
                  valueVector_t & counterVec = nodeCounterValues[updateEvent->m_nodeId];
                  counterVec.push_back (value);
                  minCounter = qMin (minCounter, value);
                  maxCounter = qMax (maxCounter, value);


                }
              else if (counterType == AnimNode::UINT32_COUNTER)
                {
                  uint32_t value = static_cast <uint32_t> (updateEvent->m_counterValue);
                  m_table->addCell (getIndexForNode (updateEvent->m_nodeId)+1, QString::number (value));
                  valueVector_t & counterVec = nodeCounterValues[updateEvent->m_nodeId];
                  counterVec.push_back (value);
                  minCounter = qMin (minCounter, (double) value);
                  maxCounter = qMax (maxCounter, (double) value);
                }

            }
        }
      m_tableItem->setMinimumWidth (sceneRect ().width ());
      m_tableItem->setMinimumHeight (sceneRect ().height ());
      m_table->adjust ();


          if (nodeTimes.empty ())
            return;

          QVector<QCPScatterStyle::ScatterShape> shapes;
          shapes << QCPScatterStyle::ssCross;
          shapes << QCPScatterStyle::ssPlus;
          shapes << QCPScatterStyle::ssCircle;
          shapes << QCPScatterStyle::ssDisc;
          shapes << QCPScatterStyle::ssSquare;
          shapes << QCPScatterStyle::ssDiamond;
          shapes << QCPScatterStyle::ssStar;
          shapes << QCPScatterStyle::ssTriangle;
          shapes << QCPScatterStyle::ssTriangleInverted;
          shapes << QCPScatterStyle::ssCrossSquare;
          shapes << QCPScatterStyle::ssPlusSquare;
          shapes << QCPScatterStyle::ssCrossCircle;
          shapes << QCPScatterStyle::ssPlusCircle;
          shapes << QCPScatterStyle::ssPeace;
          shapes << QCPScatterStyle::ssCustom;



          m_plot = new QCustomPlot;
          m_plotItem = new QGraphicsProxyWidget;
          m_plotItem->setWidget (m_plot);
          addItem (m_plotItem);

          uint32_t chartIndex = 0;
          QPen pen;
          for (nodeValueMap_t::const_iterator i = nodeTimes.begin ();
               i != nodeTimes.end ();
               ++i)
            {
              m_plot->addGraph ()->setName ("Node "+ QString::number (i->first));
              m_plot->graph (chartIndex)->setPen (pen);
              m_plot->graph (chartIndex)->setScatterStyle (QCPScatterStyle(shapes.at (chartIndex % shapes.count ()), 10));
              m_plot->graph (chartIndex)->setData (nodeTimes[i->first], nodeCounterValues[i->first]);
              pen.setColor(QColor(sin(chartIndex*0.3)*100+100, sin(chartIndex*0.6+0.7)*100+100, sin(chartIndex*0.4+0.6)*100+100));

              //NS_LOG_DEBUG ("NodeTime Count:" << nodeTimes[0].count());
              //NS_LOG_DEBUG ("NodeCounter Count:" << nodeCounterValues[0].count());
              ++chartIndex;
            }
          m_plot->xAxis->setLabel("Time");
          m_plot->yAxis->setLabel(m_currentCounterName);
          m_plot->xAxis->setRange (0, maxTime);
          m_plot->yAxis->setRange (minCounter, maxCounter);
          m_plot->legend->setVisible (true);
          m_plot->legend->setFont(QFont ("Helvetica",9));


          //m_plot->setMinimumWidth(500);
          //m_plot->setMinimumHeight(500);
          QRectF viewRect = StatsView::getInstance ()->viewport ()->rect ();
          viewRect.setWidth (viewRect.width () * 0.9);
          m_plotItem->setMinimumWidth (viewRect.width ());
          m_plotItem->setMinimumHeight (viewRect.height ());
          m_tableItem->setVisible (!m_showChart);
          m_plotItem->setVisible (m_showChart);

}

void
CounterTablesScene::showChart (bool show)
{
  m_showChart = show;
  m_tableItem->setVisible (!m_showChart);
  m_plotItem->setVisible (m_showChart);
}

void
CounterTablesScene::setAllowedNodesVector (QVector<uint32_t> allowedNodes)
{
  m_allowedNodes = allowedNodes;
}

}
