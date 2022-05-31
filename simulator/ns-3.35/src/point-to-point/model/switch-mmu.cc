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


#define SONICBUFFER 152192
#define NEWBUFFER 65571

#define LOSSLESS 0
#define LOSSY 1
#define DUMMY 2

# define DT 101
# define FAB 102
# define CS 103
# define IB 104
# define ABM 110

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

	// Buffer pools
	bufferPool = 24 * 1024 * 1024; // ASIC buffer size i.e, total shared buffer
	ingressPool = 18 * 1024 * 1024; // Size of ingress pool. Note: This is shared by both lossless and lossy traffic.
	egressPool[LOSSLESS] = 24 * 1024 * 1024; // Size of egress lossless pool. Lossless bypasses egress admission
	egressPool[LOSSY] = 14 * 1024 * 1024; // Size of egress lossy pool.
	egressPoolAll = 24 * 1024 * 1024; // Not for now. For later use.
	xoffTotal = 0; //6 * 1024 * 1024; // Total headroom space in the shared buffer pool.
	// xoffTotal value is incremented when SetHeadroom function is used. So setting it to zero initially.
	// Note: This would mean that headroom must be set explicitly.


	// aggregate run time
	// `totalUsed` IMPORTANT TO NOTE: THIS IS NOT bytes in the "ingress pool".
	// This is the total bytes USED in the switch buffer, which includes occupied buffer in reserved + headroom + ingresspool.
	totalUsed = 0;
	egressPoolUsed[LOSSLESS] = 0; // Total bytes USED in the egress lossless pool
	egressPoolUsed[LOSSY] = 0; // Total bytes USED in the egress lossy pool
	xoffTotalUsed = 0; // Total headroom bytes USED so far. Updated at runtime.
	// It is sometimes useful to keep track of total bytes used specifically from ingressPool. We don't need an additional variable.
	// This is equal to (totalUsed - xoffTotalUsed).

	for (uint32_t port = 0; port < pCnt; port++) {
		for (uint32_t q = 0; q < qCnt; q++) {
			// buffer configuration.
			reserveIngress[port][q] = 1248; // Per queue reserved buffer at ingress
			reserveEgress[port][q] = 1248; // per queue reserved buffer at egress
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
		}
	}

	ingressAlg[LOSSLESS] = DT;
	ingressAlg[LOSSY] = DT;
	egressAlg[LOSSLESS] = DT;
	egressAlg[LOSSY] = DT;


	memset(ingress_bytes, 0, sizeof(ingress_bytes));
	memset(paused, 0, sizeof(paused));
	memset(egress_bytes, 0, sizeof(egress_bytes));
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
		reserveIngress[port][q] = b;
	}
	else if (inout == "egress") {
		reserveEgress[port][q] = b;
	}
}

void
SwitchMmu::SetReserved(uint64_t b, std::string inout) {
	if (inout == "ingress") {
		for (uint32_t port = 0; port < pCnt; port++) {
			for (uint32_t q = 0; q < qCnt ; q++) {
				reserveIngress[port][q] = b;
			}
		}
	}
	else if (inout == "egress") {
		for (uint32_t port = 0; port < pCnt; port++) {
			for (uint32_t q = 0; q < qCnt; q++) {
				reserveEgress[port][q] = b;
			}
		}
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


// DT's threshold = Alpha x remaining.
// A sky high threshold for a queue can be emulated by setting the corresponding alpha to a large value. eg., UINT32_MAX
uint64_t SwitchMmu::DynamicThreshold(uint32_t port, uint32_t qIndex, std::string inout, uint32_t type) {
	if (inout == "ingress") {
		double remaining = 0;
		uint64_t ingressPoolUsed = totalUsed - xoffTotalUsed; // Total bytes used from the ingress pool specifically.
		if (ingressPool > ingressPoolUsed) {
			uint64_t remaining = ingressPool - ingressPoolUsed;
			return std::min(uint64_t(alphaIngress[port][qIndex] * (remaining)), UINT64_MAX - 1024 * 1024);
		}
		else {
			// ingressPool is full. There is no `remaining` buffer in ingressPool.
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
			return std::min(uint64_t(alphaEgress[port][qIndex] * (remaining)), UINT64_MAX - 1024 * 1024);
		}
		else {
			return 0;
		}
	}
}

uint64_t SwitchMmu::Threshold(uint32_t port, uint32_t qIndex, std::string inout, uint32_t type) {
	uint64_t thresh = 0;
	if (inout == "ingress") {
		switch (ingressAlg[type]) {
		case DT:
			thresh = DynamicThreshold(port, qIndex, inout, type);
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
		default:
			thresh = DynamicThreshold(port, qIndex, inout, type);
			break;
		}
	}
	return thresh;
}


bool SwitchMmu::CheckIngressAdmission(uint32_t port, uint32_t qIndex, uint32_t psize, uint32_t type) {

	switch (type) {
	case LOSSY:
		// if ingress bytes is greater than the ingress threshold
		if ( (psize + ingress_bytes[port][qIndex] > Threshold(port, qIndex, "ingress", type)
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
		        || (psize + xoffTotalUsed >= xoffTotal && GetHdrmBytes(port, qIndex) > 0 )
		        // if the ingresspool+headroom is full. With DT, this condition is redundant.
		        // This is just to account for any badly configured buffer or buffer sharing if any.
		        || (psize + totalUsed > ingressPool + xoffTotal)
		        // if the switch buffer is full
		        || (psize + totalUsed > bufferPool)  )
		{
			std::cout << "dropping lossless packet at ingress admission" << std::endl;
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


bool SwitchMmu::CheckEgressAdmission(uint32_t port, uint32_t qIndex, uint32_t psize, uint32_t type) {

	switch (type) {
	case LOSSY:
		// if the egress queue length is greater than the threshold
		if ( (psize + egress_bytes[port][qIndex] > Threshold(port, qIndex, "egress", type)
		        // AND if the reserved is usedup
		        && psize + egress_bytes[port][qIndex] > reserveEgress[port][qIndex])
		        // or if the egress pool is full
		        || (psize + egressPoolUsed[LOSSY] > egressPool[LOSSY])
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
		if ( ( (psize + egress_bytes[port][qIndex] > Threshold(port, qIndex, "egress", type))
		        // AND reserved is used up
		        && (psize + egress_bytes[port][qIndex] > reserveEgress[port][qIndex]) )
		        // or if the corresponding egress pool is used up
		        || (psize + egressPoolUsed[LOSSLESS] > egressPool[LOSSLESS])
		        // or if the switch buffer is full
		        || (psize + totalUsed > bufferPool) )
		{
			std::cout << "dropping lossless packet at egress admission port " << port << " qIndex " << qIndex << " egress_bytes " << egress_bytes[port][qIndex] << " threshold " << Threshold(port, qIndex, "egress", type)
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

	return true;
}

void SwitchMmu::UpdateIngressAdmission(uint32_t port, uint32_t qIndex, uint32_t psize, uint32_t type) {

	// NOTE: ingress_bytes simple counts total bytes occupied by port, qIndex,
	// This includes bytes from ingresspool as well as from headroom. ingress_bytes[port][qIndex] - xoffUsed[port][qIndex] gives us the occupancy in ingressPool.
	ingress_bytes[port][qIndex] += psize;
	totalUsed += psize; // IMPORTANT: totalUsed is only updated in the ingress. No need to update in egress. Avoid double counting.

	// Update the total headroom used.
	if (type == LOSSLESS) {
		// First, remove the previously used headroom corresponding to queue: port, qIndex. This will be updated with current value next.
		xoffTotalUsed -= xoffUsed[port][qIndex];
		// Second, get currently used headroom by the queue: port, qIndex and update `xoffUsed[port][qIndex]`
		uint64_t threshold = Threshold(port, qIndex, "ingress", type); // get the threshold
		// if headroom is zero
		if (xoffUsed[port][qIndex] == 0) {
			// if ingress bytes of the queue exceeds threshold, start using headroom. pfc pause will be triggered by CheckShouldPause later.
			if (ingress_bytes[port][qIndex] > threshold) {
				xoffUsed[port][qIndex] += ingress_bytes[port][qIndex] - threshold;
			}
		}
		// if we are already using headroom, any incoming packet must be added to headroom, UNTIL the queue drains and headroom becomes zero.
		else {
			xoffUsed[port][qIndex] += psize;
		}
		// Finally, update the total headroom used by adding (since we removed before) the latest value of xoffUsed (headroom used) by the queue
		xoffTotalUsed += xoffUsed[port][qIndex]; // add the current used headroom to total headroom
	}
}

void SwitchMmu::UpdateEgressAdmission(uint32_t port, uint32_t qIndex, uint32_t psize, uint32_t type) {
	egress_bytes[port][qIndex] += psize;
	egressPoolUsed[type] += psize;
}

void SwitchMmu::RemoveFromIngressAdmission(uint32_t port, uint32_t qIndex, uint32_t psize, uint32_t type) {
	ingress_bytes[port][qIndex] -= psize;
	totalUsed -= psize;

	// Update the total headroom used.
	if (type == LOSSLESS) {
		// First, remove the previously used headroom corresponding to queue: port, qIndex. This will be updated with current value next.
		xoffTotalUsed -= xoffUsed[port][qIndex];
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
				xoffUsed[port][qIndex] = 0;
			}
		}
		xoffTotalUsed += xoffUsed[port][qIndex]; // add the current used headroom to total headroom
	}
}
void SwitchMmu::RemoveFromEgressAdmission(uint32_t port, uint32_t qIndex, uint32_t psize, uint32_t type) {
	egress_bytes[port][qIndex] -= psize;
	egressPoolUsed[type] -= psize;
}



uint64_t SwitchMmu::GetHdrmBytes(uint32_t port, uint32_t qIndex) {

	return xoffUsed[port][qIndex];
}

bool SwitchMmu::CheckShouldPause(uint32_t port, uint32_t qIndex) {
	return !paused[port][qIndex] && (GetHdrmBytes(port, qIndex) > 0);
}

bool SwitchMmu::CheckShouldResume(uint32_t port, uint32_t qIndex) {
	if (!paused[port][qIndex])
		return false;
	return GetHdrmBytes(port, qIndex) == 0 && (ingress_bytes[port][qIndex] < xon[port][qIndex] || ingress_bytes[port][qIndex] + xon_offset[port][qIndex] <= Threshold(port, qIndex, "ingress", LOSSLESS) );
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
