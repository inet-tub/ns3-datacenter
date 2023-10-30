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

#ifndef GRAPHPACKET_H
#define GRAPHPACKET_H
#include "common.h"

namespace netanim {
class GraphPacket: public QGraphicsLineItem
{
public:
  GraphPacket (QPointF fromNodePos, QPointF toNodePos);
  void paint (QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
 // QRectF boundingRect ();
 // QPainterPath shape () const;
private:
  QPointF m_fromNodePos;
  QPointF m_toNodePos;
  QRectF m_boundingRect;
  QPainterPath m_shape;

};
}
#endif // GRAPHPACKET_H
