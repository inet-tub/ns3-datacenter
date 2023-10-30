/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 Universita' degli Studi di Napoli Federico II
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors:  Vamsi Addanki (vamsi@inet.tu-berlin.de)
 */

#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/object-factory.h"
#include "ns3/socket.h"
#include "gen-queue-disc.h"
#include <algorithm>
#include <iterator>

#include "ns3/queue.h"
#include "ns3/net-device-queue-interface.h"
#include "ns3/tcp-header.h"
#include "ns3/ipv4-header.h"
#include "ns3/ppp-header.h"
#include "ns3/flow-id-tag.h"
#include "ns3/custom-priority-tag.h"
#include "ns3/unsched-tag.h"
#include "ns3/feedback-tag.h"
#include "ns3/bufferlog-tag.h"

# define DT 101
# define FAB 102
# define CS 103
# define IB 104
# define ABM 110
# define LQD 111
# define FOLLOWLQD 112
# define CREDENCE 666

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("GenQueueDisc");

NS_OBJECT_ENSURE_REGISTERED (GenQueueDisc);

TypeId GenQueueDisc::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::GenQueueDisc")
                      .SetParent<QueueDisc> ()
                      .SetGroupName ("TrafficControl")
                      .AddConstructor<GenQueueDisc> ()
                      .AddAttribute ("nPrior", "number of queues", UintegerValue (5),
                                     MakeUintegerAccessor (&GenQueueDisc::nPrior),
                                     MakeUintegerChecker<uint32_t> ())
                      .AddAttribute ("sat", "saturation detection",
                                     UintegerValue (20 * 1400),
                                     MakeUintegerAccessor (&GenQueueDisc::sat),
                                     MakeUintegerChecker<uint32_t> ())
                      .AddAttribute ("BufferAlgorithm", "BufferAlgorithm",
                                     UintegerValue (DT),
                                     MakeUintegerAccessor (&GenQueueDisc::bufferalg),
                                     MakeUintegerChecker<uint32_t> ())
                      .AddAttribute ("enableDPPQueue", "whether to use extra priority queue or not. This concerns IB algorithm. Turn this off in single queue setting.",
                                     BooleanValue (false),
                                     MakeBooleanAccessor (&GenQueueDisc::enableDPPQueue),
                                     MakeBooleanChecker())
                      .AddAttribute ("enableINT", "enable telemetry. Enabled by default",
                                     BooleanValue (true),
                                     MakeBooleanAccessor (&GenQueueDisc::enableINT),
                                     MakeBooleanChecker())
                      .AddAttribute ("alphaUnsched", "alphaUnsched",
                                     DoubleValue (1024),
                                     MakeDoubleAccessor (&GenQueueDisc::alphaUnsched),
                                     MakeDoubleChecker<double> ())
                      .AddAttribute ("portBW", "portBW in Gbps",
                                     DoubleValue (10),
                                     MakeDoubleAccessor (&GenQueueDisc::portBW),
                                     MakeDoubleChecker<double> ())
                      .AddAttribute ("addError", "probability of flipping the actual prediction",
                                     DoubleValue (0.0),
                                     MakeDoubleAccessor (&GenQueueDisc::addErr),
                                     MakeDoubleChecker<double> ())
                      .AddAttribute ("updateInterval", "NANOSECONDS update interval for dequeue rate and N in ActiveBufferManagement", UintegerValue(30000),
                                     MakeUintegerAccessor(&GenQueueDisc::updateInterval),
                                     MakeUintegerChecker<uint64_t>())
                      .AddAttribute ("staticBuffer", "static buffer",
                                     UintegerValue (0),
                                     MakeUintegerAccessor (&GenQueueDisc::staticBuffer),
                                     MakeUintegerChecker<uint64_t> ())
                      .AddAttribute ("RoundRobin", "round robin scheduling",
                                     UintegerValue (1),
                                     MakeUintegerAccessor (&GenQueueDisc::round_robin),
                                     MakeUintegerChecker<uint32_t> ())
                      .AddAttribute ("StrictPriority", "strict priority scheduling",
                                     UintegerValue (0),
                                     MakeUintegerAccessor (&GenQueueDisc::strict_priority),
                                     MakeUintegerChecker<uint32_t> ())
                      .AddAttribute ("predict", "whether to use predictions or not in Credence buffer sharing",
                                     BooleanValue (false),
                                     MakeBooleanAccessor (&GenQueueDisc::enablePredictions),
                                     MakeBooleanChecker())
                      .AddTraceSource ("genEnqueue", "trace enqueue events",
                                       MakeTraceSourceAccessor (&GenQueueDisc::m_rxTrace),
                                       "ns3::Packet::TracedCallback")
                      .AddTraceSource ("genDequeue", "trace dequeue events",
                                       MakeTraceSourceAccessor (&GenQueueDisc::m_txTrace),
                                       "ns3::Packet::TracedCallback")
                      .AddTraceSource ("traceLQD", "trace LQD events",
                                       MakeTraceSourceAccessor (&GenQueueDisc::m_traceLQD),
                                       "ns3::Packet::TracedCallback")
                      .AddTraceSource ("getPrediction", "callback to get predictions",
                                       MakeTraceSourceAccessor (&GenQueueDisc::m_getPrediction),
                                       "ns3::Packet::TracedCallback")
                      .AddTraceSource ("arrival", "callback arrival",
                                       MakeTraceSourceAccessor (&GenQueueDisc::m_arrival),
                                       "ns3::Packet::TracedCallback")
                      .AddTraceSource ("departure", "callback departure",
                                       MakeTraceSourceAccessor (&GenQueueDisc::m_departure),
                                       "ns3::Packet::TracedCallback")
                      ;
  return tid;
}

GenQueueDisc::GenQueueDisc ()
  : QueueDisc (QueueDiscSizePolicy::MULTIPLE_QUEUES, QueueSizeUnit::BYTES)
{
  NS_LOG_FUNCTION (this);
  alphas = nullptr;
  for (uint32_t i = 0; i < 11; i++) {
    firstSeen[i] = Seconds(0);
    lastAccepted[i] = ns3::Simulator::Now();
    numBytesSent[i] = 0;
    firstSeenQueue[i] = Seconds(0);
    lastAcceptedQueue[i] = ns3::Simulator::Now();
    numBytesSentQueue[i] = 0;
    droppedBytes[i] = 0;
    DeqRate[i] = 1;
    Deq[i] = 0;
    MFair[i] = 1000 * 1000 * 4;
    QRefAfd[i] = 1000 * 15;
    nofP[i] = 0;
    DPPQueue = 1;
  }
  urv = CreateObject<UniformRandomVariable> ();
  urv->SetAttribute ("Min", DoubleValue (0));
  urv->SetAttribute ("Max", DoubleValue (1));

  numPackets_big = 0;
  numPackets_small = 0;
}

GenQueueDisc::~GenQueueDisc ()
{
  NS_LOG_FUNCTION (this);
  if (alphas)
    delete alphas;
}

uint64_t
GenQueueDisc::GetBuffersize(uint32_t p) {
  uint64_t temp = bufferMax[p];
  bufferMax[p] = 0;
  return temp;
}

double
GenQueueDisc::GetThroughputEnQueue(uint32_t p, double nanodelay) {
  double th = 1e9 * 8 * numBytesSent[p] / nanodelay;
  numBytesSent[p] = 0;
  return th;
}


bool GenQueueDisc::DynamicThresholds(uint32_t priority, Ptr<Packet> packet) {

  double remaining = sharedMemory->GetRemainingBuffer();
  uint64_t maxSize = alphas[priority] * remaining;
  if (maxSize > UINT32_MAX)
    maxSize = UINT32_MAX - 1500;

  // uint32_t qSize = GetQueueDiscClass (priority)->GetQueueDisc ()->GetNBytes();
  uint32_t qSize = sharedMemory->GetQueueSize(portId, priority);
  if ( ((qSize + packet->GetSize()) >  maxSize) || (sharedMemory->GetRemainingBuffer() < packet->GetSize())  ) {
    return false; // drop
  }
  else {
    return true;
  }

}

void
GenQueueDisc::UpdateDequeueRate(double nanodelay) { // delay in NANOSECONDS. Pay attention here.
  double num = 0;
  /* This is because of round-robin scheduling. More to be added soon. In general, its better to measure dequeue rate like PIE */
  // for (uint32_t p=0;p<nPrior;p++){
  //   if (GetQueueDiscClass (p)->GetQueueDisc ()->GetNBytes()>sat){
  //     num++;
  //   }
  // }

  // if (num==0)
  //   num=1;

  // for (uint32_t p=0;p<nPrior;p++){
  //   if (GetQueueDiscClass (p)->GetQueueDisc ()->GetNBytes()>sat){
  //     DeqRate[p] = double(1.0/num);
  //   }
  //   else{
  //     DeqRate[p]=1;
  //   }
  // }
  for (uint32_t p = 0; p < nPrior; p++) {
    double th = 8 * Deq[p] / nanodelay / portBW; // portBW should be in Gbps
    if (th < 1.0 / double(nPrior) || th > 1) {
      th = 1;
    }
    DeqRate[p] = th;
    Deq[p] = 0;
  }
}

void GenQueueDisc::UpdateNofP() {
  for (uint32_t i = 0; i < nPrior; i++) {
    nofP[i] = sharedMemory->GetNofP(i);
  }
}


void GenQueueDisc::InvokeUpdates(double nanodelay) {
  UpdateDequeueRate(nanodelay);
  UpdateNofP();
  Simulator::Schedule(NanoSeconds(nanodelay), &GenQueueDisc::InvokeUpdates, this, nanodelay);
}

bool GenQueueDisc::ActiveBufferManagement(uint32_t priority, Ptr<Packet> packet) {

  double alpha = 1;

  /* A tag is attached by the end-hosts on all the packets which are unscheduled (first RTT bytes). Find the tag first.*/
  bool found;
  uint32_t unsched = 0;
  UnSchedTag tag;
  found = packet->PeekPacketTag (tag);
  if (found) {
    unsched = tag.GetValue();
  }

  /* prioritize unscheduled packets */
  if (unsched) {
    alpha = alphaUnsched;
  }
  else {
    alpha = alphas[priority];
  }

  // uint64_t currentSize = GetQueueDiscClass (priority)->GetQueueDisc ()->GetNBytes();
  uint64_t currentSize = sharedMemory->GetQueueSize(portId, priority);

  double satLevel = double(currentSize) / sat;
  if (satLevel > 1) {
    satLevel = 1;
  }


  sharedMemory->setSaturated(portId, priority, satLevel);

  if (firstTimeUpdate) {
    firstTimeUpdate = false;
    InvokeUpdates(updateInterval);
  }

  double remaining = sharedMemory->GetRemainingBuffer();
  // std::cout << "alpha " << alpha << " n " << nofP[priority] << " deq " << DeqRate[priority] << std::endl;
  uint64_t maxSize = double(alpha * (remaining) / nofP[priority]) * DeqRate[priority];

  if (maxSize > UINT32_MAX)
    maxSize = UINT32_MAX - 1500;

  // uint32_t qSize = GetQueueDiscClass (priority)->GetQueueDisc ()->GetNBytes();
  uint32_t qSize = sharedMemory->GetQueueSize(portId, priority);
  if ( ((qSize + packet->GetSize()) >  maxSize) || (sharedMemory->GetRemainingBuffer() < packet->GetSize())  ) {
    return false; // drop
  }
  else {
    return true;
  }
}


bool GenQueueDisc::FlowAwareBuffer(uint32_t priority, Ptr<Packet> packet) {

  double alpha;

  /* Find flow-id if exists */
  bool found;
  uint32_t flowId = 0;
  FlowIdTag tag;
  found = packet->PeekPacketTag (tag);
  if (found) {
    flowId = tag.GetFlowId();
  }

  /* Find the flow entry */
  if (FlowCount.find(flowId) == FlowCount.end()) {
    FlowCount[flowId].first = 0;
    FlowCount[flowId].second = Simulator::Now();
  }

  /* If the flow did not appear in the last FabWindow duration, reset its bytes counter to zero. */
  if (Simulator::Now() - FlowCount[flowId].second > FabWindow) {
    FlowCount[flowId].first = 0;
  }

  /* Per-flow counters - increment bytes count and last updated time. */
  FlowCount[flowId].first += packet->GetSize();
  FlowCount[flowId].second = Simulator::Now();

  /* If the flow sent less than FabThreshold no.of bytes in the last FabWindow, then prioritize these packets */
  if (FlowCount[flowId].first < FabThreshold) {
    alpha = alphaUnsched; // alphaUnsched is usually set to a high value i.e., these packets are prioritized.
  }
  else {
    alpha = alphas[priority];
  }

  double remaining = sharedMemory->GetRemainingBuffer();
  uint64_t maxSize = alpha * remaining;
  if (maxSize > UINT32_MAX)
    maxSize = UINT32_MAX - 1500;


  // uint32_t qSize = GetQueueDiscClass (priority)->GetQueueDisc ()->GetNBytes();
  uint32_t qSize = sharedMemory->GetQueueSize(portId, priority);
  if ( ((qSize + packet->GetSize()) >  maxSize) || (sharedMemory->GetRemainingBuffer() < packet->GetSize())  ) {
    return false; // drop
  }
  else {
    return true;
  }

}

bool GenQueueDisc::CompleteSharing(uint32_t priority, Ptr<Packet> packet) {
  if (sharedMemory->GetRemainingBuffer() < packet->GetSize()) {
    return false;// drop
  }
  else {
    return true;
  }
}



bool GenQueueDisc::LongestQueueDrop(uint32_t priority, Ptr<Packet> packet){ 
  // std::cout << "available " << sharedMemory->GetRemainingBuffer() << " occupied " << sharedMemory->GetOccupiedBuffer() << std::endl;
  if (sharedMemory->GetRemainingBuffer() < packet->GetSize()){
    bool available = false;
    // std::cout << "using LQD" << std::endl;
    while (!available){
      // std::cout << "pushing out" << std::endl;
      uint32_t *arr = sharedMemory->RemoveLongestQueuePacket();
      // std::cout << "got arr" << std::endl;
      // std::cout << arr[0] << std::endl;
      // std::cout << arr[0] << " " << arr[1] << std::endl;
      // std::cout << arr[0] << " " << arr[1]<< " " << arr[2] << std::endl;
      // std::cout << arr[0] << " " << arr[1]<< " " << arr[2] << " " << arr[3] << std::endl;
      m_traceLQD(arr[0], arr[1], arr[2], arr[3], 1); // 0 to indicate accepted, 1 for drop.
      if (sharedMemory->GetRemainingBuffer() < packet->GetSize())
        available = false;
      else
        available = true;
    }
    BufferLogTag buffertag;
    buffertag.setQueueLength(sharedMemory->GetQueueSize(portId,priority));
    buffertag.setAverageQueueLength(sharedMemory->getAverageQueueLength(portId,priority));
    buffertag.setOccupiedBuffer(sharedMemory->GetOccupiedBuffer());
    buffertag.setAverageOccupiedBuffer(sharedMemory->getAverageOccupancy());
    packet->AddPacketTag(buffertag);
    return true;
  }
  else{
    BufferLogTag buffertag;
    buffertag.setQueueLength(sharedMemory->GetQueueSize(portId,priority));
    buffertag.setAverageQueueLength(sharedMemory->getAverageQueueLength(portId,priority));
    buffertag.setOccupiedBuffer(sharedMemory->GetOccupiedBuffer());
    buffertag.setAverageOccupiedBuffer(sharedMemory->getAverageOccupancy());
    packet->AddPacketTag(buffertag);
    return true;
  }
}


bool GenQueueDisc::Credence(uint32_t priority, Ptr<Packet> packet, Ptr<QueueDiscItem> item){
  sharedMemory->UpdateThreshold(packet->GetSize(), portId, priority);
  uint32_t bufferSize = sharedMemory->GetSharedBufferSize();
  double numPorts = sharedMemory->getPorts();
  uint32_t qlen = sharedMemory->GetQueueSize(portId,priority);
  uint32_t sharedOccupancy = sharedMemory->GetOccupiedBuffer();
  uint32_t avgqlen = sharedMemory->getAverageQueueLength(portId,priority);
  uint32_t avgsharedoccupancy = sharedMemory->getAverageOccupancy();
  uint32_t longestQueueLength = sharedMemory->GetQueueSize(sharedMemory->findLongestQueue(),1);

  int drop = 0;
  if (longestQueueLength > bufferSize/numPorts &&  priority==1){
      if (qlen + packet->GetSize() <= sharedMemory->GetThreshold(portId, priority)){
        if (enablePredictions){
          m_getPrediction(qlen, avgqlen, sharedOccupancy, avgsharedoccupancy,drop);
        }

        bool insertFalse = (urv->GetValue()<addErr);
        if (insertFalse && addErr>0){
            if(drop){
              drop = 0;
            }
            else{
              drop = 1;
            }
        }
        // if (drop){
        //   std::cout << "drop " << drop << " Time " << Simulator::Now().GetNanoSeconds() << " " << sharedMemory->GetQueueSize(portId,priority) << " " << sharedMemory->getAverageQueueLength(portId,priority) << " " << sharedMemory->GetOccupiedBuffer() << " " << sharedMemory->getAverageOccupancy() << std::endl;
        // }
      }
      else{
        drop = 1;
      }
  }
  if (drop){
    return false;
  }
  else{
    return true;
  }
}




void
GenQueueDisc::SetQrefAfd(uint32_t p, uint32_t ref) {
  QRefAfd[p] = ref;
}
uint32_t
GenQueueDisc::GetQrefAfd(uint32_t p) {
  return QRefAfd[p];
}

int
GenQueueDisc::DropAfd(double prob, uint32_t priority) {
  // uint32_t qsize = GetQueueDiscClass (priority)->GetQueueDisc ()->GetNBytes();
  uint32_t qSize = sharedMemory->GetQueueSize(portId, priority);
  double x = double(rand()) / RAND_MAX;
  // 150*1024 is the recommended value for 10Gbps links https://www.cisco.com/c/en/us/products/collateral/switches/nexus-9000-series-switches/white-paper-c11-738488.html
  return ((x < prob) && (qSize > 150 * 1024));
}


bool GenQueueDisc::IntelligentBuffer(uint32_t priority, Ptr<Packet> packet) {
  bool accept;
  if (Simulator::Now() > AfdWindow + timeSinceLastChangeAdf) {
    for (auto it = M.begin(); it != M.end(); ++it) {
      it->second.first = it->second.second;
      it->second.second = 1; //1 just to avoid divide by zero errors
    }
    for (uint32_t i = 0; i < nPrior; i++) {
      // uint32_t Qnow = GetQueueDiscClass (i)->GetQueueDisc ()->GetNBytes();
      uint32_t Qnow = sharedMemory->GetQueueSize(portId, i);
      MFair[i] = MFair[i] - a1 * ((double)Qnow - (double)QRefAfd[i]) + a2 * ((double)Qold[i] - (double)QRefAfd[i]); // a1 and a2 --> 1.8 and 1.7
      if (MFair[i] < 0)
        MFair[i] = 0;

      Qold[i] = Qnow;
    }
    timeSinceLastChangeAdf = Simulator::Now();
  }

  bool found;
  uint32_t flowId = 0;
  FlowIdTag tag;
  found = packet->PeekPacketTag (tag);
  if (found) {flowId = tag.GetFlowId();}

  if (FlowCount.find(flowId) == FlowCount.end()) {
    FlowCount[flowId].first = 0;
    FlowCount[flowId].second = Simulator::Now();
  }

  //DPP
  if (Simulator::Now() - FlowCount[flowId].second > DppWindow)
    FlowCount[flowId].first = 0;

  FlowCount[flowId].first += 1;
  FlowCount[flowId].second = Simulator::Now();

  if (FlowCount[flowId].first < DppThreshold && enableDPPQueue) { // Short flows are sent to queue-0 which is a priority queue.
    DPPQueue = 0;
    accept = DynamicThresholds(DPPQueue, packet);
  }
  else {
    M[priority].second += packet->GetSize();

    if (!M[priority].first) {
      M[priority].first = 1; // Just to avoid divide by zero.
    }
    double dropP = 1.0 - (double(std::min(15 * M[priority].first, uint32_t(MFair[priority]))) / (15 * M[priority].first));
    if (dropP < 0) {
      dropP = 0;
    }

    DPPQueue = priority;
    accept = (DynamicThresholds(DPPQueue, packet) && !DropAfd(DPPQueue, dropP));
  }
  return accept;
}



bool GenQueueDisc::AcceptPacket(uint32_t priority, Ptr<Packet> packet, Ptr<QueueDiscItem> item) {
  bool accept;
  int drop = 0;
  switch (bufferalg) {
  case DT:
    accept = DynamicThresholds(priority, packet);
    break; // This comment is intended to remind myself and others some basics that break is essential :P
  case ABM:
    accept = ActiveBufferManagement(priority, packet);
    break; // This comment is intended to remind myself and others some basics that break is essential :P
  case FAB:
    accept = FlowAwareBuffer(priority, packet);
    break; // This comment is intended to remind myself and others some basics that break is essential :P
  case CS:
    accept = CompleteSharing(priority, packet);
    break; // This comment is intended to remind myself and others some basics that break is essential :P
  case IB:
    accept = IntelligentBuffer(priority, packet);
    break; // This comment is intended to remind myself and others some basics that break is essential :P
  case LQD:
    accept = LongestQueueDrop(priority,packet);
    break; // This comment is intended to remind myself and others some basics that break is essential :P
  case CREDENCE:
    accept = Credence(priority,packet, item);
    break;
  default:
    accept = DynamicThresholds(priority, packet);
  }
  return accept;
}

// void
// GenQueueDisc::TrimPacket(Ptr<Packet> packetCopy){
//   TcpHeader th; Ipv4Header ih; PppHeader ph; IntHeader inth; HomaHeader hh; FlowIdTag ft; MyPriorityTag mt;
//   uint32_t trimsize = 0;
//   uint32_t thremoved = packetCopy->RemoveHeader(th);
//   uint32_t ihremoved = packetCopy->RemoveHeader(ih);
//   uint32_t phremoved = packetCopy->RemoveHeader(ph);
//   uint32_t hhremoved = packetCopy->RemoveHeader(hh);
//   bool intremoved = packetCopy->RemovePacketTag(inth);
//   bool ftremoved = packetCopy->RemovePacketTag(ft);
//   bool mtremoved = packetCopy->RemovePacketTag(mt);
//   packetCopy->RemoveAtEnd(packetCopy->GetSize());
//   ft.SetTrim(1);
//   if(intremoved){packetCopy->AddPacketTag(inth);}
//   if(ftremoved){packetCopy->AddPacketTag(ft);}
//   if(mtremoved){packetCopy->AddPacketTag(mt);}
//   if(thremoved){packetCopy->AddHeader(th);}
//   if(hhremoved){packetCopy->AddHeader(hh);}
//   if(ihremoved){packetCopy->AddHeader(ih);}
//   if(phremoved){packetCopy->AddHeader(ph);}

//   std::cout << packetCopy->GetSize() << std::endl;
// }

bool
GenQueueDisc::DoEnqueue (Ptr<QueueDiscItem> item)
{
  NS_LOG_FUNCTION (this << item);

  Ptr<Packet> packet = item->GetPacket();
  uint32_t p = 0;

  bool found;
  MyPriorityTag a;
  found = packet->PeekPacketTag(a);
  if (found){
    p = a.GetPriority();
  }

  if (p >= nPrior)
    p = uint32_t(nPrior - 1);

  /* Arrival Statistics*/
  numBytesSent[p] += item->GetSize();
  // uint64_t sizenow = GetQueueDiscClass (p)->GetQueueDisc ()->GetNBytes();
  uint64_t sizenow = sharedMemory->GetQueueSize(portId, p);
  if (bufferMax[p] < sizenow) {
    bufferMax[p] = sizenow;
  }
  /*Check if we can use the reserved space*/
  if (GetCurrentSize().GetValue() + item->GetSize() < staticBuffer) {
    bool ret = GetQueueDiscClass (p)->GetQueueDisc ()->Enqueue (item);

    if (firstSeen[p] == Seconds(0)) {
      firstSeen[p] = Simulator::Now();
    }
    lastAccepted[p] = Simulator::Now();
    return ret;
  }

  /*Check if the packet can be put in the shared buffer*/
  bool enqueue = AcceptPacket(p, packet, item);
  if (!enqueue) {

    NS_LOG_LOGIC ("Queue disc limit exceeded -- dropping packet");
    // std::cout << " maxSize " << maxSize << " remaining " << sharedMemory->GetRemainingBuffer() << " packetSize " << item->GetSize() << " priority " << uint32_t(p) << " alpha " << alphas[p] << " thresh " << uint64_t (alphas[p]*(sharedMemory->GetRemainingBuffer())) << " deq " << DeqRate[p] << " N " << sharedMemory->GetNofP(p) << std::endl;
    m_rxTrace(packet, p, false, this); // trace packet arrival event at queue p
    DropBeforeEnqueue (item, LIMIT_EXCEEDED_DROP);
    // std::cout << "dropped" << std::endl;
    return false;
  }
  else{
    m_rxTrace(packet, p, true, this); // trace packet arrival event at queue p
  }

  /*If algorithm is Intelligent Buffer, it may change the queue to zero (DPP prioritizes short flows to separate queue)*/
  if (bufferalg == IB && enableDPPQueue) {
    p = DPPQueue;
  }

  /*increment shared buffer occupancy*/
  bool retval;
  uint32_t unsched = 0;
  UnSchedTag tag;
  found = packet->PeekPacketTag (tag);
  if (found) {
    unsched = tag.GetValue();
  }
  if (!sharedMemory->EnqueueBuffer(item->GetPacket()->GetSize(), portId, p)) {
    DropBeforeEnqueue (item, LIMIT_EXCEEDED_DROP);
    // std::cout << "droppedTimeEEEEEEEEEEE " << Simulator::Now().GetNanoSeconds() << " priority " << p << " " << sharedMemory->GetQueueSize(portId,p) << " " << sharedMemory->getAverageQueueLength(portId,p) << " " << sharedMemory->GetOccupiedBuffer() << " " << sharedMemory->getAverageOccupancy() << std::endl;
    // std::cout << "dropped" << std::endl;
    retval = false;
    if (p==1){
      m_arrival(item->GetPacket()->GetSize(), numPackets_big, numPackets_small, unsched, Simulator::Now().GetNanoSeconds(), Simulator::Now().GetSeconds(), sharedMemory->GetQueueSize(portId, p), 0, portId, p);
    }
  }
  else {
    sharedMemory->PerPriorityStatEnq(item->GetPacket()->GetSize(), p);
    if (p==1){
      if (item->GetPacket()->GetSize() > 32){
        numPackets_big += 1;
      }
      else{
        numPackets_small += 1;
      }
      m_arrival(item->GetPacket()->GetSize(), numPackets_big, numPackets_small, unsched, Simulator::Now().GetNanoSeconds(), Simulator::Now().GetSeconds(), sharedMemory->GetQueueSize(portId, p), 1, portId, p);
    }
    retval = GetQueueDiscClass (p)->GetQueueDisc ()->Enqueue (item);
  }

  if (!retval)
  {
    NS_LOG_WARN ("Packet enqueue failed. Check the size of the internal queues");
  }
  else {
    if (firstSeen[p] == Seconds(0)) {
      firstSeen[p] = Simulator::Now();
    }
    lastAccepted[p] = Simulator::Now();
  }

  NS_LOG_LOGIC ("Number packets p " << p << ": " << GetQueueDiscClass (p)->GetQueueDisc ()->GetNPackets ());

  return retval;
}


double
GenQueueDisc::GetThroughputQueue(uint32_t p, double nanodelay) {
  double th = 8 * numBytesSentQueue[p] / nanodelay / portBW;
  numBytesSentQueue[p] = 0;
  return th;
}

double
GenQueueDisc::GetThroughputPort(double nanodelay) { // delay must be in nanoseconds
  double th = 8 * numBytesSentQueue[10] / nanodelay / portBW;
  numBytesSentQueue[10] = 0;
  return th;
}

Ptr<QueueDiscItem>
GenQueueDisc::DoDequeue (void)
{
  NS_LOG_FUNCTION (this);

  Ptr<QueueDiscItem> item;

  if (round_robin) {
  /* Round robin scheduling */
    for (uint32_t i = 0; i < GetNQueueDiscClasses(); i++)
    {
      if ((item = GetQueueDiscClass (dequeueIndex)->GetQueueDisc ()->Dequeue ()) != 0)
      {
        // return item;
        break;
      }
      Deq[dequeueIndex] += 1472;

      dequeueIndex++;
      if (dequeueIndex >= GetNQueueDiscClasses())
        dequeueIndex = 0;
    }
  }
  else {
    /*Strict priority scheduling*/
    for (uint32_t i = 0; i < GetNQueueDiscClasses(); i++)
    {
      if ((item = GetQueueDiscClass (i)->GetQueueDisc ()->Dequeue ()) != 0)
      {
        // return item;
        dequeueIndex = i;
        break;
      }
      Deq[i] += 1472;
    }
  }

  // NS_LOG_LOGIC ("Queue empty"); // We break out of loop instead of returning. So it doesn't mean that the queue is empty if we arrive here.
  if (item) {

    Ptr<Packet> packet = item->GetPacket();
    
    m_txTrace(packet, dequeueIndex, this); // trace dequeue event from dequeueIndex queue
    if (bufferalg == LQD){
      uint32_t arr[4]={0,0,0,0};
      BufferLogTag buffertag;
      bool found = item->GetPacket()->PeekPacketTag(buffertag);
      if(found){
        arr[0] = buffertag.getQueueLength();
        arr[1] = buffertag.getAverageQueueLength();
        arr[2] = buffertag.getOccupiedBuffer();
        arr[3] = buffertag.getAverageOccupiedBuffer();
      }
      m_traceLQD(arr[0], arr[1], arr[2], arr[3], 0); // 0 to indicate accepted, 1 for drop.
      bool gone = packet->RemovePacketTag (buffertag);
    }

    uint32_t p = dequeueIndex;

    numBytesSentQueue[p] += item->GetSize();

    // 10 is used for aggregate. Assuming that the actual number of queues are less than 10.
    numBytesSentQueue[10] += item->GetSize();

    Deq[p] += item->GetSize();
    if (GetCurrentSize().GetValue() + packet->GetSize() > staticBuffer) {
      sharedMemory->DequeueBuffer(item->GetPacket()->GetSize(), portId, p);
      sharedMemory->PerPriorityStatDeq(item->GetPacket()->GetSize(), p);
    }

    if (enableINT) {
      FeedbackTag Int;
      bool found;
      found = packet->PeekPacketTag(Int);
      if (found) {
        Int.setTelemetryQlenDeq(Int.getHopCount(), sharedMemory->GetQueueSize(portId, p)); // queue length at dequeue
        Int.setTelemetryTsDeq(Int.getHopCount(), Simulator::Now().GetNanoSeconds()); // timestamp at dequeue
        Int.setTelemetryBw(Int.getHopCount(), portBW * 1e9);
        Int.setTelemetryTxBytes(Int.getHopCount(), txBytesInt);
        Int.incrementHopCount(); // Incrementing hop count at Dequeue. Don't do this at enqueue.
        packet->ReplacePacketTag(Int); // replacing the tag with new values
        // std::cout << "found " << Int.getHopCount() << std::endl;
      }
      txBytesInt += packet->GetSize();
    }


    if (p==1){
      uint32_t unsched = 0;
      UnSchedTag tag;
      bool found = packet->PeekPacketTag (tag);
      if (found) {
        unsched = tag.GetValue();
      }
      if (item->GetPacket()->GetSize() > 32){
        numPackets_big = (numPackets_big==0)?0:numPackets_big-1;
      }
      else{
        numPackets_small = (numPackets_small==0)?0:numPackets_small-1;
      }
      m_departure(item->GetPacket()->GetSize(), numPackets_big, numPackets_small, unsched, Simulator::Now().GetNanoSeconds(), Simulator::Now().GetSeconds(), sharedMemory->GetQueueSize(portId, p), 2, portId, p);
    }

    dequeueIndex++; // This increment is for the round-robin case.
    if (dequeueIndex >= GetNQueueDiscClasses())
      dequeueIndex = 0;

  }
  // if (round_robin){
  //   dequeueIndex++; // This increment is for the round-robin case.
  //   if (dequeueIndex >= GetNQueueDiscClasses())
  //     dequeueIndex = 0;
  // }
  return item;
}

Ptr<const QueueDiscItem>
GenQueueDisc::DoPeek (void)
{
  NS_LOG_FUNCTION (this);

  Ptr<const QueueDiscItem> item;

  for (uint32_t i = 0; i < GetNQueueDiscClasses (); i++)
  {
    if ((item = GetQueueDiscClass (i)->GetQueueDisc ()->Peek ()) != 0)
    {
      NS_LOG_LOGIC ("Peeked from band " << i << ": " << item);
      NS_LOG_LOGIC ("Number packets band " << i << ": " << GetQueueDiscClass (i)->GetQueueDisc ()->GetNPackets ());
      return item;
    }
  }

  NS_LOG_LOGIC ("Queue empty");
  return item;
}

bool
GenQueueDisc::CheckConfig (void)
{
  NS_LOG_FUNCTION (this);
  if (GetNInternalQueues () > 0)
  {
    NS_LOG_ERROR ("GenQueueDisc cannot have internal queues");
    return false;
  }

  if (GetNQueueDiscClasses () == 0)
  {
    // create 3 fifo queue discs
    ObjectFactory factory;
    factory.SetTypeId ("ns3::FifoQueueDisc");
    for (uint8_t i = 0; i < 2; i++)
    {
      Ptr<QueueDisc> qd = factory.Create<QueueDisc> ();
      qd->Initialize ();
      Ptr<QueueDiscClass> c = CreateObject<QueueDiscClass> ();
      c->SetQueueDisc (qd);
      AddQueueDiscClass (c);
    }
  }

  if (GetNQueueDiscClasses () < 2)
  {
    NS_LOG_ERROR ("GenQueueDisc needs at least 2 classes");
    return false;
  }

  return true;
}

void
GenQueueDisc::InitializeParams (void)
{
  NS_LOG_FUNCTION (this);
}

} // namespace ns3
