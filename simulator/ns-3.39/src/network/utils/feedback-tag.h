/* Modification */
/*
 * feedback-tag.h
 *
 *  Created on: Jan 4, 2021
 *      Author: vamsi
 */

#ifndef SRC_NETWORK_MODEL_FEEDBACK_TAG_H_
#define SRC_NETWORK_MODEL_FEEDBACK_TAG_H_


#include "ns3/tag.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include <iostream>
#include "ns3/simulator.h"

namespace ns3{

class FeedbackTag : public Tag
{
public:

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (TagBuffer i) const;
  virtual void Deserialize (TagBuffer i);
  virtual void Print (std::ostream &os) const;

  struct telemetry {
	  uint64_t bandwidth;
	  uint64_t tsEnq;
	  uint64_t tsDeq; //timestamp at the time of dequeue
	  uint32_t qlenEnq;
	  uint32_t qlenDeq; // queue length 
	  uint64_t txBytes; // counter +=ptksize
  };

  void incrementHopCount(){n_hops++;}
  uint32_t getHopCount(){return n_hops;}
  uint32_t getMaxHops(){return max_hops;}
  void setMaxHops(uint32_t n){max_hops=n;}
  void pushTelemetry(FeedbackTag::telemetry t){Feedback[n_hops] = t ;} /* Here the assumption is that, before a node pushes feedback info, it first increments n_hops value. Be careful with this. */
  void setTelemetryBw(uint32_t hop, uint64_t bw){Feedback[hop].bandwidth = bw;}
  void setTelemetryTsEnq(uint32_t hop, uint64_t ts){Feedback[hop].tsEnq = ts;}
  void setTelemetryTsDeq(uint32_t hop, uint64_t ts){Feedback[hop].tsDeq = ts;}
  void setTelemetryQlenEnq(uint32_t hop, uint32_t val){Feedback[hop].qlenEnq = val;}
  void setTelemetryQlenDeq(uint32_t hop, uint32_t val){Feedback[hop].qlenDeq = val;}
  void setTelemetryTxBytes(uint32_t hop, uint64_t val){Feedback[hop].txBytes = val;}

  FeedbackTag::telemetry getFeedback(uint32_t i){return Feedback[i];}

  void setPktTimestamp(uint64_t ts){sent_timestamp = ts;}
  uint64_t getPktTimestamp(){return sent_timestamp;}

private:
  uint8_t max_hops=16; // This is hardcoded for now. Sorry!
  uint8_t n_hops=0;
  uint64_t sent_timestamp=0;
  FeedbackTag::telemetry Feedback[16]; // size of 16 is hardcoded for now. Sorry!
//  std::vector<FeedbackTag::telemetry> Feedback;
};

}
#endif /* SRC_NETWORK_MODEL_FEEDBACK_TAG_H_ */
/* Modification */