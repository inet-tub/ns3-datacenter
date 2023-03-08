/* Modification */
/*
 * PacketId-tag.cc
 *
 *  Created on: Mar 26, 2020
 *      Author: vamsi
 */


#include "PacketId-tag.h"
using namespace ns3;

TypeId
PacketId::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PacketId")
    .SetParent<Tag> ()
    .AddConstructor<PacketId> ()
    .AddAttribute ("PacketId",
                   "a PacketId used for logging purposes",
                   UintegerValue (0),
                   MakeUintegerAccessor (&PacketId::GetValue),
                   MakeUintegerChecker<uint64_t> ())
  ;
  return tid;
}
TypeId
PacketId::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}
uint32_t
PacketId::GetSerializedSize (void) const
{
  return 4;
}
void
PacketId::Serialize (TagBuffer i) const
{
  i.WriteU64 (m_value);
}
void
PacketId::Deserialize (TagBuffer i)
{
	m_value = i.ReadU64 ();
}
void
PacketId::Print (std::ostream &os) const
{
  os << "v=" << (uint64_t)m_value;
}
void
PacketId::SetValue (uint64_t value)
{
	m_value = value;
}
uint64_t
PacketId::GetValue (void) const
{
  return m_value;
}
/* Modification */