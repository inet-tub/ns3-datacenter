/* Modification */
/*
 * custom-priority-tag.cc
 *
 *  Created on: Mar 26, 2020
 *      Author: vamsi
 */

#include "custom-priority-tag.h"
namespace ns3{

TypeId
MyPriorityTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MyPriorityTag")
    .SetParent<Tag> ()
    .AddConstructor<MyPriorityTag> ()
    .AddAttribute ("priorityCustom",
                   "Priority value",
                   EmptyAttributeValue (),
                   MakeUintegerAccessor (&MyPriorityTag::GetPriority),
                   MakeUintegerChecker<uint8_t> ())
  ;
  return tid;
}
TypeId
MyPriorityTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}
uint32_t
MyPriorityTag::GetSerializedSize (void) const
{
  return 1;
}
void
MyPriorityTag::Serialize (TagBuffer i) const
{
  i.WriteU8 (m_priorityCustom);
}
void
MyPriorityTag::Deserialize (TagBuffer i)
{
  m_priorityCustom = i.ReadU8 ();
}
void
MyPriorityTag::Print (std::ostream &os) const
{
  os << "v=" << (uint32_t)m_priorityCustom;
}
void
MyPriorityTag::SetPriority (uint8_t value)
{
  m_priorityCustom = value;
}
uint8_t
MyPriorityTag::GetPriority (void) const
{
  return m_priorityCustom;
}
/* Modification */

}