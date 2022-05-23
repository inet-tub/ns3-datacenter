/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
 *
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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#include "interface-tag.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("InterfaceTag");

NS_OBJECT_ENSURE_REGISTERED (InterfaceTag);

TypeId 
InterfaceTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::InterfaceTag")
    .SetParent<Tag> ()
    .SetGroupName("Network")
    .AddConstructor<InterfaceTag> ()
  ;
  return tid;
}
TypeId 
InterfaceTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}
uint32_t 
InterfaceTag::GetSerializedSize (void) const
{
  NS_LOG_FUNCTION (this);
  return 4;
}
void 
InterfaceTag::Serialize (TagBuffer buf) const
{
  NS_LOG_FUNCTION (this << &buf);
  buf.WriteU32 (m_portId);
}
void 
InterfaceTag::Deserialize (TagBuffer buf)
{
  NS_LOG_FUNCTION (this << &buf);
  m_portId = buf.ReadU32 ();
}
void 
InterfaceTag::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION (this << &os);
  os << "Interface=" << m_portId;
}
InterfaceTag::InterfaceTag ()
  : Tag () 
{
  NS_LOG_FUNCTION (this);
}

InterfaceTag::InterfaceTag (uint32_t id)
  : Tag (),
    m_portId (id)
{
  NS_LOG_FUNCTION (this << id);
}

void
InterfaceTag::SetPortId (uint32_t id)
{
  NS_LOG_FUNCTION (this << id);
  m_portId = id;
}
uint32_t
InterfaceTag::GetPortId (void) const
{
  NS_LOG_FUNCTION (this);
  return m_portId;
}

uint32_t 
InterfaceTag::AllocatePortId (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  static uint32_t nextPortId = 1;
  uint32_t portId = nextPortId;
  nextPortId++;
  return portId;
}

} // namespace ns3

