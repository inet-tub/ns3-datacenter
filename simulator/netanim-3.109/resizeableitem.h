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

#ifndef RESIZEABLEITEM_H
#define RESIZEABLEITEM_H

#include "common.h"
#include "animatorconstants.h"

#define PIXMAP_RESIZING_BORDER 5
#define PIXMAP_WIDTH_MIN 20

class ResizeableItem : public QGraphicsItem
{

public:
  typedef enum
  {
    RESIZE_RIGHT,
    RESIZE_LEFT,
    RESIZE_TOP,
    RESIZE_BOTTOM,
    RESIZE_NOTRESIZING
  } ResizeDirection_t;
  typedef enum
  {
    RECTANGLE,
    CIRCLE,
    PIXMAP
  } ResizeableItemType_t;
  enum { Type = ANIMNODE_TYPE };
  int type () const
  {
    return ResizeableItem::Type;
  }
  ResizeableItem ();
  ~ResizeableItem ();
  QRectF boundingRect () const;
  QPainterPath shape() const;
  qreal getItemWidth ();
  qreal getItemHeight ();
  qreal getBorderWidth ();
  void setSize (qreal width, qreal height);
  void paint (QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
  void setPixmap (QPixmap pix);
  void setType (ResizeableItemType_t t);
  void setWidth (qreal width);
  void setHeight (qreal height);
  void setColor (uint8_t r, uint8_t g, uint8_t b, uint8_t alpha = 255);

protected:
  ResizeableItemType_t m_type;
  uint8_t m_r;
  uint8_t m_g;
  uint8_t m_b;
  uint8_t m_alpha;
  QPixmap * m_pixmap;

protected:
  qreal m_width;
  qreal m_height;


};

#endif // RESIZEABLEITEM_H
