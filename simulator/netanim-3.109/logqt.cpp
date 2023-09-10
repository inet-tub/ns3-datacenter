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

#include "logqt.h"
using namespace std;

namespace netanim
{

std::ostream & operator << (std::ostream & os, QPointF pt)
{
  os << "QPointF [x,y]:[" << pt.x () << "," << pt.y () << "]";
  return os;
}

std::ostream & operator << (std::ostream & os, QRectF r)
{
  os << "QRectF topLeft:" << r.topLeft () << " bottomRight:" << r.bottomRight ();
  return os;
}


std::ostream & operator << (std::ostream & os, QTransform t)
{
  os << "QTransform m11:" << t.m11 () << " m22:" << t.m22 ();
  return os;
}


std::ostream & operator << (std::ostream & os, AnimPacket * p)
{
  os << "AnimPacket:";
  os << " From Node Id:" << p->getFromNodeId ();
  os << " To Node Id:" << p->getToNodeId ();
  os << " First Bit Tx:" << p->getFirstBitTx () << endl;
  //os << "First Bit Rx:" << p->getFirstBitRx () << endl;
  //os << "Last Bit Tx:" << p->getLastBitTx () << endl;
  //os << "Last Bit Rx:" << p->getLastBitRx () << endl;
  return os;
}


}
