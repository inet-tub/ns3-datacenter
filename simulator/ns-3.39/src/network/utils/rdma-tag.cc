/* Modification */
/*
 * RdmaTag-tag.cc
 *
 *  Created on: Mar 26, 2020
 *      Author: vamsi
 */


#include "rdma-tag.h"
namespace ns3{

TypeId
RdmaTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RdmaTag")
    .SetParent<Tag> ()
    .AddConstructor<RdmaTag> ()
    .AddAttribute ("notRdma",
                   "indicates 1 if not RDMA, 0 if RDMA. Default is 0 i.e., RDMA",
                   UintegerValue (0),
                   MakeUintegerAccessor (&RdmaTag::GetValue),
                   MakeUintegerChecker<uint8_t> ())
  ;
  return tid;
}
TypeId
RdmaTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}
uint32_t
RdmaTag::GetSerializedSize (void) const
{
  return 1;
}
void
RdmaTag::Serialize (TagBuffer i) const
{
  i.WriteU8 (m_value);
}
void
RdmaTag::Deserialize (TagBuffer i)
{
	m_value = i.ReadU8 ();
}
void
RdmaTag::Print (std::ostream &os) const
{
  os << "v=" << (uint32_t)m_value;
}
void
RdmaTag::SetValue (uint8_t value)
{
	m_value = value;
}
uint8_t
RdmaTag::GetValue (void) const
{
  return m_value;
}
/* Modification */

}