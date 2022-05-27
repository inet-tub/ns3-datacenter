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
The switch has an on-chip buffer which has `bufferPool` size. This buffer is shared across all port and queues in the switch.

`bufferPool` is further split into multiple pools at the ingress and egress.

It would be easier to understand from here on if you consider Ingress/Egress are merely just counters. These are not separate buffer locations or chips...!

First, `ingressPool` (size) accounts for ingress buffering shared by both lossy and lossless traffic.
Additionally, there exists a headroom pool of size xoffTotal, and each queue may use xoff[port][q] configurable amount at each port p and queue q.
When a queue at the ingress exceeds its ingress threshold, a PFC pause message is sent and any incoming packets can use upto a maximum of xoff[port][q] headroom.

Second, at the egress, `egressPool[LOSSY]` (size) accounts for buffering lossy traffic at the egress and similarly `egressPool[LOSSLESS]` for lossless traffic.
*/


SwitchMmu::SwitchMmu(void) {

	// Here we just initalize some default values. The buffer can be configured using Set functions through the simulation file later.

	// Buffer pools
	bufferPool = 24 * 1024 * 1024; // ASIC buffer size i.e, total shared buffer
	ingressPool = 18 * 1024 * 1024; // Size of ingress pool. Note: This is shared by both lossless and lossy traffic.
	egressPool[LOSSLESS] = 24 * 1024 * 1024; // Size of egress lossless pool. Lossless bypasses egress admission
	egressPool[LOSSY] = 14 * 1024 * 1024; // Size of egress lossy pool.
	egressPoolAll = 24 * 1024 * 1024; // Not for now. For later use.
	xoffTotal = 0; //6 * 1024 * 1024; // Total headroom space in the shared buffer pool. 
	// xoffTotal value is incremented when SetHeadroom function is used. So setting it to zero initially. 
	// Note: This would mean that headroom must be set explicity.


	// aggregate run time
	totalUsed = 0;  // IMPORTANT TO NOTE: THIS IS NOT bytes in the "ingress pool". This is the Total bytes in the ingress, which includes occupied buffer in reserved + headroom + ingresspool.
	egressPoolUsed[LOSSLESS] = 0; // Total bytes in the egress lossless pool
	egressPoolUsed[LOSSY] = 0; // Total bytes in the egress lossy pool
	totalSharedUsed = 0; // Total bytes in the shared buffer.
	xoffTotalUsed = 0; // Total headroom bytes used so far. Updated at runtime.
	// It is sometimes useful to keep track of total bytes used specifically from ingressPool. We don't need an additional variable. 
	// This is equal to (totalUsed - xoffTotalUsed).

	for (uint32_t port = 0; port < pCnt; port++) {
		for (uint32_t q = 0; q < qCnt; q++) {
			// buffer configuration.
			reserveIngress[port][q] = 1248; // Per queue reserved buffer at ingress
			reserveEgress[port][q] = 1248; // per queue reserved buffer at egress
			alphaEgress[port][q] = 1; // per queue alpha value used by BM at egress/
			alphaIngress[port][q] = 1; // per queue alpha value used by BM at ingress
			xoff[port][q] = 96928; // per queue headroom used at ingress. This can be changed using SetHeadroom
			xon[port][q] = 1248; // This is used for pfc resume. Can be changed using SetXon
			xon_offset[port][q] = 2496; // This is also used for pfc resume. Can be changed using SetXonOffset/


			// per queue run time
			ingress_bytes[port][q] = 0; // total ingress bytes at each queue. This includes, bytes from reserved, ingress pool as well as any headroom.
			paused[port][q] = 0; // This is a state to keep track of which queue is currently under pause, used at egress as check whether to send out packets.
			egress_bytes[port][q] = 0; // Per queue egress bytes at each queue
			xoffUsed[port][q] = 0; // The headroom buffer used by each queue. xoffUsed[port][q] = ingress_bytes[port][q] - its Threshold
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


uint64_t SwitchMmu::DynamicThreshold(uint32_t port, uint32_t qIndex, std::string inout, uint32_t type) {
	if (inout == "ingress") {
		double remaining = 0;
		uint64_t ingressPoolUsed = totalUsed - xoffTotalUsed; // Total bytes used from the ingress pool specifically.
		if (ingressPool > ingressPoolUsed) {
			uint64_t remaining = ingressPool - ingressPoolUsed;
			// DT's threshold = Alpha x remaining.
			return std::min(uint64_t(alphaIngress[port][qIndex] * (remaining)), UINT64_MAX - 1024 * 1024);
		}
		else { // ingressPool is full. There is no `remaining` buffer in ingressPool. DT's threshold returns zero in this case, but using if else just to avoid threshold computations even in the simple case.
			return 0;
		}
	}
	else if (inout == "egress") {
		double remaining = 0;
		if (egressPool[type] > egressPoolUsed[type]) {
			uint64_t remaining = egressPool[type] - egressPoolUsed[type];
			return std::min(uint64_t(alphaEgress[port][qIndex] * (remaining)), UINT64_MAX - 1024 * 1024); // UINT64_MAX - 1024*1024 is just a randomly chosen big value. Just don't want to return UINT64_MAX value, sometimes causes overflow issues later.
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
		if ( (psize + ingress_bytes[port][qIndex] > Threshold(port, qIndex, "ingress", type)
		        && psize + ingress_bytes[port][qIndex] > reserveIngress[port][qIndex]) // if ingress bytes is greater than the ingress threshold AND if the reserved is usedup
		        || (psize + (totalUsed-xoffTotalUsed) > ingressPool) // if the ingress pool is full. With DT, this condition is redundant. This is just to account for any badly configured buffer or buffer sharing if any.
		        || (psize + totalSharedUsed > bufferPool) )  // or if the shared buffer is full
		{
			return false;
		}
		else {
			return true;
		}
		break;
	case LOSSLESS:
		if ( ( (psize + ingress_bytes[port][qIndex] > reserveIngress[port][qIndex]) // if reserved is used up
		        && (psize + GetHdrmBytes(port,q) > xoff[port][qIndex]) && GetHdrmBytes(port,q)>0 ) // and if per queue headroom is used up.
		        || (xoffTotalUsed >= xoffTotal ) // or if the headroom pool is full
		        || (psize + totalUsed > ingressPool + xoffTotal) // if the ingresspool+headroom is full. With DT, this condition is redundant. This is just to account for any badly configured buffer or buffer sharing if any.
		        || (psize + totalSharedUsed > bufferPool)  ) // if the shared buffer is full
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
		if ( (psize + egress_bytes[port][qIndex] > Threshold(port, qIndex, "egress", type) // if the egress queue length is greater than the threshold
		        && psize + egress_bytes[port][qIndex] > reserveEgress[port][qIndex]) // AND if the reserved is usedup
		        || (psize + egressPoolUsed[LOSSY] > egressPool[LOSSY]) // or if the egress pool is full
		        || (psize + totalSharedUsed > bufferPool) ) // or if the shared buffer is full
		{
			return false;
		}
		else {
			return true;
		}
		break;
	case LOSSLESS:
		if ( ( (psize + egress_bytes[port][qIndex] > Threshold(port, qIndex, "egress", type)) 
					&& (psize + egress_bytes[port][qIndex] > reserveEgress[port][qIndex]) )// if threshold is exceeded AND reserved is used up
		        || (psize + egressPoolUsed[LOSSLESS] > egressPool[LOSSLESS]) // if the corresponding egress pool is used up
		        || (psize + totalSharedUsed > bufferPool) ) // if the buffer pool is full
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
	totalUsed += psize;
	totalSharedUsed += psize; // IMPORTANT: totalSharedUsed is only updated in the ingress. No need to update in egress. Avoid double counting.

	/* Update the total headroom used.*/
	xoffTotalUsed -= xoffUsed[port][qIndex]; // First remove the previously used headroom
	uint64_t threshold = Threshold(port,qIndex,"ingress",type); // get the threshold
	if (xoffUsed[port][qIndex]==0){ // if headroom is zero
		if (ingress_bytes[port][qIndex] > threshold){// if ingress bytes of the queue exceeds threshold, start using headroom. pfc pause will be triggered by CheckShouldPause later.
			xoffUsed[port][qIndex] += ingress_bytes[port][qIndex] - threshold;
		}
	}
	else{ // if we are already using headroom, any incoming packet must be added to headroom, UNTIL the queue drains and headroom becomes zero.
		xoffUsed[port][qIndex] += psize;
	}
	xoffTotalUsed += xoffUsed[port][qIndex]; // add the current used headroom to total headroom
}

void SwitchMmu::UpdateEgressAdmission(uint32_t port, uint32_t qIndex, uint32_t psize, uint32_t type) {
	egress_bytes[port][qIndex] += psize;
	egressPoolUsed[type] += psize;
}

void SwitchMmu::RemoveFromIngressAdmission(uint32_t port, uint32_t qIndex, uint32_t psize, uint32_t type) {
	ingress_bytes[port][qIndex] -= psize;
	totalUsed -= psize;
	totalSharedUsed -= psize;

	/* Update the total headroom used.*/
	xoffTotalUsed -= xoffUsed[port][qIndex]; // First remove the previously used headroom
	if (xoffUsed[port][qIndex]>0){
		if (xoffUsed[port][qIndex]>=psize){
			xoffUsed[port][qIndex]-=psize;
		}
		else{
			xoffUsed[port][qIndex]=0;
		}
	}
	xoffTotalUsed += xoffUsed[port][qIndex]; // add the current used headroom to total headroom
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
