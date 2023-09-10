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

#ifndef ANIMATORVIEW_H
#define ANIMATORVIEW_H

#include "common.h"
#include "animatorscene.h"


namespace netanim
{

class AnimatorView : public QGraphicsView
{
public:

  static AnimatorView * getInstance ();
  void systemReset ();
  void fitSceneWithinView ();
  void postParse ();
  void setCurrentZoomFactor (qreal factor);


protected:
  void paintEvent (QPaintEvent * event);
  void wheelEvent (QWheelEvent *event);


private:
  explicit AnimatorView (QGraphicsScene *);
  AnimatorScene * getAnimatorScene ();
  void updateTransform ();
  qreal m_currentZoomFactor;

signals:

public slots:


};
} // namespace netanim
#endif // ANIMATORVIEW_H
