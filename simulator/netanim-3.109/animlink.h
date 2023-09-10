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
#ifndef ANIMLINK_H
#define ANIMLINK_H

#include "common.h"
namespace netanim
{

class AnimLink: public QGraphicsLineItem
{
public:
  AnimLink (uint32_t fromId, uint32_t toId,
           QString pointADescription = "", QString pointBDescription = "",
           QString linkDescription = "", bool p2p = true);

  ~AnimLink ();
  uint32_t m_fromId;
  uint32_t m_toId;
  QString * m_pointADescription;
  QString * m_pointBDescription;
  QString * m_currentLinkDescription;
  bool    m_p2p;

  void updateCurrentLinkDescription (QString linkDescription);
  void resetCurrentLinkDescription ();
  QString toString ();
  QPointF getInterfacePosA ();
  QPointF getInterfacePosB ();
  QString getInterfaceADescription ();
  QString getInterfaceBDescription ();
  void repairLink ();
  bool isP2p ();

protected:
  void paint (QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
private:

  QString * m_originalLinkDescription;

  QPointF getLinkDescriptionCenter (QPainter *, QPointF *);
  QPointF m_interfacePosA;
  QPointF m_interfacePosB;

};

class LinkManager
{
public:
  typedef QVector <AnimLink *> AnimLinkVector_t;
  typedef std::map <uint32_t, AnimLinkVector_t> NodeIdAnimLinkVectorMap_t;
  static LinkManager * getInstance ();
  AnimLink * addLink (uint32_t fromId, uint32_t toId,
                     QString pointADescription,
                     QString pointBDescription, QString linkDescription, bool p2p = true);


  NodeIdAnimLinkVectorMap_t * getLinks ();
  AnimLink * getAnimLink (uint32_t fromId, uint32_t toId, bool p2p = true);
  void updateLink (uint32_t fromId, uint32_t toId, QString linkDescription);
  void repairLinks (uint32_t nodeId);
  void systemReset ();

private:
  LinkManager ();
  //AnimLinkVector_t             m_pointToPointLinks;
  NodeIdAnimLinkVectorMap_t    m_pointToPointLinks;

};



} // namespace netanim

#endif // ANIMLINK_H
