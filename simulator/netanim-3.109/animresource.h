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

#ifndef ANIMRESOURCE_H
#define ANIMRESOURCE_H

#include "common.h"


class AnimResourceManager
{
public:
  static AnimResourceManager * getInstance ();
  void add (uint32_t resourceId, QString resourcePath);
  QString get (uint32_t resourceid);
  uint32_t getNewResourceId ();
private:
  AnimResourceManager ();
  std::map <uint32_t, QString> m_resources;

  uint32_t m_maxResourceId;

};
#endif // ANIMRESOURCE_H
