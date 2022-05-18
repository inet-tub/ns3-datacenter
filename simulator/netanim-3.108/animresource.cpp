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


#include "animresource.h"

AnimResourceManager * pResourceManager = 0;

AnimResourceManager::AnimResourceManager ():
    m_maxResourceId (0)
{

}

AnimResourceManager *
AnimResourceManager::getInstance ()
{
  if (!pResourceManager)
    {
      pResourceManager = new AnimResourceManager;
    }
  return pResourceManager;
}


void
AnimResourceManager::add (uint32_t resourceId, QString resourcePath)
{
  m_maxResourceId = qMax (resourceId, m_maxResourceId);
  m_resources[resourceId] = resourcePath;
}

uint32_t
AnimResourceManager::getNewResourceId ()
{
  return m_maxResourceId+1;
}


QString
AnimResourceManager::get (uint32_t resourceid)
{
  if (m_resources.find (resourceid) == m_resources.end ())
    {
      NS_FATAL_ERROR ("Unable to find resource:" << resourceid);
    }
  return m_resources[resourceid];
}
