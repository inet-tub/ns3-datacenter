/* Modification */
/*
 * custom-priority-tag.h
 *
 *  Created on: Mar 26, 2020
 *      Author: vamsi
 */

#ifndef EXAMPLES_PLASTICINE_CUSTOM_PRIORITY_TAG_H_
#define EXAMPLES_PLASTICINE_CUSTOM_PRIORITY_TAG_H_



#include "ns3/tag.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include <iostream>

namespace ns3{

class MyPriorityTag : public Tag
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

  // these are our accessors to our tag structure
  /**
   * Set the tag value
   * \param value The tag value.
   */
  void SetPriority (uint8_t value);
  /**
   * Get the tag value
   * \return the tag value.
   */
  uint8_t GetPriority (void) const;
private:
  uint8_t m_priorityCustom;  //!< tag value
};

}

#endif /* EXAMPLES_PLASTICINE_CUSTOM_PRIORITY_TAG_H_ */
/* Modification */
