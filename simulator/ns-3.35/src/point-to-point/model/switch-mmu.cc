#include <iostream>
#include <fstream>
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/object-vector.h"
#include "ns3/uinteger.h"
#include "ns3/log.h"
#include "ns3/assert.h"
#include "ns3/global-value.h"
#include "ns3/boolean.h"
#include "ns3/simulator.h"
#include "ns3/random-variable.h"
#include "switch-mmu.h"

#define LOSSLESS 0
#define LOSSY 1
#define DUMMY 2

# define DT 101
# define FAB 102
# define CS 103
# define IB 104
# define ABM 110
# define REVERIE 111

NS_LOG_COMPONENT_DEFINE("SwitchMmu");
namespace ns3 {
TypeId SwitchMmu::GetTypeId(void) {
	static TypeId tid = TypeId("ns3::SwitchMmu")
	                    .SetParent<Object>()
	                    .AddConstructor<SwitchMmu>();
	return tid;
}

/*
We model the switch shared memory (purely based on our understanding and experience).
The switch has an on-chip buffer which has `bufferPool` size.
This buffer is shared across all port and queues in the switch.

`bufferPool` is further split into multiple pools at the ingress and egress.

It would be easier to understand from here on if you consider Ingress/Egress are merely just counters.
These are not separate buffer locations or chips...!

First, `ingressPool` (size) accounts for ingress buffering shared by both lossy and lossless traffic.
Additionally, there exists a headroom pool of size xoffTotal,
and each queue may use xoff[port][q] configurable amount at each port p and queue q.
When a queue at the ingress exceeds its ingress threshold, a PFC pause message is sent and
any incoming packets can use upto a maximum of xoff[port][q] headroom.

Second, at the egress, `egressPool[LOSSY]` (size) accounts for buffering lossy traffic at the egress and
similarly `egressPool[LOSSLESS]` for lossless traffic.
*/


SwitchMmu::SwitchMmu(void) {

	// Here we just initialize some default values.
	// The buffer can be configured using Set functions through the simulation file later.

	// Buffer model
	bufferModel = "sonic"; // currently SONiC buffer (based on our understanding) and "reverie" buffer model are supported. The bufferModel can be set using SetBufferModel function externally.

	// Buffer pools
	bufferPool = 24 * 1024 * 1024; // ASIC buffer size i.e, total shared buffer
	ingressPool = 18 * 1024 * 1024; // Size of ingress pool. Note: This is shared by both lossless and lossy traffic.
	egressPool[LOSSLESS] = 24 * 1024 * 1024; // Size of egress lossless pool. Lossless bypasses egress admission
	egressPool[LOSSY] = 14 * 1024 * 1024; // Size of egress lossy pool.
	sharedPool = 18 * 1024 * 1024; // For Reverie which maintains a single shared buffer pool, all lossless and lossy share this pool
	egressPoolAll = 24 * 1024 * 1024; // Not for now. For later use.
	xoffTotal = 0; //6 * 1024 * 1024; // Total headroom space in the shared buffer pool.
	// xoffTotal value is incremented when SetHeadroom function is used. So setting it to zero initially.
	// Note: This would mean that headroom must be set explicitly.
	totalIngressReserved = 0;
	totalIngressReservedUsed = 0;


	// aggregate run time
	// `totalUsed` IMPORTANT TO NOTE: THIS IS NOT bytes in the "ingress pool".
	// This is the total bytes USED in the switch buffer, which includes occupied buffer in reserved + headroom + ingresspool.
	totalUsed = 0;
	egressPoolUsed[LOSSLESS] = 0; // Total bytes USED in the egress lossless pool
	egressPoolUsed[LOSSY] = 0; // Total bytes USED in the egress lossy pool
	xoffTotalUsed = 0; // Total headroom bytes USED so far. Updated at runtime.
	sharedPoolUsed = 0; // For Reverie: total shared pool used buffer.
	// It is sometimes useful to keep track of total bytes used specifically from ingressPool. We don't need an additional variable.
	// This is equal to (totalUsed - xoffTotalUsed).

	Reveriegamma = 0.99;

	for (uint32_t port = 0; port < pCnt; port++) {
		for (uint32_t q = 0; q < qCnt; q++) {
			// buffer configuration.
			reserveIngress[port][q] = 0; // Per queue reserved buffer at ingress. IMPORTANT: reserve SHOULD BE SET EXPLICITLY in a simulation.
			reserveEgress[port][q] = 0; // per queue reserved buffer at egress. Not used at the moment. TODO.
			alphaEgress[port][q] = 1; // per queue alpha value used by Buffer Management/PFC Threshold at egress
			alphaIngress[port][q] = 1; // per queue alpha value used by Buffer Management/PFC Threshold at ingress
			xoff[port][q] = 0; // per queue headroom LIMIT at ingress. This can be changed using SetHeadroom. IMPORTANT: xoff SHOULD BE SET EXPLICITLY in a simulation.
			xon[port][q] = 1248; // For pfc resume. Can be changed using SetXon
			xon_offset[port][q] = 2496; // For pfc resume. Can be changed using SetXonOffset


			// per queue run time
			ingress_bytes[port][q] = 0; // total ingress bytes USED at each queue. This includes, bytes from reserved, ingress pool as well as any headroom.
			// MMU maintains paused state for all Ingress queues to keep track if a queue is currently pausing the peer (an egress queue on the other end of the link)
			// NOTE: QbbNetDevices (ports) maintain a separate paused state to keep track if an egress queue is paused or not. This can be found in qbb-net-device.cc
			paused[port][q] = 0; // a state (see above).
			egress_bytes[port][q] = 0; // Per queue egress bytes USED at each queue
			xoffUsed[port][q] = 0; // The headroom buffer USED by each queue.
			ingressLpf_bytes[port][q] = 0;
			egressLpf_bytes[port][q] = 0;

			// ABM related variables
			congestedIngress[port][q] = 0; // This keeps track of the number of congested queues at the ingress
			congestedEgress[port][q] = 0; // This keeps track of the number of congested queues at the egress
			txBytesIngress[port][q] = 0; // used for calculating dequeue rates. counter for tx bytes of ingress queues
			txBytesEgress[port][q] = 0; // used for calculating dequeue rates. counter for tx bytes of egress queues
			dequeueRateIngress[port][q] = 1; // normalized dequeue rate of an ingress queue
			dequeueRateEgress[port][q]  = 1; // normalized dequeue rate of an egress queue
		}
	}

	for (uint32_t qIndex = 0; qIndex < qCnt; qIndex++) {
		NofPIngress[qIndex] = 0;
		NofPEgress[qIndex] = 0;
	}
	for (uint32_t portId = 0; portId < pCnt; portId++) {
		bandwidth[portId] = 25 * 1e9;
	}
	congestionIndicator = 20 * 1024;

	ingressAlg[LOSSLESS] = DT;
	ingressAlg[LOSSY] = DT;
	egressAlg[LOSSLESS] = DT;
	egressAlg[LOSSY] = DT;


	memset(ingress_bytes, 0, sizeof(ingress_bytes));
	memset(paused, 0, sizeof(paused));
	memset(egress_bytes, 0, sizeof(egress_bytes));

	dequeueUpdatedOnce = 0; // For ABM, to trigger dequeue rate updates
	lpfUpdatedOnce = 0; // For Reverie, LPF updates
	updateIntervalNS = 25 * 1000; // default 25us update interval for dequeue rates
	alphaHigh = 1024; // default value to imitate a sky high threshold for all unscheduled packets
	portCount = pCnt; // default value is 257. This should be set to the real port count using SetPortCount function externally based on the simulation setup
}

void
SwitchMmu::SetBufferPool(uint64_t b) {
	bufferPool = b;
}

void
SwitchMmu::SetIngressPool(uint64_t b) {
	ingressPool = b;
}

void
SwitchMmu::SetSharedPool(uint64_t b) {
	sharedPool = b;
}

void
SwitchMmu::SetEgressPoolAll(uint64_t b) {
	egressPoolAll = b;
}

void
SwitchMmu::SetEgressLossyPool(uint64_t b) {
	egressPool[LOSSY] = b;
}

void
SwitchMmu::SetEgressLosslessPool(uint64_t b) {
	egressPool[LOSSLESS] = b;
}

void
SwitchMmu::SetReserved(uint64_t b, uint32_t port, uint32_t q, std::string inout) {
	if (inout == "ingress") {
		if (totalIngressReserved >= reserveIngress[port][q])
			totalIngressReserved -= reserveIngress[port][q];
		else
			totalIngressReserved = 0;
		reserveIngress[port][q] = b;
		totalIngressReserved += reserveIngress[port][q];
	}
	else if (inout == "egress") {
		std::cout << "setting reserved for egress is not supported. Exiting..!" << std::endl;
		exit(1);
		// reserveEgress[port][q] = b;
	}
}

void
SwitchMmu::SetReserved(uint64_t b, std::string inout) {
	if (inout == "ingress") {
		for (uint32_t port = 0; port < pCnt; port++) {
			for (uint32_t q = 0; q < qCnt ; q++) {
				if (totalIngressReserved >= reserveIngress[port][q])
					totalIngressReserved -= reserveIngress[port][q];
				else
					totalIngressReserved = 0;
				reserveIngress[port][q] = b;
				totalIngressReserved += reserveIngress[port][q];
			}
		}
	}
	else if (inout == "egress") {
		std::cout << "setting reserved for egress is not supported. Exiting..!" << std::endl;
		exit(1);
		// for (uint32_t port = 0; port < pCnt; port++) {
		// 	for (uint32_t q = 0; q < qCnt; q++) {
		// 		reserveEgress[port][q] = b;
		// 	}
		// }
	}
}

void
SwitchMmu::SetAlphaIngress(double value, uint32_t port, uint32_t q) {
	alphaIngress[port][q] = value;
}

void
SwitchMmu::SetAlphaIngress(double value) {
	for (uint32_t port = 0; port < pCnt; port++) {
		for (uint32_t q = 0; q < qCnt; q++) {
			alphaIngress[port][q] = value;
		}
	}
}

void
SwitchMmu::SetAlphaEgress(double value, uint32_t port, uint32_t q) {
	alphaEgress[port][q] = value;
}

void
SwitchMmu::SetAlphaEgress(double value) {
	for (uint32_t port = 0; port < pCnt; port++) {
		for (uint32_t q = 0; q < qCnt; q++) {
			alphaEgress[port][q] = value;
		}
	}
}


// This function allows for setting headroom per queue. When ever this is set, the xoffTotal (total headroom) is updated.
void
SwitchMmu::SetHeadroom(uint64_t b, uint32_t port, uint32_t q) {
	xoffTotal -= xoff[port][q];
	xoff[port][q] = b;
	xoffTotal += xoff[port][q];
}

// This function allows for setting headroom for all queues in oneshot. When ever this is set, the xoffTotal (total headroom) is updated.
void
SwitchMmu::SetHeadroom(uint64_t b) {
	for (uint32_t port = 0; port < pCnt; port++) {
		for (uint32_t q = 0; q < qCnt; q++) {
			xoffTotal -= xoff[port][q];
			xoff[port][q] = b;
			xoffTotal += xoff[port][q];
		}
	}
}

void
SwitchMmu::SetXon(uint64_t b, uint32_t port, uint32_t q) {
	xon[port][q] = b;
}
void
SwitchMmu::SetXon(uint64_t b) {
	for (uint32_t port = 0; port < pCnt; port++) {
		for (uint32_t q = 0; q < qCnt; q++) {
			xon[port][q] = b;
		}
	}
}

void
SwitchMmu::SetXonOffset(uint64_t b, uint32_t port, uint32_t q) {
	xon_offset[port][q] = b;
}
void
SwitchMmu::SetXonOffset(uint64_t b) {
	for (uint32_t port = 0; port < pCnt; port++) {
		for (uint32_t q = 0; q < qCnt; q++) {
			xon_offset[port][q] = b;
		}
	}
}

void
SwitchMmu::SetGamma(double value) {
	Reveriegamma = value;
}

void
SwitchMmu::SetIngressLossyAlg(uint32_t alg) {
	ingressAlg[LOSSY] = alg;
}

void
SwitchMmu::SetIngressLosslessAlg(uint32_t alg) {
	ingressAlg[LOSSLESS] = alg;
}

void
SwitchMmu::SetEgressLossyAlg(uint32_t alg) {
	egressAlg[LOSSY] = alg;
}

void
SwitchMmu::SetEgressLosslessAlg(uint32_t alg) {
	egressAlg[LOSSLESS] = alg;
}

uint64_t SwitchMmu::GetIngressReservedUsed() {
	return totalIngressReservedUsed;
}

uint64_t SwitchMmu::GetIngressReservedUsed(uint32_t port, uint32_t qIndex) {
	if (ingress_bytes[port][qIndex] > reserveIngress[port][qIndex]) {
		return reserveIngress[port][qIndex];
	}
	else {
		return ingress_bytes[port][qIndex];
	}
}

uint64_t SwitchMmu::GetIngressSharedUsed() {
	return (totalUsed - xoffTotalUsed - totalIngressReservedUsed);
}

// DT's threshold = Alpha x remaining.
// A sky high threshold for a queue can be emulated by setting the corresponding alpha to a large value. eg., UINT32_MAX
uint64_t SwitchMmu::DynamicThreshold(uint32_t port, uint32_t qIndex, std::string inout, uint32_t type) {
	if (inout == "ingress") {
		double remaining = 0;
		uint64_t ingressPoolSharedUsed = GetIngressSharedUsed(); // Total bytes used from the ingress "shared" pool specifically.
		uint64_t ingressSharedPool = ingressPool - totalIngressReserved;
		if (ingressSharedPool > ingressPoolSharedUsed) {
			uint64_t remaining = ingressSharedPool - ingressPoolSharedUsed;
			return std::min(uint64_t(alphaIngress[port][qIndex] * (remaining)), UINT64_MAX - 1024 * 1024);
		}
		else {
			// ingressPoolShared is full. There is no `remaining` buffer in ingressPoolShared.
			// DT's threshold returns zero in this case, but using if else just to avoid threshold computations even in the simple case.
			return 0;
		}
	}
	else if (inout == "egress") {
		double remaining = 0;
		if (egressPool[type] > egressPoolUsed[type]) {
			uint64_t remaining = egressPool[type] - egressPoolUsed[type];
			// UINT64_MAX - 1024*1024 is just a randomly chosen big value.
			// Just don't want to return UINT64_MAX value, sometimes causes overflow issues later.
			uint64_t threshold = std::min(uint64_t(alphaEgress[port][qIndex] * (remaining)), UINT64_MAX - 1024 * 1024);
			return threshold;
		}
		else {
			return 0;
		}
	}
}
void SwitchMmu::setCongested(uint32_t portId, uint32_t qIndex, std::string inout, double satLevel) {
	if (inout == "ingress") {
		// NofPIngress[qIndex] -= congestedIngress[portId][qIndex];
		// if (ingressLpf_bytes[portId][qIndex] > congestionIndicator){
		// 	NofPIngress[qIndex] += 1;
		// 	congestedIngress[portId][qIndex] = 1;
		// }
		// else{
		// 	congestedIngress[portId][qIndex] = 0;
		// }
		NofPIngress[qIndex] +=  satLevel - congestedIngress[portId][qIndex];
		congestedIngress[portId][qIndex] = satLevel;
	}
	else if (inout == "egress") {
		// NofPEgress[qIndex] -= congestedEgress[portId][qIndex];
		// if (egressLpf_bytes[portId][qIndex] > congestionIndicator){
		// 	NofPEgress[qIndex] += 1;
		// 	congestedEgress[portId][qIndex] = 1;
		// }
		// else{
		// 	congestedEgress[portId][qIndex] = 0;
		// }
		NofPEgress[qIndex] += satLevel - congestedEgress[portId][qIndex];
		congestedEgress[portId][qIndex] = satLevel;
	}
}
double SwitchMmu::GetNofP(std::string inout, uint32_t qIndex) {
	if (inout == "ingress") {
		if (NofPIngress[qIndex] < 1)
			return 1;
		else
			return NofPIngress[qIndex];
	}
	else if (inout == "egress") {
		if (NofPEgress[qIndex] < 1)
			return 1;
		else
			return NofPEgress[qIndex];
	}
	return 0;
}
double SwitchMmu::getDequeueRate(uint32_t port, uint32_t qIndex, std::string inout) {
	if (inout == "ingress") {
		return dequeueRateIngress[port][qIndex];
	}
	else if (inout == "egress") {
		return dequeueRateEgress[port][qIndex];
	}
	return 0;
}
void SwitchMmu::updateDequeueRates() {
	for (uint32_t i = 0; i < portCount; i++) {
		for (uint32_t j = 0; j < qCnt; j++) {
			// update ingress queues dequeue rates
			uint64_t temp = txBytesIngress[i][j];
			txBytesIngress[i][j] = 0;
			double temp1 = (1e9 * temp * 8.0 / updateIntervalNS) / (bandwidth[i]);
			if (ingress_bytes[i][j] > congestionIndicator && temp > 2 * 1024)
				dequeueRateIngress[i][j] = temp1;
			else
				dequeueRateIngress[i][j] = 1;
			// if (dequeueRateIngress[i][j] < 0.125) // min 1/8 considering 8 queues, with round-robin
			// 	dequeueRateIngress[i][j] = 0.125;

			//update egress queues dequeue rates
			temp = txBytesEgress[i][j];
			txBytesEgress[i][j] = 0;
			temp1 = (1e9 * temp * 8.0 / updateIntervalNS) / (bandwidth[i]);
			if (egress_bytes[i][j] > congestionIndicator && temp > 2 * 1024)
				dequeueRateEgress[i][j] = temp1;
			else
				dequeueRateEgress[i][j] = 1;
			// dequeueRateEgress[i][j] = 0.125 + (0.875)*(temp1*0.8 + dequeueRateEgress[i][j]*0.2);
			// dequeueRateEgress[i][j] = (1e9 * temp * 8.0 / updateIntervalNS) / (bandwidth[i]);
			// if (dequeueRateEgress[i][j] < 0.125) // min 1/8 considering 8 queues, with round-robin
			// 	dequeueRateEgress[i][j] = 0.125;
		}
	}
	dequeueUpdatedOnce = 1;
	Simulator::Schedule(NanoSeconds(updateIntervalNS), &SwitchMmu::updateDequeueRates, this);
}

uint64_t SwitchMmu::ActiveBufferManagement(uint32_t port, uint32_t qIndex, std::string inout, uint32_t type, uint32_t unsched) {
	if (!dequeueUpdatedOnce) {
		updateDequeueRates();
	}
	if (inout == "ingress") {
		double remaining = 0;
		uint64_t ingressPoolSharedUsed = GetIngressSharedUsed(); // Total bytes used from the ingress "shared" pool specifically.
		uint64_t ingressSharedPool = ingressPool - totalIngressReserved;
		double satLevel = double(ingress_bytes[port][qIndex]) / congestionIndicator;
		if (satLevel > 1) {
			satLevel = 1;
		}
		setCongested(port, qIndex, inout, satLevel);
		if (ingressSharedPool > ingressPoolSharedUsed) {
			uint64_t remaining = ingressSharedPool - ingressPoolSharedUsed;
			double alphaP = 1;
			if (unsched) {
				alphaP = alphaHigh;
			}
			else {
				alphaP = alphaIngress[port][qIndex];
			}
			uint64_t ABM_Threshold = alphaP * (remaining) * (1.0 / GetNofP(inout, qIndex)) * (getDequeueRate(port, qIndex, inout));
			// if (type == LOSSLESS)
			// 	std::cout << getDequeueRate(port, qIndex, inout) << " port " << port  << " qIndex " << qIndex << std::endl;
			return std::min(uint64_t(ABM_Threshold), UINT64_MAX - 1024 * 1024);
		}
		else {
			// ingressPoolShared is full. There is no `remaining` buffer in ingressPoolShared.
			// DT's threshold returns zero in this case, but using if else just to avoid threshold computations even in the simple case.
			return 0;
		}
	}
	else if (inout == "egress") {
		double remaining = 0;
		double satLevel = double(egress_bytes[port][qIndex]) / congestionIndicator;
		if (satLevel > 1) {
			satLevel = 1;
		}
		setCongested(port, qIndex, inout, satLevel);
		if (egressPool[type] > egressPoolUsed[type]) {
			uint64_t remaining = egressPool[type] - egressPoolUsed[type];
			// UINT64_MAX - 1024*1024 is just a randomly chosen big value.
			// Just don't want to return UINT64_MAX value, sometimes causes overflow issues later.
			double alphaP = 1;
			if (unsched) {
				alphaP = alphaHigh;
			}
			else {
				alphaP = alphaEgress[port][qIndex];
			}
			uint64_t ABM_Threshold = alphaP * (remaining) * (1.0 / GetNofP(inout, qIndex)) * (getDequeueRate(port, qIndex, inout));
			return std::min(ABM_Threshold, UINT64_MAX - 1024 * 1024);
		}
		else {
			return 0;
		}
	}
}

uint64_t SwitchMmu::FlowAwareBuffer(uint32_t port, uint32_t qIndex, std::string inout, uint32_t type, uint32_t unsched) {
	if (inout == "ingress") {
		double remaining = 0;
		uint64_t ingressPoolSharedUsed = GetIngressSharedUsed(); // Total bytes used from the ingress "shared" pool specifically.
		uint64_t ingressSharedPool = ingressPool - totalIngressReserved;
		if (ingressSharedPool > ingressPoolSharedUsed) {
			uint64_t remaining = ingressSharedPool - ingressPoolSharedUsed;
			double alphaP = 1;
			if (unsched) {
				alphaP = alphaHigh;
			}
			else {
				alphaP = alphaIngress[port][qIndex];
			}
			uint64_t FAB_Threshold = alphaP * (remaining);
			return std::min(uint64_t(FAB_Threshold), UINT64_MAX - 1024 * 1024);
		}
		else {
			// ingressPoolShared is full. There is no `remaining` buffer in ingressPoolShared.
			// DT's threshold returns zero in this case, but using if else just to avoid threshold computations even in the simple case.
			return 0;
		}
	}
	else if (inout == "egress") {
		double remaining = 0;
		if (egressPool[type] > egressPoolUsed[type]) {
			uint64_t remaining = egressPool[type] - egressPoolUsed[type];
			// UINT64_MAX - 1024*1024 is just a randomly chosen big value.
			// Just don't want to return UINT64_MAX value, sometimes causes overflow issues later.
			double alphaP = 1;
			if (unsched) {
				alphaP = alphaHigh;
			}
			else {
				alphaP = alphaEgress[port][qIndex];
			}
			uint64_t FAB_Threshold = alphaP * (remaining);
			return std::min(FAB_Threshold, UINT64_MAX - 1024 * 1024);
		}
		else {
			return 0;
		}
	}
}



uint64_t SwitchMmu::ReverieThreshold(uint32_t port, uint32_t qIndex, uint32_t type, uint32_t unsched) {
	if (type == LOSSLESS) {
		// double remaining = 0;
		double satLevel = double(ingressLpf_bytes[port][qIndex]) / congestionIndicator;
		if (satLevel > 1) {
			satLevel = 1;
		}
		setCongested(port, qIndex, "ingress", satLevel);
		
		// uint64_t ingressPoolSharedUsed = GetIngressSharedUsed(); // Total bytes used from the ingress "shared" pool specifically.
		// uint64_t ingressSharedPool = ingressPool - totalIngressReserved;
		// if (ingressSharedPool > ingressPoolSharedUsed) {
		// 	uint64_t remaining = ingressSharedPool - ingressPoolSharedUsed;
		// 	return std::min(uint64_t(alphaIngress[port][qIndex] * (remaining)), UINT64_MAX - 1024 * 1024);
		// }
		// else {
		// 	// ingressPoolShared is full. There is no `remaining` buffer in ingressPoolShared.
		// 	// DT's threshold returns zero in this case, but using if else just to avoid threshold computations even in the simple case.
		// 	return 0;
		// }
		uint64_t sharedusedbuffer = sharedPoolUsed; //GetIngressSharedUsed();
		uint64_t sharedbuffer = sharedPool; // ingressPool - totalIngressReserved;
		if (sharedbuffer > sharedusedbuffer ) {
			uint64_t remaining = sharedbuffer - sharedusedbuffer;
			double alphaP = alphaIngress[port][qIndex];
			uint64_t Reverie_Threshold = alphaP * (remaining) * (1.0 / GetNofP("ingress", qIndex)) ;//+ (ingress_bytes[port][qIndex] - ingressLpf_bytes[port][qIndex]);
			return std::min(uint64_t(Reverie_Threshold), UINT64_MAX - 1024 * 1024);
		}
		else {
			// SharedPool is full. There is no `remaining` buffer.
			// The threshold returns zero in this case, but using if else just to avoid threshold computations even in the simple case.
			return 0;
		}
	}
	else if (type == LOSSY) {
		// double remaining = 0;
		double satLevel = double(egressLpf_bytes[port][qIndex]) / congestionIndicator;
		if (satLevel > 1) {
			satLevel = 1;
		}
		setCongested(port, qIndex, "egress", satLevel);
		if (sharedPool > sharedPoolUsed) {
			uint64_t remaining = sharedPool - sharedPoolUsed;
			double alphaP = 1;
			if (unsched) {
				alphaP = alphaHigh;
			}
			else {
				alphaP = alphaEgress[port][qIndex];
			}
			uint64_t Reverie_Threshold = alphaP * (remaining) * (1.0 / GetNofP("egress", qIndex)); //* egress_bytes[port][qIndex]/egressLpf_bytes[port][qIndex];
			return std::min(uint64_t(Reverie_Threshold), UINT64_MAX - 1024 * 1024);
		}
		else {
			// SharedPool is full. There is no `remaining` buffer.
			// The threshold returns zero in this case, but using if else just to avoid threshold computations even in the simple case.
			return 0;
		}
	}
}

uint64_t SwitchMmu::Threshold(uint32_t port, uint32_t qIndex, std::string inout, uint32_t type, uint32_t unsched) {
	uint64_t thresh = 0;
	if (inout == "ingress") {
		switch (ingressAlg[type]) {
		case DT:
			thresh = DynamicThreshold(port, qIndex, inout, type);
			break;
		case ABM:
			thresh = ActiveBufferManagement(port, qIndex, inout, type, unsched);
			break;
		case FAB:
			thresh = FlowAwareBuffer(port, qIndex, inout, type, unsched);
			break;
		default:
			thresh = DynamicThreshold(port, qIndex, inout, type);
			break;
		}
	}
	else if (inout == "egress") {
		switch (egressAlg[type]) {
		case DT:
			thresh = DynamicThreshold(port, qIndex, inout, type);
			break;
		case ABM:
			thresh = ActiveBufferManagement(port, qIndex, inout, type, unsched);
			break;
		case FAB:
			thresh = FlowAwareBuffer(port, qIndex, inout, type, unsched);
			break;
		default:
			thresh = DynamicThreshold(port, qIndex, inout, type);
			break;
		}
	}
	return thresh;
}

bool SwitchMmu::CheckIngressAdmission(uint32_t port, uint32_t qIndex, uint32_t psize, uint32_t type, uint32_t unsched) {
	std::string model = bufferModel;
	if (model == "reverie") {
		// if (!lpfUpdatedOnce){
		// 	UpdateLpfCounters();
		// }
		switch (type) {
		case LOSSY:
			return true;
			break;
		case LOSSLESS:
			// if reserved is used up
			if ( ( (psize + ingress_bytes[port][qIndex] > reserveIngress[port][qIndex])
			        // AND if per queue headroom is used up.
			        && (psize + GetHdrmBytes(port, qIndex) > xoff[port][qIndex]) && GetHdrmBytes(port, qIndex) > 0 )
			        // or if the headroom pool is full
			        || (psize + xoffTotalUsed > xoffTotal && GetHdrmBytes(port, qIndex) > 0 )
			        // if the ingresspool+headroom is full. With DT, this condition is redundant.
			        // This is just to account for any badly configured buffer or buffer sharing if any.
			        || (psize + totalUsed > ingressPool + xoffTotal)
			        // if the switch buffer is full
			        || (psize + totalUsed > bufferPool)  ) {

				std::cout << "reverie: dropping lossless packet at ingress admission headroom " << GetHdrmBytes(port, qIndex) << " xoff " << xoff[port][qIndex] << " pktSize " << psize << " xoffTotalUsed " << xoffTotalUsed  << " totalUsed " <<  totalUsed << " ingresspool " << ingressPool << " threshold " << ReverieThreshold(port, qIndex, LOSSLESS, unsched) << " ingress_bytes " << ingressLpf_bytes[port][qIndex] << std::endl;
				return false;
			}
			else {
				return true;
			}
			break;
		default:
			std::cout << "unknown type came in to CheckIngressAdmission function! This is not expected. Abort!" << std::endl;
			exit(1);
		}
	}
	else if (model == "sonic") {
		switch (type) {
		case LOSSY:
			// if ingress bytes is greater than the ingress threshold
			if ( (psize + ingress_bytes[port][qIndex] > Threshold(port, qIndex, "ingress", type , unsched)
			        // AND if the reserved is usedup
			        && psize + ingress_bytes[port][qIndex] > reserveIngress[port][qIndex])
			        // if the ingress pool is full. With DT, this condition is redundant.
			        // This is just to account for any badly configured buffer or buffer sharing if any.
			        || (psize + (totalUsed - xoffTotalUsed) > ingressPool)
			        // or if the switch buffer is full
			        || (psize + totalUsed > bufferPool) )
			{
				return false;
			}
			else {
				return true;
			}
			break;
		case LOSSLESS:
			// if reserved is used up
			if ( ( (psize + ingress_bytes[port][qIndex] > reserveIngress[port][qIndex])
			        // AND if per queue headroom is used up.
			        && (psize + GetHdrmBytes(port, qIndex) > xoff[port][qIndex]) && GetHdrmBytes(port, qIndex) > 0 )
			        // or if the headroom pool is full
			        || (psize + xoffTotalUsed > xoffTotal && GetHdrmBytes(port, qIndex) > 0 )
			        // if the ingresspool+headroom is full. With DT, this condition is redundant.
			        // This is just to account for any badly configured buffer or buffer sharing if any.
			        || (psize + totalUsed > ingressPool + xoffTotal)
			        // if the switch buffer is full
			        || (psize + totalUsed > bufferPool)  )
			{
				std::cout << "dropping lossless packet at ingress admission headroom " << GetHdrmBytes(port, qIndex) << " xoff " << xoff[port][qIndex] << " pktSize " << psize << " xoffTotalUsed " << xoffTotalUsed << " totalUsed " <<  totalUsed << std::endl;
				return false;
			}
			else {
				return true;
			}
			break;
		default:
			std::cout << "unknown type came in to CheckIngressAdmission function! This is not expected. Abort!" << std::endl;
			exit(1);
		}
	}
	else {
		std::cout << "unknown bufferModel encountered in CheckIngressAdmission function! This is not expected. Abort!" << std::endl;
		exit(1);
	}
}


bool SwitchMmu::CheckEgressAdmission(uint32_t port, uint32_t qIndex, uint32_t psize, uint32_t type, uint32_t unsched) {
	std::string model = bufferModel;
	if (model == "reverie") {
		switch (type) {
		case LOSSLESS:
			return true;
			break;
		case LOSSY:
			// if the egress queue length is greater than the threshold
			if ( (psize + egressLpf_bytes[port][qIndex] > ReverieThreshold(port, qIndex, LOSSY, unsched)
			        // AND if the reserved is usedup. THiS IS NOT SUPPORTED AT THE MOMENT. NO reserved at the egress.
			        // && psize + egress_bytes[port][qIndex] > reserveEgress[port][qIndex]
			     )
			        // or if the egress pool is full
			        || (psize + sharedPoolUsed > sharedPool)
			        // or if the switch buffer is full
			        || (psize + totalUsed > bufferPool) )

			{
				return false;
			}
			else {
				return true;
			}
			break;
		default:
			std::cout << "unknown type came in to CheckIngressAdmission function! This is not expected. Abort!" << std::endl;
			exit(1);
		}
	}
	else if (model == "sonic") {
		switch (type) {
		case LOSSY:
			// if the egress queue length is greater than the threshold
			if ( (psize + egress_bytes[port][qIndex] > Threshold(port, qIndex, "egress", type, unsched)
			        // AND if the reserved is usedup. THiS IS NOT SUPPORTED AT THE MOMENT. NO reserved at the egress.
			        // && psize + egress_bytes[port][qIndex] > reserveEgress[port][qIndex]
			     )
			        // or if the egress pool is full
			        || (psize + egressPoolUsed[type] > egressPool[type])
			        // or if the switch buffer is full
			        || (psize + totalUsed > bufferPool) )
			{
				return false;
			}
			else {
				return true;
			}
			break;
		case LOSSLESS:
			// if threshold is exceeded
			if ( ( (psize + egress_bytes[port][qIndex] > Threshold(port, qIndex, "egress", type, unsched))
			        // AND reserved is used up. THiS IS NOT SUPPORTED AT THE MOMENT. NO reserved at the egress.
			        // && (psize + egress_bytes[port][qIndex] > reserveEgress[port][qIndex])
			     )
			        // or if the corresponding egress pool is used up
			        || (psize + egressPoolUsed[type] > egressPool[type])
			        // or if the switch buffer is full
			        || (psize + totalUsed > bufferPool) )
			{
				std::cout << "dropping lossless packet at egress admission port " << port << " qIndex " << qIndex << " egress_bytes " << egress_bytes[port][qIndex] << " threshold " << Threshold(port, qIndex, "egress", type, unsched)
				          << std::endl;
				return false;
			}
			else {
				return true;
			}
			break;
		default:
			std::cout << "unknown type came in to CheckEgressAdmission function! This is not expected. Abort!" << std::endl;
			exit(1);
		}
	}
	else {
		std::cout << "unknown bufferModel encountered in CheckIngressAdmission function! This is not expected. Abort!" << std::endl;
		exit(1);
	}
	return true;
}

void SwitchMmu::UpdateIngressAdmission(uint32_t port, uint32_t qIndex, uint32_t psize, uint32_t type, uint32_t unsched) {

	std::string model = bufferModel;

	// if (Threshold(port, qIndex, "ingress", LOSSLESS, unsched) != ReverieThreshold(port, qIndex, LOSSLESS, unsched)){
	// 	std::cout << "FUCK" << std::endl;
	// }
	// If else are simply unnecessary but its a safety check to avoid magic scenarios (if a packet vanishes in the buffer) where we
	// might assign negative value to unsigned intergers.
	if (totalIngressReservedUsed >= GetIngressReservedUsed(port, qIndex)) // removing the old reserved used (will be updated next)
		totalIngressReservedUsed -= GetIngressReservedUsed(port, qIndex);
	else
		totalIngressReservedUsed = 0;
	// NOTE: ingress_bytes simple counts total bytes occupied by port, qIndex,
	// This includes bytes from ingresspool as well as from headroom and also reserved. ingress_bytes[port][qIndex] - xoffUsed[port][qIndex] gives us the occupancy in ingressPool.
	// ingress_bytes[port][qIndex] - xoffUsed[port][qIndex] - GetIngressReservedUsed(port,qIndex) gives us the occupancy in ingress shared pool.
	ingress_bytes[port][qIndex] += psize;
	totalUsed += psize; // IMPORTANT: totalUsed is only updated in the ingress. No need to update in egress. Avoid double counting.

	totalIngressReservedUsed += GetIngressReservedUsed(port, qIndex); // updating with the new reserved used.

	// Update the total headroom used.
	if (type == LOSSLESS) {
		sharedPoolUsed += psize;
		// uint64_t inst_ingress_shared_bytes = ingress_bytes[port][qIndex];//-xoffUsed[port][qIndex];
		// ingressLpf_bytes[port][qIndex] = Reveriegamma * ingressLpf_bytes[port][qIndex] + (1.0 - Reveriegamma) * (inst_ingress_shared_bytes);
		// if (ingress_bytes[port][qIndex] < ingressLpf_bytes[port][qIndex]) {
		// // if (1){
		// 	ingressLpf_bytes[port][qIndex] = ingress_bytes[port][qIndex];
		// }
		uint64_t threshold = 0;

		if (model=="sonic"){
			threshold = Threshold(port, qIndex, "ingress", LOSSLESS, unsched);
		}
		else if (model == "reverie"){
			threshold = ReverieThreshold(port, qIndex, LOSSLESS, unsched); // get the threshold
		}
		// First, remove the previously used headroom corresponding to queue: port, qIndex. This will be updated with current value next.
		xoffTotalUsed -= xoffUsed[port][qIndex];
		// Second, get currently used headroom by the queue: port, qIndex and update `xoffUsed[port][qIndex]`
		// if headroom is zero
		if (xoffUsed[port][qIndex] == 0) {
			// if ingress bytes of the queue exceeds threshold, start using headroom. pfc pause will be triggered by CheckShouldPause later.
			uint64_t temp = 0;
			if (model=="sonic"){
				temp = ingress_bytes[port][qIndex];
			}
			else if (model=="reverie"){
				temp = ingressLpf_bytes[port][qIndex];
			}
			if (temp > threshold) {
				// LOL: The commented part below was a HUGE mistake identified after debugging some of the lossless packets being dropped. It was a good lesson.
				// xoffUsed[port][qIndex] += ingress_bytes[port][qIndex] - threshold;
				xoffUsed[port][qIndex] += psize;
				sharedPoolUsed -= psize;
			}
		}
		// if we are already using headroom, any incoming packet must be added to headroom, UNTIL the queue drains and headroom becomes zero.
		else if (xoffUsed[port][qIndex] > 0) {
			xoffUsed[port][qIndex] += psize;
			sharedPoolUsed -= psize;
		}
		// Finally, update the total headroom used by adding (since we removed before) the latest value of xoffUsed (headroom used) by the queue
		xoffTotalUsed += xoffUsed[port][qIndex]; // add the current used headroom to total headroom
		// uint64_t inst_ingress_shared_bytes = ingress_bytes[port][qIndex]-xoffUsed[port][qIndex];
		// ingressLpf_bytes[port][qIndex] = Reveriegamma * ingressLpf_bytes [port][qIndex] + (1-Reveriegamma) * (inst_ingress_shared_bytes);
	}
}

void SwitchMmu::UpdateEgressAdmission(uint32_t port, uint32_t qIndex, uint32_t psize, uint32_t type) {
	egress_bytes[port][qIndex] += psize;
	egressPoolUsed[type] += psize;
	if (type == LOSSY) {
		sharedPoolUsed += psize;
		// egressLpf_bytes[port][qIndex] = Reveriegamma * egressLpf_bytes[port][qIndex] + (1-Reveriegamma) * (egress_bytes[port][qIndex]);
	}
}

void SwitchMmu::RemoveFromIngressAdmission(uint32_t port, uint32_t qIndex, uint32_t psize, uint32_t type) {

	txBytesIngress[port][qIndex] += psize; // We assume that the packet will not be dropped after this step for any other reason.

	// If else are simply unnecessary but its a safety check to avoid magic scenarios (if a packet vanishes in the buffer) where we
	// might assign negative value to unsigned intergers.

	if (totalIngressReservedUsed >= GetIngressReservedUsed(port, qIndex)) // removing the old reserved used (will be updated next)
		totalIngressReservedUsed -= GetIngressReservedUsed(port, qIndex);
	else
		totalIngressReservedUsed = 0;

	if (ingress_bytes[port][qIndex] >= psize)
		ingress_bytes[port][qIndex] -= psize;
	else
		ingress_bytes[port][qIndex] = 0;

	if (totalUsed >= psize) // IMPORTANT: totalUsed is only updated in the ingress. No need to update in egress. Avoid double counting.
		totalUsed -= psize;
	else
		totalUsed = 0;

	totalIngressReservedUsed += GetIngressReservedUsed(port, qIndex); // updating with the new reserved used.

	// Update the total headroom used.
	if (type == LOSSLESS) {
		uint64_t inst_ingress_shared_bytes = ingress_bytes[port][qIndex];//-xoffUsed[port][qIndex];
		ingressLpf_bytes[port][qIndex] = Reveriegamma * ingressLpf_bytes[port][qIndex] + (1.0 - Reveriegamma) * (inst_ingress_shared_bytes);
		if (ingress_bytes[port][qIndex] < ingressLpf_bytes[port][qIndex]) {
			ingressLpf_bytes[port][qIndex] = ingress_bytes[port][qIndex];
		}
		// First, remove the previously used headroom corresponding to queue: port, qIndex. This will be updated with current value next.
		if (xoffTotalUsed >= xoffUsed[port][qIndex])
			xoffTotalUsed -= xoffUsed[port][qIndex];
		else
			xoffTotalUsed = 0;
		// Second, check whether we are currently using any headroom. If not, nothing to do here: headroom is zero.
		if (xoffUsed[port][qIndex] > 0) {
			// Depending on the value of headroom used, the following cases arise:
			// 1. A packet can be removed entirely from the headroom
			// 2. Headroom occupancy is already less than the packet size.
			// So the dequeued packet decrements some part of headroom (emptying it) and some from ingress pool.
			if (xoffUsed[port][qIndex] >= psize) {
				xoffUsed[port][qIndex] -= psize;
			}
			else {
				sharedPoolUsed -= psize - xoffUsed[port][qIndex];
				xoffUsed[port][qIndex] = 0;
			}
		}
		else {
			if (sharedPoolUsed >= psize)
				sharedPoolUsed -= psize;
			else
				sharedPoolUsed = 0;
		}
		xoffTotalUsed += xoffUsed[port][qIndex]; // add the current used headroom to total headroom
	}
}

// void SwitchMmu::UpdateLpfCounters(){
// 	for (uint32_t port = 0; port < portCount; port++){
// 		for (uint32_t qIndex=0;qIndex<qCnt;qIndex++){
// 			uint64_t inst_ingress_shared_bytes = ingress_bytes[port][qIndex];//-xoffUsed[port][qIndex];
// 			ingressLpf_bytes[port][qIndex] = Reveriegamma * ingressLpf_bytes[port][qIndex] + (1-Reveriegamma) * (inst_ingress_shared_bytes);

// 			egressLpf_bytes[port][qIndex] = Reveriegamma * egressLpf_bytes[port][qIndex] + (1-Reveriegamma) * (egress_bytes[port][qIndex]);
// 		}
// 	}
// 	lpfUpdatedOnce = 1;
// 	double delay = 1e9*1500*8/bandwidth[0];

// 	Simulator::Schedule(NanoSeconds(delay),&SwitchMmu::UpdateLpfCounters,this);
// }

void SwitchMmu::RemoveFromEgressAdmission(uint32_t port, uint32_t qIndex, uint32_t psize, uint32_t type) {

	txBytesEgress[port][qIndex] += psize; // We assume that the packet will not be dropped after this step for any other reason.

	if (egress_bytes[port][qIndex] >= psize)
		egress_bytes[port][qIndex] -= psize;
	else
		egress_bytes[port][qIndex] = 0;

	if (egressPoolUsed[type] >= psize)
		egressPoolUsed[type] -= psize;
	else
		egressPoolUsed[type] = 0;

	if (type == LOSSY) {
		if (sharedPoolUsed >= psize)
			sharedPoolUsed -= psize;
		else
			sharedPoolUsed = 0;

		egressLpf_bytes[port][qIndex] = Reveriegamma * egressLpf_bytes[port][qIndex] + (1.0 - Reveriegamma) * (egress_bytes[port][qIndex]);
		if (egress_bytes[port][qIndex] < egressLpf_bytes[port][qIndex]) {
			egressLpf_bytes[port][qIndex] = egress_bytes[port][qIndex];
		}
	}
}



uint64_t SwitchMmu::GetHdrmBytes(uint32_t port, uint32_t qIndex) {

	return xoffUsed[port][qIndex];
}

bool SwitchMmu::CheckShouldPause(uint32_t port, uint32_t qIndex) {
	return !paused[port][qIndex] && (GetHdrmBytes(port, qIndex) > 0);
}

bool SwitchMmu::CheckShouldResume(uint32_t port, uint32_t qIndex) {
	std::string model = bufferModel;
	if (!paused[port][qIndex])
		return false;
	if (model == "sonic") {
		return GetHdrmBytes(port, qIndex) == 0 && (ingress_bytes[port][qIndex] < xon[port][qIndex] || ingress_bytes[port][qIndex] + xon_offset[port][qIndex] <= Threshold(port, qIndex, "ingress", LOSSLESS, 0) );
	}
	else if (model == "reverie") {
		return GetHdrmBytes(port, qIndex) == 0 && (ingressLpf_bytes[port][qIndex] < xon[port][qIndex] || ingressLpf_bytes[port][qIndex] + xon_offset[port][qIndex] <= ReverieThreshold(port, qIndex, LOSSLESS, 0) );
	}
	// Minor detail: Threshold(port, qIndex, "ingress", LOSSLESS, 0) is used above where type=LOSSLESS and unsched=0; It is obvious that resume is triggered only for LOSSLESS queues.
	// Abound unsched=0: sending resume must be independent of arriving traffic and hence the threshold used is the default value and a prioritized value cannot be used here as is done for admission of priority packets in ABM.
}

void SwitchMmu::SetPause(uint32_t port, uint32_t qIndex) {
	paused[port][qIndex] = true;
}
void SwitchMmu::SetResume(uint32_t port, uint32_t qIndex) {
	paused[port][qIndex] = false;
}

bool SwitchMmu::ShouldSendCN(uint32_t ifindex, uint32_t qIndex) {
	if (qIndex == 0)
		return false;
	if (egress_bytes[ifindex][qIndex] > kmax[ifindex])
		return true;
	if (egress_bytes[ifindex][qIndex] > kmin[ifindex]) {
		double p = pmax[ifindex] * double(egress_bytes[ifindex][qIndex] - kmin[ifindex]) / (kmax[ifindex] - kmin[ifindex]);
		if (UniformVariable(0, 1).GetValue() < p)
			return true;
	}
	return false;
}
void SwitchMmu::ConfigEcn(uint32_t port, uint32_t _kmin, uint32_t _kmax, double _pmax) {
	kmin[port] = _kmin * 1000;
	kmax[port] = _kmax * 1000;
	pmax[port] = _pmax;
}

}
