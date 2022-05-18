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
 * Contributions: Dmitrii Shakshin <d.shakshin@gmail.com> (Open Source and Linux Laboratory http://dev.osll.ru/)
 *                Makhtar Diouf <makhtar.diouf@gmail.com>
 */


#include "animatorscene.h"
#include "animatorview.h"
#include "animatormode.h"
#include "animpropertybrowser.h"
#include "logqt.h"

namespace netanim
{

NS_LOG_COMPONENT_DEFINE ("AnimatorScene");
AnimatorScene * pAnimatorScene = 0;

AnimatorScene::AnimatorScene ():
  QGraphicsScene (0, 0, ANIMATORSCENE_USERAREA_WIDTH, ANIMATORSCENE_USERAREA_WIDTH),
  m_backgroundImage (0),
  m_enableMousePositionLabel (false)
{
  m_mousePositionLabel = new QLabel ("");
  m_mousePositionLabel->setSizePolicy (QSizePolicy::Minimum, QSizePolicy::Minimum);
  m_mousePositionProxyWidget = addWidget (m_mousePositionLabel, Qt::ToolTip);
  m_mousePositionProxyWidget->setFlag (QGraphicsItem::ItemIgnoresTransformations);
  m_nGridLines = GRID_LINES_DEFAULT;
  m_showGrid = true;

  m_sceneInfoText = new QGraphicsSimpleTextItem;
  m_sceneInfoText->setFlag(QGraphicsItem::ItemIgnoresTransformations);
  addItem(m_sceneInfoText);

  initGridCoordinates ();
}


AnimatorScene *
AnimatorScene::getInstance ()
{
  if (!pAnimatorScene)
    {
      pAnimatorScene = new AnimatorScene;
    }
  return pAnimatorScene;
}

void
AnimatorScene::systemReset ()
{
  purgeNodeTrajectories ();
  purgeAnimatedPackets ();
  purgeAnimatedNodes ();
  purgeAnimatedLinks ();
  resetInterfaceTexts ();
  setSceneRect (0, 0, ANIMATORSCENE_USERAREA_WIDTH, ANIMATORSCENE_USERAREA_WIDTH);
  resetGrid ();
  if (m_backgroundImage)
    {
      removeItem (m_backgroundImage);
      delete (m_backgroundImage);
      m_backgroundImage = 0;
    }
}

void AnimatorScene::testSlot ()
{

}

void
AnimatorScene::setSimulationBoundaries (QPointF minPoint, QPointF maxPoint)
{
  m_minPoint = minPoint;
  m_maxPoint = maxPoint;
  qreal boundaryWidth = m_maxPoint.x () * 0.1;
  qreal boundaryHeight = m_maxPoint.y () * 0.2;
  qreal boundary = qMax (boundaryWidth, boundaryHeight);

  m_sceneMinPoint = QPointF (m_minPoint.x () - boundary, m_minPoint.y () - boundary);
  m_sceneMaxPoint = QPointF (m_maxPoint.x () + boundary, m_maxPoint.y () + boundary);

  // Make it square
  qreal minimum = qMin (m_sceneMinPoint.x (), m_sceneMinPoint.y ());
  m_sceneMinPoint = QPointF (minimum, minimum);
  qreal maximum = qMax (m_sceneMaxPoint.x (), m_sceneMaxPoint.y ());
  m_sceneMaxPoint = QPointF (maximum, maximum);
  if ((m_sceneMaxPoint.x () == 0) && (m_sceneMaxPoint.y () == 0))
    {
      m_sceneMaxPoint = QPointF (1, 1);
    }

  setSceneRect (QRectF (m_sceneMinPoint, m_sceneMaxPoint));
}

void
AnimatorScene::enableMousePositionLabel (bool enable)
{
  m_enableMousePositionLabel = enable;
}


void
AnimatorScene::setBackgroundX (qreal x)
{
  if (!m_backgroundImage)
    return;
  m_backgroundImage->setX(x);
}

void
AnimatorScene::setBackgroundY (qreal y)
{
  if (!m_backgroundImage)
    return;
  m_backgroundImage->setY(y);
}

void AnimatorScene::setScale (QGraphicsPixmapItem* img, qreal x, qreal y)
{
  img->setTransform (QTransform::fromScale (x, y), true);
}

void
AnimatorScene::setBackgroundScaleX (qreal x)
{
  if (!m_backgroundImage)
    return;
  //m_backgroundImage->setTransform (m_originalBackgroundTransform);
  setScale (m_backgroundImage, x, 1);
}


void
AnimatorScene::setBackgroundScaleY (qreal y)
{
  if (!m_backgroundImage)
    return;
  //m_backgroundImage->setTransform (m_originalBackgroundTransform);
  setScale (m_backgroundImage, 1, y);
}

void
AnimatorScene::setBackgroundOpacity (qreal opacity)
{
  if (!m_backgroundImage)
    return;
  m_backgroundImage->setOpacity (opacity);
}

QGraphicsPixmapItem *
AnimatorScene::getBackgroundImage ()
{
  return m_backgroundImage;
}

void
AnimatorScene::setBackgroundImage (QString fileName, qreal x, qreal y, qreal scaleX, qreal scaleY, qreal opacity)
{

  QPixmap pix (fileName);
  if (pix.isNull ())
    {
      AnimatorMode::getInstance ()->showPopup ("Failed to load background image:" + fileName);
      return;
    }
  if (m_backgroundImage)
    {
      removeItem (m_backgroundImage);
      delete m_backgroundImage;
      m_backgroundImage = 0;
    }
  m_backgroundImage = new QGraphicsPixmapItem;
  m_backgroundImage->setPixmap (pix);
  addItem (m_backgroundImage);
  m_backgroundImage->setPos (x, y);
  m_backgroundImage->setFlags (QGraphicsItem::ItemIsMovable|QGraphicsItem::ItemIsSelectable);
  m_originalBackgroundTransform = m_backgroundImage->transform ();
  setScale (m_backgroundImage, scaleX, scaleY); // scale (scaleX, scaleY);
  m_backgroundImage->setZValue (ANIMBACKGROUND_ZVALUE);
  m_backgroundImage->setOpacity (opacity);

}

void
AnimatorScene::setSceneInfoText(QString text, bool show)
{
    m_sceneInfoText->setText (text);
    m_sceneInfoText->setVisible (show);
    QFontMetrics fm (font ());
    QRectF r = sceneRect ();
    QPointF pos = QPointF ((sceneRect ().width () - fm.width (text))/2, r.center ().y ());
    m_sceneInfoText->setPos (pos);
}



void
AnimatorScene::setShowNodeTrajectory (AnimNode *animNode)
{
  uint32_t nodeId = animNode->getNodeId ();
  if (m_nodeTrajectory.find (nodeId) == m_nodeTrajectory.end ())
    {
      QPainterPath path;
      AnimNodeMgr::TimePosVector_t positions =  AnimNodeMgr::getInstance ()->getPositions (animNode->getNodeId ());
      for (AnimNodeMgr::TimePosVector_t::const_iterator i = positions.begin ();
          i != positions.end ();
          ++i)
        {
          TimePosition_t tp = *i;
          QPointF pt = tp.p;
          path.moveTo (pt);
          break;
        }


      positions =  AnimNodeMgr::getInstance ()->getPositions (animNode->getNodeId ());
      for (AnimNodeMgr::TimePosVector_t::const_iterator i = positions.begin ();
          i != positions.end ();
          ++i)
        {
          TimePosition_t tp = *i;
          QPointF pt = tp.p;
          path.lineTo (pt);
        }
      QGraphicsPathItem * pathItem = addPath (path);
      QPen pen;
      pen.setCosmetic (true);
      pathItem->setPen (pen);
      m_nodeTrajectory[nodeId] = pathItem;
    }
  m_nodeTrajectory[nodeId]->setVisible (animNode->getShowNodeTrajectory ());
}

void
AnimatorScene::addWirelessCircle (QRectF r)
{
  //NS_LOG_DEBUG ("WirelessCircles Rect:" << r);
  if (r.width() == 0)
    return;
  AnimWirelessCircles * w = new AnimWirelessCircles ();
  QPen p = w->pen ();
  p.setCosmetic (true);
  p.setColor (QColor (0, 0, 255, 50));
  w->setPen (p);
  w->setRect (r);
  addItem (w);
  m_animatedWirelessCircles.push_back (w);
}

void
AnimatorScene::purgeAnimatedNodes ()
{
  for (QVector <AnimNode*>::const_iterator i = m_animatedNodes.begin ();
      i != m_animatedNodes.end ();
      ++i)
    {
      AnimNode * animNode = *i;
      animNode->setVisible (false);
      QGraphicsTextItem * ti = animNode->getDescription ();
      removeItem (animNode);
      removeItem (ti);
      delete animNode;
    }
  m_animatedNodes.clear ();
  AnimNodeMgr::getInstance ()->systemReset ();

}

void
AnimatorScene::purgeNodeTrajectories ()
{
  for (NodeTrajectoryMap_t::const_iterator i = m_nodeTrajectory.begin ();
       i != m_nodeTrajectory.end ();
       ++i)
    {
      i->second->setVisible (false);
      removeItem (i->second);
    }
  m_nodeTrajectory.clear ();
}

void
AnimatorScene::purgeAnimatedLinks ()
{

  for (QVector <AnimLink *>::const_iterator i = m_animatedLinks.begin ();
      i != m_animatedLinks.end ();
      ++i)
    {
      AnimLink * animLink = *i;
      animLink->setVisible (false);
      removeItem (animLink);
      delete animLink;

    }
  m_animatedLinks.clear ();
  LinkManager::getInstance ()->systemReset ();

}


void
AnimatorScene::showAnimatedPackets (bool show)
{
  for (std::map <AnimPacket *, AnimPacket *>::const_iterator i = m_wirelessAnimatedPackets.begin ();
      i != m_wirelessAnimatedPackets.end ();
      ++i)
    {
      AnimPacket * p = i->first;
      p->setVisible (show);
    }
  for (std::map <AnimPacket *, AnimPacket *>::const_iterator i = m_wiredAnimatedPackets.begin ();
      i != m_wiredAnimatedPackets.end ();
      ++i)
    {
      AnimPacket * p = i->first;
      p->setVisible (show);
    }

  for (QVector <AnimWirelessCircles *>::const_iterator i = m_animatedWirelessCircles.begin ();
      i != m_animatedWirelessCircles.end ();
      ++i)
    {
      AnimWirelessCircles * w = *i;
      w->setVisible (show);
    }
}


void
AnimatorScene::purgeWirelessPackets ()
{
  for (std::map <AnimPacket *, AnimPacket *>::const_iterator i = m_wirelessAnimatedPackets.begin ();
      i != m_wirelessAnimatedPackets.end ();
      ++i)
    {
      AnimPacket * p = i->first;
      p->setVisible (false);
      removeItem (p->getInfoTextItem ());
      removeItem (p);
      delete p;
    }
  m_wirelessAnimatedPackets.clear ();
  for (QVector <AnimWirelessCircles *>::const_iterator i = m_animatedWirelessCircles.begin ();
      i != m_animatedWirelessCircles.end ();
      ++i)
    {
      AnimWirelessCircles * w = *i;
      w->setVisible (false);
      removeItem (w);
      delete w;
    }
  m_animatedWirelessCircles.clear ();

}
void
AnimatorScene::purgeAnimatedPackets ()
{
  for (std::map <AnimPacket *, AnimPacket *>::const_iterator i = m_wiredAnimatedPackets.begin ();
      i != m_wiredAnimatedPackets.end ();
      ++i)
    {
      AnimPacket * p = i->first;
      p->setVisible (false);
      removeItem (p->getInfoTextItem ());
      removeItem (p);
      delete p;
    }
  m_wiredAnimatedPackets.clear ();
  for (std::map <AnimPacket *, AnimPacket *>::const_iterator i = m_wirelessAnimatedPackets.begin ();
      i != m_wirelessAnimatedPackets.end ();
      ++i)
    {
      AnimPacket * p = i->first;
      p->setVisible (false);
      removeItem (p->getInfoTextItem ());
      removeItem (p);
      delete p;
    }
  m_wirelessAnimatedPackets.clear ();

  for (QVector <AnimWirelessCircles *>::const_iterator i = m_animatedWirelessCircles.begin ();
      i != m_animatedWirelessCircles.end ();
      ++i)
    {
      AnimWirelessCircles * w = *i;
      w->setVisible (false);
      removeItem (w);
      delete w;
    }
  m_animatedWirelessCircles.clear ();
}

void
AnimatorScene::addLink (AnimLink *animLink)
{
  addItem (animLink);
  m_animatedLinks.push_back (animLink);
}

void
AnimatorScene::addNode (AnimNode *animNode)
{
  addItem (animNode);
  m_animatedNodes.push_back (animNode);
  animNode->setPos (animNode->getX (), animNode->getY ());
  addItem (animNode->getDescription ());
  animNode->getDescription ()->setPos (animNode->sceneBoundingRect ().bottomRight ());
}

void
AnimatorScene::addWirelessPacket (AnimPacket *p)
{
  addItem (p);
  //p->getInfoTextItem ()->setPos (p->boundingRect ().bottomLeft ());
  m_wirelessAnimatedPackets[p] = p;
}


void
AnimatorScene::removeWiredPacket (AnimPacket *p)
{
  m_wiredAnimatedPackets.erase (p);
  p->setVisible (false);
  removeItem (p);
}


void
AnimatorScene::removeWirelessPacket (AnimPacket *p)
{
  p->setVisible (false);
  m_wirelessAnimatedPackets.erase (p);
  removeItem (p);
}


void
AnimatorScene::addWiredPacket (AnimPacket *p)
{
  addItem (p);
  //p->getInfoTextItem ()->setPos (p->boundingRect ().bottomLeft ());
  m_wiredAnimatedPackets[p] = p;
}


void
AnimatorScene::setMousePositionLabel (QPointF pos)
{

  //QString string = "    (" + QString::number (qRound (pos.x ())) + "," + QString::number (qRound (pos.y ())) + ")";
  QString string = "    (" + QString::number ( (pos.x ())) + "," + QString::number ( (pos.y ())) + ")";

  m_mousePositionLabel->setText (string);
  m_mousePositionProxyWidget->setPos (pos.x (), pos.y ());
  m_mousePositionLabel->adjustSize ();

}

void
AnimatorScene::showMousePositionLabel (bool show)
{
  m_mousePositionProxyWidget->setVisible (show);
}


void
AnimatorScene::mouseDoubleClickEvent (QGraphicsSceneMouseEvent *event)
{
  QList <QGraphicsItem *> list = items (event->scenePos ());
  foreach (QGraphicsItem * i, list)
    {
      if (i->type () == ANIMNODE_TYPE)
        {

          AnimNode * animNode = qgraphicsitem_cast <AnimNode *> (i);
          if (animNode)
            {
              AnimPropertyBroswer::getInstance ()->setCurrentNodeId (animNode->getNodeId ());
              AnimatorMode::getInstance ()->openPropertyBroswer ();
              break;
            }
        }
    }
}

void
AnimatorScene::mouseMoveEvent (QGraphicsSceneMouseEvent *event)
{
  if (m_enableMousePositionLabel)
    {
      QPointF scenePos = event->scenePos ();
    //   QString s = "Mouse:" + QString::number (event->scenePos ().x ()) + "," + QString::number (event->scenePos ().y ());
    //   qDebug (s.toAscii ().data ());
      setMousePositionLabel (scenePos);
      if ((scenePos.x () < 0) ||
          (scenePos.y () < 0))
        {
          showMousePositionLabel (false);
        }
      else
        {
          showMousePositionLabel (true);
        }
    }
  return QGraphicsScene::mouseMoveEvent (event);
}


QVector <QGraphicsSimpleTextItem *>
AnimatorScene::getGridCoordinatesItems ()
{
  return m_gridCoordinates;
}


void
AnimatorScene::initGridCoordinates ()
{
  for (int i = 0; i < m_gridCoordinates.size (); ++i)
    {
      QGraphicsSimpleTextItem * item = m_gridCoordinates[i];
      removeItem (item);
      delete item;
    }
  m_gridCoordinates.clear ();
  for (int i = 0; i < 9; i++) // only 9 coordinates will be marked
    {
      QGraphicsSimpleTextItem * item = new QGraphicsSimpleTextItem;
      item->setFlag (QGraphicsItem::ItemIgnoresTransformations);
      m_gridCoordinates.push_back (item);
      addItem (item);

    }
  markGridCoordinates ();

}

void
AnimatorScene::markGridCoordinates ()
{
  QRectF simulationRect (m_minPoint, m_maxPoint);
  if ((simulationRect.width () == 0) && (simulationRect.height () == 0))
    return;
  int i = 0;
  for (qreal x = 0; x <= simulationRect.right () ; x = x + (simulationRect.right ()/2))
    for (qreal y = 0; y <= simulationRect.bottom () ; y = y + (simulationRect.bottom ()/2))
      {
        if (i == 9)
          return;
        QString text = QString::number (x, 'f', 1)
                       + ","
                       + QString::number (y, 'f', 1);
        m_gridCoordinates[i]->setText (text);
        m_gridCoordinates[i]->setPos (QPointF (x, y));
        m_gridCoordinates[i]->setVisible (m_showGrid);
        i++;
      }

}

void
AnimatorScene::addGrid ()
{
  m_showGrid = true;
  qreal maximum = qMax (m_maxPoint.x (), m_maxPoint.y ());
  QRectF gridRect (QPointF (0, 0), QPointF (maximum, maximum));
  qreal xStep = (gridRect.right ())/ (m_nGridLines-1);
  qreal yStep = (gridRect.bottom ())/ (m_nGridLines-1);
  //xStep = ceil (xStep);
  //yStep = ceil (yStep);
  QPen pen (QColor (100, 100, 155, 125));
  pen.setCosmetic (true);

  // draw horizontal grid
  qreal y = 0;
  qreal x = 0;
  for (int c = 0; c < m_nGridLines; ++c, y += yStep)
    {
      m_gridLines.push_back (addLine (0, y, gridRect.right (), y, pen));
    }
  // now draw vertical grid
  for (int c = 0; c < m_nGridLines; ++c, x += xStep)
    {
      m_gridLines.push_back (addLine (x, 0, x,  gridRect.bottom (), pen));
    }
  initGridCoordinates ();
  markGridCoordinates ();


}

QRectF
AnimatorScene::getBoundaryRect ()
{
  return QRectF (m_sceneMinPoint, m_sceneMaxPoint);
}

void
AnimatorScene::setGridLinesCount (int nGridLines)
{
  m_nGridLines = nGridLines;
  bool showGrid = m_showGrid;
  resetGrid ();
  m_showGrid = showGrid;
  if (m_showGrid)
    {
      addGrid ();
    }
  update ();
}

void
AnimatorScene::resetGrid ()
{
  m_showGrid = false;
  for (LineItemVector_t::const_iterator i = m_gridLines.begin ();
      i != m_gridLines.end ();
      ++i)
    {

      removeItem (*i);
      delete (*i);
    }
  m_gridLines.clear ();

  for (GridCoordinatesVector_t::const_iterator i = m_gridCoordinates.begin ();
      i != m_gridCoordinates.end ();
      ++i)
    {
      removeItem (*i);
      delete (*i);
    }
  m_gridCoordinates.clear ();
}


void
AnimatorScene::setShowInterfaceTexts (bool showIp, bool showMac)
{
  resetInterfaceTexts ();
  m_showIpInterfaceTexts = showIp;
  m_showMacInterfaceTexts = showMac;
  if (!m_showIpInterfaceTexts && !m_showMacInterfaceTexts)
    {
      return;
    }
  if (!m_interfaceATexts.size ())
    {
      for (LinkManager::NodeIdAnimLinkVectorMap_t::const_iterator i = LinkManager::getInstance ()->getLinks ()->begin ();
          i != LinkManager::getInstance ()->getLinks ()->end ();
          ++i)
        {

          LinkManager::AnimLinkVector_t linkVector = i->second;

          for (LinkManager::AnimLinkVector_t::const_iterator j = linkVector.begin ();
              j != linkVector.end ();
              ++j)
            {
              AnimLink * animLink = *j;

              QString pointADescription = animLink->getInterfaceADescription ();
              QPointF pointApos = animLink->getInterfacePosA ();
              AnimInterfaceText * interfaceAText = new AnimInterfaceText (pointADescription);
              interfaceAText->setPos (pointApos);
              addItem (interfaceAText);
              m_interfaceATexts.push_back (interfaceAText);
              interfaceAText->setMode (m_showIpInterfaceTexts, m_showMacInterfaceTexts);

              QString pointBDescription = animLink->getInterfaceBDescription ();
              if (pointBDescription == "")
                {
                  continue;
                }
              QPointF pointBpos = animLink->getInterfacePosB ();
              AnimInterfaceText * interfaceBText = new AnimInterfaceText (pointBDescription, true);
              interfaceBText->setMode (m_showIpInterfaceTexts, m_showMacInterfaceTexts);
              addItem (interfaceBText);
              interfaceBText->setPos (pointBpos);
              m_interfaceBTexts.push_back (interfaceBText);
            }
        }
      update ();
      removeInterfaceTextCollision ();
      return;
    }
  QPen pen;
  pen.setCosmetic (true);
  for (AnimInterfaceTextVector_t::const_iterator i = m_interfaceATexts.begin ();
      i != m_interfaceATexts.end ();
      ++i)
    {
      AnimInterfaceText * interfaceText = *i;
      interfaceText->setMode (m_showIpInterfaceTexts, m_showMacInterfaceTexts);
      QGraphicsLineItem * l = interfaceText->getLine ();
      l->setPen (pen);
      if (l)
        {
          l->setVisible (showIp || showMac);
        }
      interfaceText->setVisible (showIp || showMac);
    }
  for (AnimInterfaceTextVector_t::const_iterator i = m_interfaceBTexts.begin ();
      i != m_interfaceBTexts.end ();
      ++i)
    {
      AnimInterfaceText * interfaceText = *i;
      interfaceText->setMode (m_showIpInterfaceTexts, m_showMacInterfaceTexts);
      QGraphicsLineItem * l = interfaceText->getLine ();
      l->setPen (pen);
      if (l)
        {
          l->setVisible (showIp || showMac);
        }
      interfaceText->setVisible (showIp || showMac);
    }
  removeInterfaceTextCollision ();
  update ();
}


QList <QGraphicsItem *>
AnimatorScene::getInterfaceTextCollisionList (AnimInterfaceText * text)
{
  QList <QGraphicsItem *> l = text->collidingItems ();
  QList <QGraphicsItem *> collidingList;
  for (QList <QGraphicsItem *>::const_iterator i = l.begin ();
       i != l.end ();
       ++i)
    {
      QGraphicsItem * item = *i;
      if (item->type () == (ANIMINTERFACE_TEXT_TYPE))
        {
          collidingList.append (item);
        }
    }
  return collidingList;
}


void
AnimatorScene::repositionInterfaceText (AnimInterfaceText *textItem)
{
  bool isRight = textItem->pos ().x () > (sceneRect ().width ()/2);
  QPointF oldPos = textItem->pos ();
  QFontMetrics fm (font ());
  QPointF newPos;
  if (!isRight)
    {
      textItem->setLeftAligned (false);
      qreal y = m_leftTop + 1.5 * fm.height ()/AnimatorView::getInstance ()->transform ().m11 ();
      newPos = QPointF (-fm.width (textItem->getText ())/AnimatorView::getInstance ()->transform ().m11 (), y);
      m_leftTop = newPos.y ();
    }
  else
    {
      textItem->setLeftAligned (true);
      qreal y = m_rightTop + 1.5 * fm.height ()/AnimatorView::getInstance ()->transform ().m11 ();
      newPos = QPointF (m_maxPoint.x () + fm.width (textItem->getText ())/AnimatorView::getInstance ()->transform ().m11 (), y);
      m_rightTop = newPos.y ();
    }
  textItem->setPos (newPos);
  QLineF l (oldPos, newPos);
  if (textItem->setLine (l))
    {
      addItem (textItem->getLine ());
    }

}

void
AnimatorScene::resetInterfaceTexts ()
{
  resetInterfaceTextTop ();
  for (AnimInterfaceTextVector_t::const_iterator i = m_interfaceATexts.begin ();
      i != m_interfaceATexts.end ();
      ++i)
    {
      AnimInterfaceText * text = *i;
      QGraphicsLineItem * l = text->getLine ();
      if (l)
        {
          removeItem (l);
        }
      removeItem (*i);
      delete (*i);
    }
  m_interfaceATexts.clear ();
  for (AnimInterfaceTextVector_t::const_iterator i = m_interfaceBTexts.begin ();
      i != m_interfaceBTexts.end ();
      ++i)
    {
      AnimInterfaceText * text = *i;
      QGraphicsLineItem * l = text->getLine ();
      if (l)
        {
          removeItem (l);
        }
      removeItem (*i);
      delete (*i);
    }
  m_interfaceBTexts.clear ();
  update ();
}

void
AnimatorScene::resetInterfaceTextTop ()
{
  m_leftTop = 0;
  m_rightTop = 0;
}

void
AnimatorScene::removeInterfaceTextCollision ()
{

  for (AnimInterfaceTextVector_t::iterator i = m_interfaceATexts.begin ();
      i != m_interfaceATexts.end ();
      ++i)
    {
      AnimInterfaceText * text = *i;
      QList <QGraphicsItem *> collidingList = getInterfaceTextCollisionList (text);
      //qDebug (collidingList.count (), "CL count");
      //NS_LOG_DEBUG ("Colliding List:" << collidingList.count());

      //NS_LOG_DEBUG ("Colliding List S:" << collidingList.size());
      if (collidingList.count ())
        {
          repositionInterfaceText (text);
        }

    }
  for (AnimInterfaceTextVector_t::iterator i = m_interfaceBTexts.begin ();
      i != m_interfaceBTexts.end ();
      ++i)
    {
      AnimInterfaceText * text = *i;
      QList <QGraphicsItem *> collidingList = getInterfaceTextCollisionList (text);
      //qDebug (collidingList.count (), "CL count");
      if (collidingList.count ())
        {
          repositionInterfaceText (text);
        }
    }

}


AnimInterfaceText::AnimInterfaceText (QString description, bool leftAligned):QGraphicsTextItem (description),
  m_leftAligned (leftAligned),
  m_line (0)
{
  setFlag (QGraphicsItem::ItemIgnoresTransformations);
  setZValue (ANIMINTERFACE_TEXT_TYPE);
}

AnimInterfaceText::~AnimInterfaceText ()
{
  if (m_line)
    {
      delete m_line;
    }
}

void
AnimInterfaceText::setLeftAligned (bool leftAligned)
{
  m_leftAligned = leftAligned;
}

QPainterPath
AnimInterfaceText::shape () const
{
  QPainterPath p;
  QFontMetrics fm (font ());
  QRectF r (0, 0, fm.width (getText ())/AnimatorView::getInstance ()->transform ().m11 (),
           fm.height ()/AnimatorView::getInstance ()->transform ().m11 ());
  p.addRect (r);
  return p;
}

QString
AnimInterfaceText::getText () const
{
  QStringList parts = toPlainText ().split ('~');
  if (m_mode == AnimInterfaceText::IPV4)
    {
      return parts.at (0);
    }
  if (m_mode == AnimInterfaceText::MAC)
    {
      if (parts.length () != 2)
        {
          return "";
        }
      return parts.at (1);
    }
  return toPlainText ();
}


void
AnimInterfaceText::paint (QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{

  Q_UNUSED (option)
  Q_UNUSED (widget)
  if (m_leftAligned)
    {
      QFontMetrics fm = painter->fontMetrics ();
      QPointF leftAlignPoint = QPointF (-fm.width (getText ()), 0);
      painter->save ();
      painter->translate (leftAlignPoint);
      painter->drawText (QPointF (0, 0), getText ());
      //QGraphicsTextItem::paint (painter, option, widget);
      painter->restore ();
    }
  else
    {
      //QGraphicsTextItem::paint (painter, option, widget);
      painter->drawText (QPointF (0, 0), getText ());

    }
}

bool
AnimInterfaceText::setLine (QLineF l)
{
  bool newLine = false;
  if (!m_line)
    {
      m_line = new QGraphicsLineItem;
      newLine = true;
    }
  QPen p;
  p.setCosmetic (true);
  p.setColor (QColor (0, 0, 255, 50));
  m_line->setPen (p);
  m_line->setLine (l);
  return newLine;
}

QGraphicsLineItem *
AnimInterfaceText::getLine ()
{
  return m_line;
}

void
AnimInterfaceText::setMode (bool showIpv4, bool showMac)
{
  if (!showIpv4 && !showMac)
    {
      m_mode = AnimInterfaceText::NONE;
    }
  if (showIpv4 && !showMac)
    {
      m_mode = AnimInterfaceText::IPV4;
    }
  if (!showIpv4 && showMac)
    {
      m_mode = AnimInterfaceText::MAC;
    }
  if (showIpv4 && showMac)
    {
      m_mode = AnimInterfaceText::BOTH;
    }
}



} // namespace netanim
