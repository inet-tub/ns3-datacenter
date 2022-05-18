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
 * Author: John Abraham <john.abraham@gatech.edu>
 */

#include "routingstatsscene.h"
#include "textbubble.h"
#include "statisticsconstants.h"
#include "statsmode.h"
#include "animatormode.h"
#include "timevalue.h"

namespace netanim
{

RoutingStatsScene * pRoutingStatsScene = 0;

RoutingStatsScene::RoutingStatsScene ():QGraphicsScene (0, 0, STATSSCENE_WIDTH_DEFAULT, STATSSCENE_HEIGHT_DEFAULT)
{
  m_lastX = 0;
  m_lastY = 0;
  m_lastTime = -1;
  m_infoWidget = addWidget (new TextBubble ("Info:", "No data available\nDid you load the XML file?"));
  showInfoWidget ();
}

RoutingStatsScene *
RoutingStatsScene::getInstance ()
{
  if (!pRoutingStatsScene)
    {
      pRoutingStatsScene = new RoutingStatsScene;
    }
  return pRoutingStatsScene;
}


void
RoutingStatsScene::addToProxyWidgetsMap (uint32_t nodeId, QString title, QString content)
{

  if (m_nodeIdProxyWidgets.find (nodeId) == m_nodeIdProxyWidgets.end ())
    {
      TextBubble * tb = new TextBubble (title, content);
      QFont f (tb->font ());
      f.setPointSizeF (StatsMode::getInstance ()->getCurrentFontSize ());
      tb->setFont (f);
      QGraphicsProxyWidget * pw = addWidget (tb);


      QFontMetrics fm (f);
      pw->setMaximumHeight (fm.height () * tb->text ().count ("\n"));
      pw->adjustSize ();
      showInfoWidget (false);
      m_nodeIdProxyWidgets[nodeId] = pw;
      qreal newX = m_lastX + pw->widget ()->width ();
      if (newX >= sceneRect ().right ())
        {
          m_lastX = 0;
          m_lastY += pw->widget ()->height () + INTERSTATS_SPACE;
        }
      pw->setPos (m_lastX, m_lastY);

      m_lastX = pw->pos ().x () + pw->widget ()->width () + INTERSTATS_SPACE;
      m_lastY = pw->pos ().y ();
      m_bottomY = m_lastY + pw->widget ()->height ();
      //qDebug (QString ("Last X" + QString::number (m_lastX) + " w:" + QString::number (pw->widget ()->width ())));
      adjustRect ();
      return;
    }

}

uint32_t
RoutingStatsScene::getNodeCount ()
{
  return m_nodeIdProxyWidgets.size ();
}

void
RoutingStatsScene::add (uint32_t nodeId, qreal time, QString rt)
{
  if (m_nodeIdTimeValues.find (nodeId) == m_nodeIdTimeValues.end ())
    {
      TimeValue <QString> tv;
      tv.add (time, rt);
      m_nodeIdTimeValues[nodeId] = tv;
      addToProxyWidgetsMap (nodeId, "", rt);
      return;
    }
  TimeValue <QString> & tv = m_nodeIdTimeValues[nodeId];
  tv.add (time, rt);

}

void
RoutingStatsScene::addRp (uint32_t nodeId, QString destination, qreal time, RoutePathElementsVector_t elements)
{
  NodeIdDest_t nd = { nodeId, destination };
  if (m_rps.find (nd) == m_rps.end ())
    {
      TimeValue <RoutePathElementsVector_t> tv;
      tv.add (time, elements);
      m_rps[nd] = tv;
      return;
    }
  TimeValue <RoutePathElementsVector_t> & tv = m_rps[nd];
  tv.add (time, elements);
}

RoutePathVector_t
RoutingStatsScene::getRoutePaths (qreal currentTime)
{
  RoutePathVector_t routePaths;
  for (NodeIdDestRPMap_t::const_iterator i = m_rps.begin ();
      i != m_rps.end ();
      ++i)
    {
      NodeIdDest_t nd = i->first;
      TimeValue <RoutePathElementsVector_t> & v = m_rps[nd];
      v.setCurrentTime (currentTime);
      RoutePath_t rp = { nd, v.getCurrent () };
      routePaths.push_back (rp);
    }
  return routePaths;
}

void
RoutingStatsScene::clearProxyWidgetsMap ()
{
  showInfoWidget ();
  for (NodeIdProxyWidgetMap_t::const_iterator i = m_nodeIdProxyWidgets.begin ();
      i != m_nodeIdProxyWidgets.end ();
      ++i)
    {

      removeItem (i->second);
      delete (i->second);

    }
  m_nodeIdProxyWidgets.clear ();
}

void
RoutingStatsScene::adjustRect ()
{
  QRectF currentRect = sceneRect ();
  QRectF newRect = QRectF (currentRect.topLeft (), QPointF (currentRect.bottomRight ().x (), m_bottomY));
  setSceneRect (newRect);
}

void
RoutingStatsScene::test ()
{
  for (uint32_t i=0; i < 100; ++i)
    {
      //qDebug (sceneRect (), "Scene Rect");
      //add (i, "10.1.1.1~00:00:00:00:00:06", i+1, "10.1.1.1~00:00:00:00:00:06", "lp.linkDescription");
    }
}

void
RoutingStatsScene::systemReset ()
{
  m_lastX = 0;
  m_lastY = 0;
  m_bottomY = 0;
  clearProxyWidgetsMap ();
  clearNodeIdTimeValues ();
}

void
RoutingStatsScene::clearNodeIdTimeValues ()
{
  m_nodeIdTimeValues.clear ();
}

void
RoutingStatsScene::showInfoWidget (bool show)
{
  m_infoWidget->setVisible (show);
  m_infoWidget->setPos (sceneRect ().width ()/2, sceneRect ().height ()/2);
}

void
RoutingStatsScene::updateContent (uint32_t nodeId, QGraphicsProxyWidget *pw)
{
  //qDebug ("Updating for :" + QString::number (nodeId));
  TimeValue <QString> & v = m_nodeIdTimeValues[nodeId];
  v.setCurrentTime (StatsMode::getInstance ()->getCurrentTime ());
  TextBubble * tb = ( (TextBubble *)pw->widget ());
  QFont f (tb->font ());
  f.setPointSizeF (StatsMode::getInstance ()->getCurrentFontSize ());
  tb->setFont (f);
  QFontMetrics fm (f);
  pw->setMaximumHeight (fm.height () * tb->text ().count ("\n"));
  pw->adjustSize ();
  tb->setText (v.getCurrent ());
}

void
RoutingStatsScene::reloadContent (bool force)
{
  if (m_nodeIdProxyWidgets.empty ())
    {
      return;
    }

  m_lastX = 0;
  m_lastY = 0;
  m_bottomY = 0;
  qreal currentTime = StatsMode::getInstance ()->getCurrentTime ();

  qreal currentMaxHeight = 0;
  for (NodeIdProxyWidgetMap_t::const_iterator i = m_nodeIdProxyWidgets.begin ();
      i != m_nodeIdProxyWidgets.end ();
      ++i)
    {
      QGraphicsProxyWidget * pw = i->second;

      if ((force) || (!m_lastTime) || (m_lastTime != currentTime))
        {
          updateContent (i->first, pw);
        }


      bool nodeIsActive = StatsMode::getInstance ()->isNodeActive (i->first);
      pw->setVisible (nodeIsActive);
      if (nodeIsActive)
        {
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

  m_lastTime = currentTime;


}

} // namespace netanim
