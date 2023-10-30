/* Modification */
/*
 * unsched-tag.cc
 *
 *  Created on: Mar 26, 2020
 *      Author: vamsi
 */


#include "unsched-tag.h"
namespace ns3{

TypeId
UnSchedTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::UnSchedTag")
    .SetParent<Tag> ()
    .AddConstructor<UnSchedTag> ()
    .AddAttribute ("unsched",
                   "indicates an unscheduled packet if 1, no if 0",
                   UintegerValue (0),
                   MakeUintegerAccessor (&UnSchedTag::GetValue),
                   MakeUintegerChecker<uint8_t> ())
  ;
  return tid;
}
TypeId
UnSchedTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}
uint32_t
UnSchedTag::GetSerializedSize (void) const
{
  return 1;
}
void
UnSchedTag::Serialize (TagBuffer i) const
{
  i.WriteU8 (m_value);
}
void
UnSchedTag::Deserialize (TagBuffer i)
{
	m_value = i.ReadU8 ();
}
void
UnSchedTag::Print (std::ostream &os) const
{
  os << "v=" << (uint32_t)m_value;
}
void
UnSchedTag::SetValue (uint8_t value)
{
	m_value = value;
}
uint8_t
UnSchedTag::GetValue (void) const
{
  return m_value;
}
/* Modification */

}