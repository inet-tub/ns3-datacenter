/*
 * shared-memory.cc
 *
 *  Created on: May 13, 2020
 *      Author: vamsi
 */
#include "shared-memory.h"

namespace ns3 {
NS_LOG_COMPONENT_DEFINE ("SharedMemoryBuffer");
NS_OBJECT_ENSURE_REGISTERED (SharedMemoryBuffer);


TypeId SharedMemoryBuffer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SharedMemoryBuffer")
    .SetParent<Object> ()
   	.AddConstructor<SharedMemoryBuffer> ()
		.AddAttribute ("BufferSize",
	                   "TotalBufferSize",
	                   UintegerValue (1000*1000),
	                   MakeUintegerAccessor (&SharedMemoryBuffer::TotalBuffer),
	                   MakeUintegerChecker <uint32_t> ())
		;
  return tid;
}

SharedMemoryBuffer::SharedMemoryBuffer(){
	for (int i=0;i<8;i++){
		N[i]=0;
	}
	for (int i=0;i<99;i++){
		for (int j=0;j<8;j++){
			saturated[i][j]=0;
			sumBytes[i][j]=1500;
			tDiff[i][j]=Seconds(0);
			queueLength[i][j]=0;
			averageQueueLength[i][j]=0;
			threshold[i][j]=0;
		}
	}

	OccupiedBuffer=0;
	numPorts = 0;
	numQueues = 0;
	averageSharedOccupancy=0;
	thresholdBusy=false;
	totalThreshold=0;
}

SharedMemoryBuffer::~SharedMemoryBuffer ()
{
  NS_LOG_FUNCTION (this);
}


void
SharedMemoryBuffer::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  for (int i=0;i<8;i++){
  		N[i]=0;
  	}
  	for (int i=0;i<99;i++){
  		for (int j=0;j<8;j++){
  			saturated[i][j]=0;
  			timestamp[i][j]=Seconds(0);
			queueLength[i][j]=0;
			averageQueueLength[i][j]=0;
			threshold[i][j]=0;
  		}
  	}
  	TotalBuffer=0;
  	OccupiedBuffer=0;
  	RemainingBuffer=0;
	numPorts = 0;
	numQueues = 0;
	averageSharedOccupancy=0;

  Object::DoDispose ();
}

void
SharedMemoryBuffer::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
  for (int i=0;i<8;i++){
  		N[i]=1;
  	}
  	for (int i=0;i<99;i++){
  		for (int j=0;j<8;j++){
  			saturated[i][j]=0;
			queueLength[i][j]=0;
			averageQueueLength[i][j]=0;
			threshold[i][j]=0;
		}
  	}

  	OccupiedBuffer=0;
	averageSharedOccupancy=0;
	numPorts = 0;
	numQueues = 0;
  Object::DoInitialize ();
}


void SharedMemoryBuffer::SetSharedBufferSize(uint32_t size){
	TotalBuffer = size;
	RemainingBuffer = size-OccupiedBuffer;
}
uint32_t SharedMemoryBuffer::GetSharedBufferSize(){
	return TotalBuffer;
}

void SharedMemoryBuffer::PerPriorityStatEnq(uint32_t size, uint32_t priority){
	OccupiedBufferPriority[priority] += size;
}

void SharedMemoryBuffer::PerPriorityStatDeq(uint32_t size, uint32_t priority){
	OccupiedBufferPriority[priority] -= size;
}

uint32_t SharedMemoryBuffer::GetQueueSize(uint32_t port, uint32_t queue){
	return queueLength[port][queue];
}


uint32_t SharedMemoryBuffer::findLongestThreshold(){
	uint32_t longestQueueLength = 0;
	uint32_t longestQueue = 0;
	for (uint32_t i = 0; i < numPorts; i++){
		if (threshold[i][1] > longestQueueLength ){ // For now, this is a single queue case. We assume only queue id 1 at each port receives data packets.
			longestQueueLength = threshold[i][1];
			longestQueue = i;
		}
	}
	return longestQueue;
}

void SharedMemoryBuffer::UpdateThreshold(uint32_t size, uint32_t port, uint32_t queue){
	while(totalThreshold + size > TotalBuffer){
		uint32_t lq = findLongestThreshold();
		if (threshold[lq][1] >= 1500){ // 1500 MTU
			threshold[lq][1] -= 1500;
			if (totalThreshold>=1500){
				totalThreshold -= 1500;
			}
			else{
				totalThreshold = 0;
			}
		}
		else{
			if (totalThreshold>=threshold[lq][1]){
				totalThreshold -= threshold[lq][1];
			}
			else{
				totalThreshold = 0;
			}
			threshold[lq][1] = 0;
		}
	}
	threshold[port][queue] = std::min(threshold[port][queue]+size, TotalBuffer);
	totalThreshold = std::min(totalThreshold+size, TotalBuffer);
	// if (switchId==0 && port==17 && queue==1){
	// 	std::cout << "UpdateThreshold: " << "queuelength " << queueLength[port][queue] << " threshold " << threshold[port][queue] << " totalThreshold " << totalThreshold << " size " << size  << std::endl;
	// }
	return;
}

bool SharedMemoryBuffer::EnqueueBuffer(uint32_t size, uint32_t port, uint32_t queue){
	if(RemainingBuffer>size){
		RemainingBuffer-=size;
		OccupiedBuffer= std::min(OccupiedBuffer+size, TotalBuffer);
		queueLength[port][queue]+=size;
		// if (switchId==0 && port==17 && queue==1){
		// 	std::cout << "Enqueue: " << "queuelength " << queueLength[port][queue] << " threshold " << threshold[port][queue] << " totalThreshold " << totalThreshold << " size " << size  << std::endl;
		// }
		return true;
	}
	else{
		// if (switchId==0 && port==17 && queue==1){
		// 	std::cout << "Drop: " << "queuelength " << queueLength[port][queue] << " threshold " << threshold[port][queue] << " totalThreshold " << totalThreshold << " size " << size  << std::endl;
		// }
		return false;
	}
}

void SharedMemoryBuffer::DequeueBuffer(uint32_t size, uint32_t port, uint32_t queue){
	double gammaqueue = (std::min((Simulator::Now()-LastUpdatedAverage[port][queue]).GetNanoSeconds(), AverageInterval.GetNanoSeconds())/double(AverageInterval.GetNanoSeconds()));
	LastUpdatedAverage[port][queue] = Simulator::Now();
	double gamma = (std::min((Simulator::Now()-LastUpdatedAverageTotal).GetNanoSeconds(), AverageInterval.GetNanoSeconds())/double(AverageInterval.GetNanoSeconds()));
	LastUpdatedAverageTotal = Simulator::Now();
	if(OccupiedBuffer>size){
		OccupiedBuffer-=size;
		averageSharedOccupancy = gamma*(double(OccupiedBuffer)) + (1-gamma)*double(averageSharedOccupancy);
		RemainingBuffer+=size;
	}
	else{
		OccupiedBuffer = 0;
		averageSharedOccupancy = gamma*(double(OccupiedBuffer)) + (1-gamma)*double(averageSharedOccupancy);
		RemainingBuffer = TotalBuffer;
	}
	if (queueLength[port][queue]>size){
		queueLength[port][queue]-=size;
		averageQueueLength[port][queue] = gammaqueue*(double(queueLength[port][queue])) + (1-gammaqueue)*double(averageQueueLength[port][queue]);
	}
	else{
		queueLength[port][queue]=0;
		averageQueueLength[port][queue] = gammaqueue*(double(queueLength[port][queue])) + (1-gammaqueue)*double(averageQueueLength[port][queue]);
	}
	// Dequeue thresholds
	if (queueLength[port][queue]==0){
		if (totalThreshold >= threshold[port][queue]){
			totalThreshold -= threshold[port][queue];
			threshold[port][queue] = 0;// reset thresholds when queue drains to zero.
		}
		else{
			totalThreshold = 0;
			threshold[port][queue] = 0;
		}
	}
	else{
		if (threshold[port][queue] >=size){
			threshold[port][queue] -= size;
			if (totalThreshold>= size){
				totalThreshold -= size;
			}
			else{
				totalThreshold = 0;
			}
		}
		else{
			if (totalThreshold>= threshold[port][queue]){
				totalThreshold -= threshold[port][queue];
				threshold[port][queue] = 0;
			}
			else{
				totalThreshold = 0;
				threshold[port][queue] = 0;
			}	
		}
	}
	// if (switchId==0 && port==17 && queue==1){
	// 	std::cout << "Dequeue: " <<  "queuelength " << queueLength[port][queue] << " threshold " << threshold[port][queue] << " totalThreshold " << totalThreshold << " size " << size << std::endl;
	// }
}

double SharedMemoryBuffer::GetNofP(uint32_t priority){
	if(N[priority]>=1)
		return N[priority];
	else
		return 1;
}

void SharedMemoryBuffer::addQueuePtr(Ptr<QueueDisc> queuePtr, uint32_t port){
	QueuePtr[port]=queuePtr;
}

uint32_t SharedMemoryBuffer::findLongestQueue(){
	uint32_t longestQueueLength = 0;
	uint32_t longestQueue = 0;
	for (uint32_t i = 0; i < numPorts; i++){
		if (queueLength[i][1] > longestQueueLength ){ // For now, this is a single queue case. We assume only queue id 1 at each port receives data packets.
			longestQueueLength = queueLength[i][1];
			longestQueue = i;
		}
	}
	return longestQueue;
}

uint32_t*
SharedMemoryBuffer::RemoveLongestQueuePacket(){
	uint32_t longestQueue = findLongestQueue();
	Ptr<const QueueDiscItem> item = QueuePtr[longestQueue]->GetQueueDiscClass (1)->GetQueueDisc ()->GetInternalQueue(0)->GetTailPacket();
  static uint32_t arr[4]={0,0,0,0};
  if (item){
		DequeueBuffer(item->GetPacket()->GetSize(), longestQueue, 1);
		PerPriorityStatDeq(item->GetPacket()->GetSize(), 1);
	  BufferLogTag buffertag;
	  bool found = item->GetPacket()->PeekPacketTag(buffertag);
	  if(found){
	    arr[0] = buffertag.getQueueLength();
	    arr[1] = buffertag.getAverageQueueLength();
	    arr[2] = buffertag.getOccupiedBuffer();
	    arr[3] = buffertag.getAverageOccupiedBuffer();
	  }
		Ptr<QueueDiscItem> removedItem = QueuePtr[longestQueue]->GetQueueDiscClass (1)->GetQueueDisc ()->GetInternalQueue(0)->PushOut();
		// std::cout << "removed bytes" << removedItem->GetSize() << std::endl; 
		QueuePtr[longestQueue]->GetQueueDiscClass(1)->GetQueueDisc()->DropAfterEnqueue(removedItem);
		// if (removedItem){
		// 	std::cout << "still not deleted from queue" << std::endl;
		// }
	}
	// else{
	// 	std::cout << "remove failed " << longestQueue << " " <<  QueuePtr[longestQueue]->GetQueueDiscClass (1)->GetQueueDisc ()->GetNBytes() << std::endl;
	// }
	return arr;
}


// # define 	DT 		101
// # define 	FAB 	102
// # define 	AFD 	103
// # define 	ABM 	110

// uint64_t SharedMemoryBuffer::getThreshold(uint32_t priority, uint32_t unsched){
// 	uint64_t threshold = TotalBuffer;
// 	switch(algorithm){
// 		case DT:
// 			threshold = alpha[priority]*RemainingBuffer;
// 			break;
// 		case FAB:
			

// 	}
// 	if (unsched){
// 		return (alphaUnsched*(RemainingBuffer)/(GetNofP(priority)));
// 	}
// }


void SharedMemoryBuffer::setSaturated(uint32_t port,uint32_t priority, double satLevel){
	N[priority] += satLevel - saturated[port][priority];
	saturated[port][priority]=satLevel;
}

void SharedMemoryBuffer::addDeq(uint32_t bytes,uint32_t prio, uint32_t port){
	if(Deq[port][prio].size() > 100){
		sumBytes[port][prio]-= Deq[port][prio][0].first;
		Deq[port][prio].erase(Deq[port][prio].begin());
	}

	std::pair<uint32_t,Time> temp;
	temp.first=bytes;
	temp.second=Simulator::Now();
	Deq[port][prio].push_back(temp);
	sumBytes[port][prio]+=bytes;
}

double SharedMemoryBuffer::getDeq(uint32_t prio,uint32_t port){
	Time t = Seconds(0);
//	std::cout << "Size " << Deq[port][prio].size() << " SumBytes " << sumBytes[port][prio] << std::endl;
	if(Deq[port][prio].size()>1){
//		std::cout << "tEnd " << (Deq[port][prio].end()-1)->second.GetSeconds() << " tBegin " << Deq[port][prio].begin()->second.GetSeconds() << std::endl;
		t = (Deq[port][prio].end()-1)->second - Deq[port][prio].begin()->second;
	}
	else
		return 1;
	double deq = 8*sumBytes[port][prio]/t.GetSeconds()/MaxRate;
//	std::cout << "Size " << Deq[port][prio].size() << " SumBytes " << sumBytes[port][prio] << " Deq " << deq << " t " << t.GetSeconds()<< std::endl;
//	std::cout << "Deq " << deq << std::endl;
	if (deq>1 || deq<0) // sanity check
		return 1;
	else
		return deq;
}


}
