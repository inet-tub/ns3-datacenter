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

#include "interfacestatsscene.h"
#include "textbubble.h"
#include "statisticsconstants.h"
#include "statsmode.h"
#include "animlink.h"
#include "animatormode.h"

namespace netanim
{

InterfaceStatsScene * pInterfaceStatsScene = 0;

InterfaceStatsScene::InterfaceStatsScene ():QGraphicsScene (100, 0, STATSSCENE_WIDTH_DEFAULT, STATSSCENE_HEIGHT_DEFAULT),
  m_dirty (true)
{
  m_lastX = 0;
  m_lastY = 0;
  m_infoWidget = addWidget (new TextBubble ("Info:", "No data available\nDid you load the XML file from the Animator Tab?"));
  showInfoWidget ();
}

InterfaceStatsScene *
InterfaceStatsScene::getInstance ()
{
  if (!pInterfaceStatsScene)
    {
      pInterfaceStatsScene = new InterfaceStatsScene;
    }
  return pInterfaceStatsScene;
}


void
InterfaceStatsScene::addToProxyWidgetsMap (uint32_t nodeId, QGraphicsProxyWidget * pw)
{
  if (m_nodeIdProxyWidgets.find (nodeId) == m_nodeIdProxyWidgets.end ())
    {
      ProxyWidgetVector_t v;
      v.push_back (pw);
      m_nodeIdProxyWidgets[nodeId] = v;
      return;
    }
  ProxyWidgetVector_t & v = m_nodeIdProxyWidgets[nodeId];
  v.push_back (pw);
}

void
InterfaceStatsScene::add (uint32_t nodeId, QString pointADescription, uint32_t otherNodeId, QString pointBDescription, QString linkDescription)
{
  if (!StatsMode::getInstance ()->isNodeActive (nodeId))
    {
      return;
    }
  if (!pointADescription.contains ("~"))
    {
      return;
    }
  showInfoWidget (false);
  QStringList parts = pointADescription.split ('~');
  //qDebug (pointADescription);
  QString IP = "\n";
  AnimNode::Ipv4Set_t ipv4Addresses = AnimNodeMgr::getInstance ()->getNode (nodeId)->getIpv4Addresses ();
  for (AnimNode::Ipv4Set_t::const_iterator i = ipv4Addresses.begin ();
       i != ipv4Addresses.end ();
       ++i)
    {
        IP += "\t" + *i + "\n";
    }


  QString IPv6 = "\n";
  AnimNode::Ipv6Set_t ipv6Addresses = AnimNodeMgr::getInstance ()->getNode (nodeId)->getIpv6Addresses ();
  for (AnimNode::Ipv6Set_t::const_iterator i = ipv6Addresses.begin ();
       i != ipv6Addresses.end ();
       ++i)
    {
        IPv6 += "\t" + *i + "\n";
    }

  //qDebug (IP);
  QString MAC = parts.at (1);
  QString otherIP = "";
  QString otherMAC = "";

  if (pointBDescription != "")
    {
      parts = pointBDescription.split ('~');
      otherIP = parts.at (0);
      otherMAC = parts.at (1);
    }

  QString title = "Node:" + QString::number (nodeId);
  QString content = "IP:" + IP ;
  content += "IPv6:" + IPv6;
  content += "\nMAC:\n" + MAC ;

  if (pointBDescription != "")
    {
      content += "\n\nOther Node:" + QString::number (otherNodeId) ;
      content += "\nOther Node IP:" + otherIP;
      content += "\nOther Node MAC:\n" + otherMAC;
      content += "\nInfo:\n" + linkDescription;
    }
  TextBubble * tb = new TextBubble (title, content);
  QGraphicsProxyWidget * pw = addWidget (tb);
  QFont f (tb->font ());
  f.setPointSizeF (StatsMode::getInstance ()->getCurrentFontSize ());
  tb->setFont (f);
  QFontMetrics fm (f);
  pw->setMaximumHeight (fm.height () * tb->text ().count ("\n"));

  addToProxyWidgetsMap (nodeId, pw);
  qreal newX = m_lastX + pw->widget ()->width ();
  m_currentMaxHeight = qMax (m_currentMaxHeight, pw->size ().height ());
  if (newX >= sceneRect ().right ())
    {
      m_lastX = 0;
      m_lastY += m_currentMaxHeight + INTERSTATS_SPACE;
      m_currentMaxHeight = 0;
    }
  pw->setPos (m_lastX, m_lastY);
  m_lastX = pw->pos ().x () + pw->widget ()->width () + INTERSTATS_SPACE;
  m_lastY = pw->pos ().y ();
  m_bottomY = m_lastY + m_currentMaxHeight;
  //qDebug (QString ("Last X" + QString::number (m_lastX) + " w:" + QString::number (pw->widget ()->width ())));
  adjustRect ();

}

void
InterfaceStatsScene::clearProxyWidgetsMap ()
{
  showInfoWidget ();
  for (NodeIdProxyWidgetVectorMap_t::const_iterator i = m_nodeIdProxyWidgets.begin ();
      i != m_nodeIdProxyWidgets.end ();
      ++i)
    {
      ProxyWidgetVector_t v = i->second;
      for (ProxyWidgetVector_t::const_iterator j = v.begin ();
          j != v.end ();
          ++j)
        {
          removeItem (*j);
          delete (*j);
        }
      v.clear ();
    }
  m_nodeIdProxyWidgets.clear ();
}

void
InterfaceStatsScene::adjustRect ()
{
  QRectF currentRect = sceneRect ();
  QRectF newRect = QRectF (currentRect.topLeft (), QPointF (currentRect.bottomRight ().x (), m_bottomY));
  setSceneRect (newRect);
}

void
InterfaceStatsScene::test ()
{
  for (uint32_t i=0; i < 100; ++i)
    {
      //qDebug (sceneRect (), "Scene Rect");
      add (i, "10.1.1.1~00:00:00:00:00:06", i+1, "10.1.1.1~00:00:00:00:00:06", "lp.linkDescription");
    }
}

void
InterfaceStatsScene::systemReset ()
{
  m_dirty = true;
  m_lastX = 0;
  m_lastY = 0;
  m_bottomY = 0;
}

void
InterfaceStatsScene::showInfoWidget (bool show)
{
  m_infoWidget->setVisible (show);
  m_infoWidget->setPos (sceneRect ().width ()/2, sceneRect ().height ()/2);
}



void
InterfaceStatsScene::reloadContent (bool force)
{
  if (!m_dirty && !force)
    {
      return;
    }
  m_lastX = 0;
  m_lastY = 0;
  m_bottomY = 0;
  clearProxyWidgetsMap ();

  typedef std::vector <LinkProperty_t> LinkPropertyVector_t;
  typedef std::map <uint32_t, LinkPropertyVector_t> NodeIdLinkPropertyMap_t;

  NodeIdLinkPropertyMap_t flatMap ;
  for (LinkManager::NodeIdAnimLinkVectorMap_t::const_iterator i = LinkManager::getInstance ()->getLinks ()->begin ();
       i != LinkManager::getInstance ()->getLinks ()->end ();
       ++i) // 1
    {

      LinkManager::AnimLinkVector_t v = i->second;
      for (LinkManager::AnimLinkVector_t::const_iterator j = v.begin ();
          j != v.end ();
          ++j)
        {
          m_dirty = false;
          showInfoWidget (false);
          AnimLink * pLink = *j;
          LinkProperty_t link = {pLink->m_toId, "", "", ""};
          LinkProperty_t reverseLink = {i->first, "", "", ""};

          if (pLink->m_pointADescription)
            {
              link.pointADescription = *pLink->m_pointADescription;
              reverseLink.pointBDescription = *pLink->m_pointADescription;
            }
          if (pLink->m_pointBDescription)
            {
              link.pointBDescription = *pLink->m_pointBDescription;
              reverseLink.pointADescription = *pLink->m_pointBDescription;
            }
          if (pLink->m_currentLinkDescription)
            {
              link.linkDescription = *pLink->m_currentLinkDescription;
              reverseLink.linkDescription = *pLink->m_currentLinkDescription;
            }

          if (flatMap.find (i->first) == flatMap.end ())
            {
              LinkPropertyVector_t lpv;
              lpv.push_back (link);
              flatMap[i->first] = lpv;
            }
          else
            {
              LinkPropertyVector_t & lpv = flatMap[i->first];
              lpv.push_back (link);
            }
          if (pLink->m_toId == i->first)
            {
              continue;  //Wireless
            }

          if (flatMap.find (pLink->m_toId) == flatMap.end ())
            {
              LinkPropertyVector_t lpv;
              lpv.push_back (reverseLink);
              flatMap[pLink->m_toId] = lpv;
            }
          else
            {
              LinkPropertyVector_t & lpv = flatMap[pLink->m_toId];
              lpv.push_back (reverseLink);
            }
        }

    } // 1


  for (NodeIdLinkPropertyMap_t::const_iterator i = flatMap.begin ();
      i != flatMap.end ();
      ++i)
    {
      uint32_t fromNodeId = i->first;
      LinkPropertyVector_t lpv = i->second;
      for (LinkPropertyVector_t::const_iterator j = lpv.begin ();
          j != lpv.end ();
          ++j)
        {
          LinkProperty_t lp = *j;
          //qDebug (QString::number (fromNodeId) + ":" +QString::number (lp.toId) + ":" + lp.pointADescription + lp.pointBDescription + lp.linkDescription);
          InterfaceStatsScene::getInstance ()->add (fromNodeId, lp.pointADescription, lp.toId, lp.pointBDescription, lp.linkDescription);
        }

    }

}

} // namespace netanim
