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


#include "animatorview.h"

namespace netanim
{

NS_LOG_COMPONENT_DEFINE ("AnimatorView");
AnimatorView * pAnimatorView = 0;

AnimatorView::AnimatorView (QGraphicsScene * scene) :
  QGraphicsView (scene),
  m_currentZoomFactor (1)

{
  setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);
  setViewportUpdateMode (BoundingRectViewportUpdate);
}

AnimatorView *
AnimatorView::getInstance ()
{
  if (!pAnimatorView)
    {
      pAnimatorView = new AnimatorView (AnimatorScene::getInstance ());
    }
  return pAnimatorView;
}

void
AnimatorView::paintEvent (QPaintEvent *event)
{
  //qDebug (transform);
  try
    {
      QGraphicsView::paintEvent (event);
    }
  catch (...)
    {

    }
}

AnimatorScene *
AnimatorView::getAnimatorScene ()
{
  return  AnimatorScene::getInstance ();
}

void
AnimatorView::updateTransform ()
{
  QTransform transform;
  QRectF sceneBoundaryRect = AnimatorScene::getInstance ()->getBoundaryRect ();

  qreal minDimension = qMin (sceneBoundaryRect.width (), sceneBoundaryRect.height ());

  qreal xScale = viewport ()->width ()/minDimension;
  qreal yScale = viewport ()->height ()/minDimension;
  //qDebug (width (), "Width");
  //qDebug (height (), "height");
  qreal minScale = qMin (xScale, yScale);
  transform.scale (minScale, minScale);
  setTransform (transform);

}

void
AnimatorView::setCurrentZoomFactor (qreal factor)
{
  if (m_currentZoomFactor < factor)
    {
      scale (1.1, 1.1);
    }
  else
    {
      scale (0.9, 0.9);
    }
  m_currentZoomFactor = factor;
  //update ();
}

void
AnimatorView::wheelEvent (QWheelEvent *event)
{
  QGraphicsView::wheelEvent (event);
  update ();
}

void
AnimatorView::fitSceneWithinView ()
{
  //QGraphicsView::fitInView (sceneRect ());
  updateTransform ();

}

void
AnimatorView::systemReset ()
{
  m_currentZoomFactor = 1;

  resetTransform ();
}

void
AnimatorView::postParse ()
{

  fitSceneWithinView ();
}


} // namespace netanim

