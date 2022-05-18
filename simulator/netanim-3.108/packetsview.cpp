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

#include "packetsview.h"
#include "packetsscene.h"
#include "logqt.h"

namespace netanim {

NS_LOG_COMPONENT_DEFINE ("PacketsView");
PacketsView * pPacketsView = 0;
PacketsView::PacketsView ():
  QGraphicsView (PacketsScene::getInstance ())
{
  setRenderHint (QPainter::Antialiasing);
  setViewportUpdateMode (BoundingRectViewportUpdate);
}

PacketsView *
PacketsView::getInstance ()
{
  if (!pPacketsView)
    {
      pPacketsView = new PacketsView;
    }
  return pPacketsView;
}

void
PacketsView::test()
{
}

void
PacketsView::postParse ()
{

  return;
}

void
PacketsView::wheelEvent(QWheelEvent *event)
{
  QGraphicsView::wheelEvent(event);
}

void
PacketsView::zoomIn ()
{
  scale (1.1, 1.1);
}

void
PacketsView::zoomOut ()
{
  scale (0.9, 0.9);
}


}
