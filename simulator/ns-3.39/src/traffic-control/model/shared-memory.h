/*
 * shared-memory.h
 *
 *  Created on: May 13, 2020
 *      Author: vamsi
 */

#ifndef SRC_TRAFFIC_CONTROL_MODEL_SHARED_MEMORY_H_BKP_
#define SRC_TRAFFIC_CONTROL_MODEL_SHARED_MEMORY_H_BKP_

#include "ns3/simulator.h"
#include "ns3/object.h"
#include "ns3/queue-disc.h"
#include "fifo-queue-disc.h"
#include "ns3/queue.h"
#include "unordered_map"
#include "ns3/log.h"
#include "ns3/abort.h"
#include "ns3/uinteger.h"
#include "ns3/pointer.h"
#include "ns3/object-vector.h"
#include "ns3/packet.h"
#include "ns3/socket.h"
#include "ns3/unused.h"

namespace ns3 {

class SharedMemoryBuffer : public Object{
public:
	static TypeId GetTypeId (void);
	SharedMemoryBuffer();
	virtual ~SharedMemoryBuffer();

	void SetSharedBufferSize(uint32_t size);

	double GetNofP(uint32_t priority);
	bool EnqueueBuffer(uint32_t size, uint32_t port, uint32_t queue);
	void DequeueBuffer(uint32_t size, uint32_t port, uint32_t queue);

	void AddN(uint32_t group){N[group]++;}
	void SubN(uint32_t group){N[group]--;}

	void setSaturated(uint32_t port,uint32_t priority, double satLevel);
	
	uint32_t isSaturated(uint32_t port,uint32_t priority){return saturated[port][priority];}

	void setPriorityToGroup(uint32_t priority,uint32_t group){PriorityToGroupMap[priority]=group;}

	uint32_t getPriorityToGroup(uint32_t priority){return PriorityToGroupMap[priority];}

	void updateN(void);

	double getDeq(uint32_t prio,uint32_t port);
	void addDeq(uint32_t bytes,uint32_t prio, uint32_t port);
	Time getTimestamp(uint32_t port, uint32_t queue){return timestamp[port][queue];}
	void setTimestamp(Time x,uint32_t port,uint32_t queue){timestamp[port][queue]=x;}

	void PerPriorityStatEnq(uint32_t size, uint32_t priority);
	void PerPriorityStatDeq(uint32_t size, uint32_t priority);

	void addQueuePtr(Ptr<QueueDisc> queue, uint32_t port);

	void setPorts(uint32_t ports){
		numPorts = ports;
	}
	uint32_t getPorts(){
		return numPorts;
	}
	void setQueues(uint32_t queues){
		numQueues = queues;
	}

	uint32_t findLongestQueue();

	uint32_t* RemoveLongestQueuePacket();

	void setSwitchId(uint32_t id){
		switchId = id;
	}
	void setAverageInteral(Time t){
		AverageInterval = t;
	}
	uint32_t findLongestThreshold();
	void UpdateThreshold(uint32_t size, uint32_t port, uint32_t queue);

	///////////// USeful for logs ///////////////
	uint32_t GetSharedBufferSize();
	uint32_t GetOccupiedBuffer(){return OccupiedBuffer;}
	uint32_t GetRemainingBuffer(){return RemainingBuffer;}
	uint32_t GetPerPriorityOccupied(uint32_t priority){return OccupiedBufferPriority[priority];}
	uint32_t GetQueueSize(uint32_t port, uint32_t queue);
	uint32_t getAverageQueueLength(uint32_t port, uint32_t queue){
		return averageQueueLength[port][queue];
	}
	uint32_t getAverageOccupancy(){
		return averageSharedOccupancy;
	}
	uint32_t GetThreshold(uint32_t port, uint32_t queue){
		return threshold[port][queue];
	}
	/////////////////////////////////////////////

protected:
	void DoInitialize (void);

	virtual void DoDispose (void);


private:
	uint32_t TotalBuffer;
	uint32_t OccupiedBuffer;
	uint32_t OccupiedBufferPriority[8]={0,0,0,0,0,0,0,0};
	uint32_t RemainingBuffer;
	double N[8]; // N corresponds to each queue (one-one mapping with priority) at each port. 8 queues exist at each port.
	double saturated[100][8]; // 100 ports and 8 queues per node which share the buffer, are supported for now.

	std::unordered_map<uint32_t,uint32_t> PriorityToGroupMap;

	std::vector<std::pair<uint32_t,Time>> Deq[100][8];
	double sumBytes[100][8];
	Time tDiff[100][8];
	uint64_t MaxRate;
	Time timestamp[100][8];

	///////////////
	uint32_t queueLength[100][8];
	uint32_t averageQueueLength[100][8];
	uint32_t threshold[100][8];
	uint32_t totalThreshold;
	uint32_t averageSharedOccupancy;
	Ptr<QueueDisc> QueuePtr[100];
	uint32_t numPorts;
	uint32_t numQueues;
	uint32_t switchId;
	Time LastUpdatedAverage[100][8];
	Time LastUpdatedAverageTotal;
	Time AverageInterval;

	bool thresholdBusy;
};
}



#endif /* SRC_TRAFFIC_CONTROL_MODEL_SHARED_MEMORY_H_BKP_ */
