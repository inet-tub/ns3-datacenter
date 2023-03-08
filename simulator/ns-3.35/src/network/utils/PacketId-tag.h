/* Modification */
/*
 * PacketId-tag.h
 *
 *  Created on: Mar 26, 2020
 *      Author: vamsi
 */

#ifndef RDMA_TAG_H_
#define RDMA_TAG_H_



#include "ns3/tag.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include <iostream>

using namespace ns3;

class PacketId : public Tag
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (TagBuffer i) const;
  virtual void Deserialize (TagBuffer i);
  virtual void Print (std::ostream &os) const;

  void SetValue (uint64_t value);
  /**
   * Get the tag value
   * \return the tag value.
   */
  uint64_t GetValue (void) const;
private:
  uint64_t m_value;  //!< tag value
};




#endif /* RDMA_TAG_H_ */
/* Modification */