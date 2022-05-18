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
 * Contributions: Makhtar Diouf <makhtar.diouf@gmail.com>
 */

#include "resizeableitem.h"

NS_LOG_COMPONENT_DEFINE ("ResizeableItem");

ResizeableItem::ResizeableItem ():
  m_type (ResizeableItem::CIRCLE),
  m_r (255),
  m_g (0),
  m_b (0),
  m_alpha (240),
  m_pixmap (0),
  m_width (1),
  m_height (1)
{
  //NS_LOG_FUNCTION (m_mousePressed);
  setAcceptHoverEvents (true);  // Typo fixed
}

ResizeableItem::~ResizeableItem ()
{
  if (m_pixmap)
    {
      delete m_pixmap;
    }

}

void ResizeableItem::setPixmap (QPixmap pix)
{
  m_pixmap = new QPixmap (pix);
  setType (ResizeableItem::PIXMAP);
}

void ResizeableItem::setType (ResizeableItemType_t t)
{
  m_type = t;
}


void
ResizeableItem::setColor (uint8_t r, uint8_t g, uint8_t b, uint8_t alpha)
{
  m_r = r;
  m_g = g;
  m_b = b;
  m_alpha = alpha;
}
void
ResizeableItem::setWidth (qreal width)
{
  m_width = width;
  update ();
}


void
ResizeableItem::setHeight (qreal height)
{
  m_height = height;
  update ();

}

void ResizeableItem::setSize (qreal width, qreal height)
{
  setWidth (width);
  setHeight (height);
  qreal xScale = width/sceneBoundingRect ().width ();
  qreal yScale = height/sceneBoundingRect ().height ();
  setTransform (QTransform::fromScale (xScale, yScale), true);
}

void ResizeableItem::paint (QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
  Q_UNUSED (option)
  Q_UNUSED (widget)
  switch (m_type)
    {
    case ResizeableItem::RECTANGLE:
      painter->drawRect (0, 0, m_width, m_height);
      break;
    case ResizeableItem::CIRCLE:
    {
      QPen pen;
      pen.setCosmetic (true);
      QBrush brush;
      brush.setStyle (Qt::SolidPattern);
      brush.setColor (QColor (m_r, m_g, m_b, m_alpha));
      painter->setPen (pen);
      painter->setBrush (brush);
      painter->drawEllipse (QPointF (0, 0), m_width, m_height);

    }
    break;
    case ResizeableItem::PIXMAP:
      if (m_pixmap)
        {
          painter->drawPixmap (-m_width/2, -m_height/2, m_width, m_height, *m_pixmap);
        }
      break;
    }
}



qreal ResizeableItem::getItemWidth ()
{
  return sceneBoundingRect ().width ();
}

qreal ResizeableItem::getItemHeight ()
{
  return sceneBoundingRect ().height ();
}


QPainterPath
ResizeableItem::shape () const
{
  QPainterPath path;
  switch (m_type)
    {
      case ResizeableItem::CIRCLE:
        path.addEllipse (QPointF (0, 0), m_width, m_height);
        break;
      case ResizeableItem::PIXMAP:
        path.addRect (-m_width/2, -m_height/2, m_width, m_height);
        break;
      default:
        path.addRect (QRectF (0, 0, m_width, m_height));
        break;
    }
  return path;
}



QRectF ResizeableItem::boundingRect () const
{
  QPainterPath path;

  switch (m_type)
    {
      case ResizeableItem::CIRCLE:
        path.addEllipse (QPointF (0, 0), m_width, m_height);
        break;
      case ResizeableItem::PIXMAP:
        path.addRect (-m_width/2, -m_height/2, m_width, m_height);
        break;
      default:
        path.addRect (0, 0, m_width, m_height);
        break;
    }
  return path.boundingRect();
}



