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

#include "animnode.h"
#include "animlink.h"

namespace netanim
{

LinkManager       * pLinkManager = 0;

AnimLink::AnimLink (uint32_t fromId, uint32_t toId,
                   QString pointADescription,
                   QString pointBDescription, QString linkDescription, bool p2p):
  m_fromId (fromId), m_toId (toId), m_p2p (p2p)
{
  m_pointADescription = 0;
  m_pointBDescription = 0;
  m_currentLinkDescription = 0;

  if (!m_p2p)
    {

      m_toId = m_fromId;
    }
  QLineF line (AnimNodeMgr::getInstance ()->getNode (m_fromId)->getCenter (),
               AnimNodeMgr::getInstance ()->getNode (m_toId)->getCenter ());
  setLine (line);

  if (pointADescription != "")
    {
      m_pointADescription = new QString (pointADescription);
      QStringList parts = (*m_pointADescription).split ('~');
      if (parts.count () == 2)
        {
          if (!parts.at (0).contains (":"))
            {
              AnimNodeMgr::getInstance ()->addIpv4Address (fromId, parts.at (0));
            }
          else
            {
              AnimNodeMgr::getInstance ()->addIpv6Address (fromId, parts.at (0));
            }

          AnimNodeMgr::getInstance ()->addMacAddress (fromId, parts.at (1));
        }

    }
  if (pointBDescription != "")
    {
      m_pointBDescription = new QString (pointBDescription);
      QStringList parts = (*m_pointBDescription).split ('~');
      if (parts.count () == 2)
        {
          if (!parts.at (0).contains (":"))
            {
              AnimNodeMgr::getInstance ()->addIpv4Address (toId, parts.at (0));
            }
          else
            {
              AnimNodeMgr::getInstance ()->addIpv6Address (toId, parts.at (0));
            }
          AnimNodeMgr::getInstance ()->addMacAddress (toId, parts.at (1));
        }
    }
  m_originalLinkDescription = new QString ("");
  if (linkDescription != "")
    {
      m_currentLinkDescription = new QString (linkDescription);
      *m_originalLinkDescription = linkDescription;
    }
  setZValue (ANIMLINK_ZVALUE);
  //setVisible (false);
}

AnimLink::~AnimLink ()
{
  if (m_pointADescription)
    delete m_pointADescription;
  if (m_pointBDescription)
    delete m_pointBDescription;
  if (m_currentLinkDescription)
    delete m_currentLinkDescription;
  if (m_originalLinkDescription)
    delete m_originalLinkDescription;

}

bool
AnimLink::isP2p ()
{
  return m_p2p;
}

void
AnimLink::repairLink ()
{
  if (!m_p2p)
    return;
  QLineF line (AnimNodeMgr::getInstance ()->getNode (m_fromId)->getCenter (),
               AnimNodeMgr::getInstance ()->getNode (m_toId)->getCenter ());
  setLine (line);
}

QPointF
AnimLink::getLinkDescriptionCenter (QPainter * painter , QPointF * offset)
{
  QFontMetrics fm = painter->fontMetrics ();
  qreal x = (line ().length () - fm.horizontalAdvance (GET_DATA_PTR (m_currentLinkDescription)))/2;
  QPointF pOffset = line ().p1 ().x () < line ().p2 ().x ()? line ().p1 ():line ().p2 ();
  *offset = pOffset;
  QPointF p = QPointF (x, -1);
  return p;
}

void
AnimLink::paint (QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
  Q_UNUSED (option);
  Q_UNUSED (widget);
  QFont font;
  font.setPointSize (2);
  QPen pen;
  pen.setCosmetic (true);
  QColor bl(0, 0, 0, 50);
  pen.setColor (bl);
  painter->setFont (font);
  painter->setPen (pen);

  painter->drawLine (line ());
  bl = QColor (0, 0, 0);
  pen.setColor (bl);
  pen.setCosmetic (true);
  painter->setPen (pen);

  if (m_currentLinkDescription)
    {
      QPointF offset;
      QPointF center = getLinkDescriptionCenter (painter, &offset);
      painter->save ();
      painter->translate (offset);

      if (offset != line ().p1 ())
        {
          painter->rotate (180-line ().angle ());
        }
      else
        {
          painter->rotate (-line ().angle ());
        }
      painter->drawText (center, *m_currentLinkDescription);
      painter->restore ();
    }

  if (!m_p2p)
    {
      m_interfacePosA = line ().p1 ();
    }

  QList <QGraphicsItem *> collidingList = collidingItems ();
  for (QList <QGraphicsItem *>::const_iterator i = collidingList.begin ();
       i != collidingList.end ();
       ++i)
    {
      QGraphicsItem * item = *i;
      if (item->type () == ANIMNODE_TYPE)
        {
          AnimNode * node = qgraphicsitem_cast <AnimNode *> (item);
          qreal radius = node->sceneBoundingRect ().width ()/2;
          QPointF center = node->getCenter ();


          QPointF other;
          if ( (center.x () == line ().x1 ()) && (center.y () == line ().y1 ()))
            {
              other = line ().p2 ();
              QLineF l (center, other);
              if (!m_p2p)
                {
                  l = QLineF (center, QPointF (center.x (), 0));
                }

              l.setLength (radius);
              m_interfacePosA = l.p1 ();
            }
          else
            {
              other = line ().p1 ();
              QLineF l (center, other);
              l.setLength (radius);
              m_interfacePosB = l.p2 ();
            }


        }
    }
}


void
AnimLink::updateCurrentLinkDescription (QString linkDescription)
{
  if (!m_currentLinkDescription)
    {
      m_currentLinkDescription = new QString (linkDescription);
      return;
    }
  *m_currentLinkDescription = linkDescription;
}

void
AnimLink::resetCurrentLinkDescription ()
{
  if (m_originalLinkDescription)
    {
      m_currentLinkDescription = m_originalLinkDescription;
    }
}

QString
AnimLink::toString ()
{
  QString s = QString ("From:") + QString::number (m_fromId) + " To:" + QString::number (m_toId);
  return s;
}

QPointF
AnimLink::getInterfacePosA ()
{
  return m_interfacePosA;
}

QPointF
AnimLink::getInterfacePosB ()
{
  return m_interfacePosB;
}

QString
AnimLink::getInterfaceADescription ()
{
  if (m_pointADescription)
    {
      return *m_pointADescription;
    }
  else
    {
      return "";
    }

}

QString
AnimLink::getInterfaceBDescription ()
{
  if (m_pointBDescription)
    {
      return *m_pointBDescription;
    }
  else
    {
      return "";
    }
}


LinkManager::LinkManager ()
{

}

LinkManager *
LinkManager::getInstance ()
{
  if (!pLinkManager)
    {
      pLinkManager = new LinkManager;
    }
  return pLinkManager;
}

AnimLink *
LinkManager::addLink (uint32_t fromId, uint32_t toId, QString pointADescription, QString pointBDescription, QString linkDescription, bool p2p)
{
  AnimLink * item = new AnimLink (fromId, toId, pointADescription, pointBDescription, linkDescription, p2p);
  if (m_pointToPointLinks.find (fromId) == m_pointToPointLinks.end ())
    {
      LinkManager::AnimLinkVector_t v;
      v.push_back (item);
      m_pointToPointLinks[fromId] = v;
      return item;
    }
  else
    {
      LinkManager::AnimLinkVector_t & v = m_pointToPointLinks[fromId];
      v.push_back (item);
      return item;
    }
}

LinkManager::NodeIdAnimLinkVectorMap_t *
LinkManager::getLinks ()
{
  return &m_pointToPointLinks;
}

AnimLink *
LinkManager::getAnimLink (uint32_t fromId, uint32_t toId, bool p2p)
{
  AnimLink * theLink = 0;
  for (LinkManager::NodeIdAnimLinkVectorMap_t::const_iterator i = m_pointToPointLinks.begin ();
      i != m_pointToPointLinks.end ();
      ++i)
    {
      if (fromId != i->first)
        {
          continue;
        }
      LinkManager::AnimLinkVector_t v = i->second;
      for (LinkManager::AnimLinkVector_t::const_iterator j = v.begin ();
          j != v.end ();
          ++j)
        {
          AnimLink * link = *j;
          if (!p2p)
            {
              if (!link->isP2p ())
                return link;
            }
          if ( (link->m_fromId == fromId && link->m_toId == toId) ||
              (link->m_fromId == toId && link->m_toId == fromId))
            return link;
        }
    }
  return theLink;

}

void
LinkManager::updateLink (uint32_t fromId, uint32_t toId, QString linkDescription)
{
  AnimLink * animLink = getAnimLink (fromId, toId);
  if (animLink)
    {
      animLink->updateCurrentLinkDescription (linkDescription);
    }
  //animLink-
}

void
LinkManager::systemReset ()
{
  // remove links
  m_pointToPointLinks.clear ();

}

void
LinkManager::repairLinks (uint32_t nodeId)
{
  for (LinkManager::NodeIdAnimLinkVectorMap_t::const_iterator i = m_pointToPointLinks.begin ();
      i != m_pointToPointLinks.end ();
      ++i)
    {
      LinkManager::AnimLinkVector_t v = i->second;
      for (LinkManager::AnimLinkVector_t::const_iterator j = v.begin ();
          j != v.end ();
          ++j)
        {
          AnimLink * animLink = *j;
          if ((animLink->m_fromId == nodeId) || (animLink->m_toId == nodeId))
            {
              animLink->repairLink ();
            }
        }
    }

}



} // namespace netanim
