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

#include "textbubble.h"

using namespace std;
namespace netanim
{

QRectF br;

TextBubble::TextBubble (QString title, QString content)
{
  content += '\0';
  QString str = title + "\n";
  QStringList list =  content.split ('^');
  foreach (QString s, list)
  {
    str += s + "\n";
  }
  setText (str);
  setFrameStyle (QFrame::Panel | QFrame::Sunken);
  adjustSize ();
  setTextInteractionFlags (Qt::TextSelectableByMouse);

}
TextBubble::~TextBubble ()
{

}


} //namespace netanim


