/* Modification */
/*
 * rdma-tag.h
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

namespace ns3{

class RdmaTag : public Tag
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

  void SetValue (uint8_t value);
  /**
   * Get the tag value
   * \return the tag value.
   */
  uint8_t GetValue (void) const;
private:
  uint8_t m_value;  //!< tag value
};


}


#endif /* RDMA_TAG_H_ */
/* Modification */