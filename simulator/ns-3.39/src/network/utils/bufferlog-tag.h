#ifndef SRC_NETWORK_MODEL_BUFFERLOG_TAG_H_
#define SRC_NETWORK_MODEL_BUFFERLOG_TAG_H_


#include "ns3/tag.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include <iostream>
#include "ns3/simulator.h"

namespace ns3{

class BufferLogTag : public Tag
{
public:

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (TagBuffer i) const;
  virtual void Deserialize (TagBuffer i);
  virtual void Print (std::ostream &os) const;

  void setQueueLength(uint32_t q);
  void setAverageQueueLength(uint32_t q);
  void setOccupiedBuffer(uint32_t q);
  void setAverageOccupiedBuffer(uint32_t q);

  uint32_t getQueueLength();
  uint32_t getAverageQueueLength();
  uint32_t getOccupiedBuffer();
  uint32_t getAverageOccupiedBuffer();

private:
  uint32_t queueLength;
  uint32_t averageQueueLength;
  uint32_t OccupiedBuffer;
  uint32_t averageOccupiedBuffer;
};

}
#endif /* SRC_NETWORK_MODEL_BUFFERLOG_TAG_H_ */