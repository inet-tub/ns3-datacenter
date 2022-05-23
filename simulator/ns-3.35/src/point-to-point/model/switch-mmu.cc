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

SwitchMmu::SwitchMmu(void) {

	// Buffer pools
	bufferPool = 24 * 1024 * 1024; // ASIC buffer size i.e, total shared buffer
	ingressPool = 18 * 1024 * 1024; // ingress lossless pool
	egressPool[LOSSLESS] = 24 * 1024 * 1024; // egress lossless pool. Lossless bypasses egress admission
	egressPool[LOSSY] = 14 * 1024 * 1024; // egress lossy pool.
	egressPoolAll = 24 * 1024 * 1024;
	xoffTotal = 6 * 1024 * 1024;


	for (uint32_t port = 0; port < pCnt; port++) {
		for (uint32_t q = 0; q < qCnt; q++) {
			// buffer configuration.
			reserveIngress[port][q] = 1248;
			reserveEgress[port][q] = 1248;
			alphaEgress[port][q] = 1;
			alphaIngress[port][q] = 1;
			xoff[port][q] = 96928;
			xon[port][q] = 1248;
			xon_offset[port][q] = 2496;

			// aggregate run time
			ingressPoolUsed = 0;
			egressPoolUsed[LOSSLESS] = 0;
			egressPoolUsed[LOSSY] = 0;
			totalSharedUsed = 0;

			// per queue run time
			ingress_bytes[port][q] = 0;
			hdrm_bytes[port][q] = 0;
			paused[port][q] = 0;
			egress_bytes[port][q] = 0;
		}
	}

	ingressAlg[LOSSLESS] = DT;
	ingressAlg[LOSSY] = DT;
	egressAlg[LOSSLESS] = DT;
	egressAlg[LOSSY] = DT;


	memset(hdrm_bytes, 0, sizeof(hdrm_bytes));
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
	if (inout == "ingress"){
		reserveIngress[port][q] = b;
	}
	else if (inout == "egress"){
		reserveEgress[port][q] = b;
	}
}

void
SwitchMmu::SetReserved(uint64_t b, std::string inout) {
	if (inout == "ingress"){
		for (uint32_t port = 0; port<pCnt; port++){
			for(uint32_t q=0; q < qCnt ; q++){
				reserveIngress[port][q] = b;
			}
		}
	}
	else if (inout == "egress"){
		for (uint32_t port = 0; port< pCnt; port++){
			for (uint32_t q = 0; q < qCnt; q++){
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
	for (uint32_t port = 0; port< pCnt; port++){
		for (uint32_t q = 0; q < qCnt; q++){
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
	for (uint32_t port = 0; port< pCnt; port++){
		for (uint32_t q = 0; q < qCnt; q++){
			alphaEgress[port][q] = value;
		}
	}
}

void
SwitchMmu::SetHeadroom(uint64_t b, uint32_t port, uint32_t q) {
	xoff[port][q] = b;
}

void
SwitchMmu::SetHeadroom(uint64_t b) {
	for (uint32_t port = 0; port< pCnt; port++){
		for (uint32_t q = 0; q < qCnt; q++){
			xoff[port][q] = b;
		}
	}
}

void
SwitchMmu::SetXon(uint64_t b, uint32_t port, uint32_t q) {
	xon[port][q] = b;
}
void
SwitchMmu::SetXon(uint64_t b) {
	for (uint32_t port = 0; port< pCnt; port++){
		for (uint32_t q = 0; q < qCnt; q++){
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
	for (uint32_t port = 0; port< pCnt; port++){
		for (uint32_t q = 0; q < qCnt; q++){
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
		if (ingressPool > ingressPoolUsed) {
			uint64_t remaining = ingressPool - ingressPoolUsed;
			return std::min(uint64_t(alphaIngress[port][qIndex] * (remaining)), UINT64_MAX - 1024 * 1024);
		}
		else {
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
		if ( (psize + ingress_bytes[port][qIndex] > Threshold(port, qIndex, "ingress", type) && psize + ingress_bytes[port][qIndex] > reserveIngress[port][qIndex])
		        // || (psize + ingressPoolUsed > ingressPool )
		        || (psize + totalSharedUsed > bufferPool) ) {
			return false;
		}
		else {
			return true;
		}
		break;
	case LOSSLESS:
		if ( ( (psize + ingress_bytes[port][qIndex] > reserveIngress[port][qIndex])
		        && (psize + ingress_bytes[port][qIndex] > Threshold(port, qIndex, "ingress", type) + xoff[port][qIndex]) )
		        || (psize + ingressPoolUsed > ingressPool + xoffTotal)
		        || (psize + totalSharedUsed > bufferPool)  ) {
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
		if ( (psize + egress_bytes[port][qIndex] > Threshold(port, qIndex, "egress", type) && psize + egress_bytes[port][qIndex] > reserveEgress[port][qIndex])
		        || (psize + egressPoolUsed[LOSSY] > egressPool[LOSSY])
		        || (psize + totalSharedUsed > bufferPool) ) {
			return false;
		}
		else {
			return true;
		}
		break;
	case LOSSLESS:
		if ( (psize + egress_bytes[port][qIndex] > Threshold(port, qIndex, "egress", type) && psize + egress_bytes[port][qIndex] > reserveEgress[port][qIndex])
		        || (psize + egressPoolUsed[LOSSLESS] > egressPool[LOSSLESS])
		        || (psize + totalSharedUsed > bufferPool) ) {
			std::cout << "dropping lossless packet at egress admission port " << port << " qIndex " << qIndex << " egress_bytes " << egress_bytes[port][qIndex] << " threshold " << Threshold(port,qIndex,"egress",type)
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
	ingress_bytes[port][qIndex] += psize;
	ingressPoolUsed += psize;
	totalSharedUsed += psize; // IMPORTANT: totalSharedUsed is only updated in the ingress. No need to update in egress. Avoid double counting.
}

void SwitchMmu::UpdateEgressAdmission(uint32_t port, uint32_t qIndex, uint32_t psize, uint32_t type) {
	egress_bytes[port][qIndex] += psize;
	egressPoolUsed[type] += psize;
}

void SwitchMmu::RemoveFromIngressAdmission(uint32_t port, uint32_t qIndex, uint32_t psize, uint32_t type) {
	ingress_bytes[port][qIndex] -= psize;
	ingressPoolUsed -= psize;
	totalSharedUsed -= psize;
}
void SwitchMmu::RemoveFromEgressAdmission(uint32_t port, uint32_t qIndex, uint32_t psize, uint32_t type) {
	egress_bytes[port][qIndex] -= psize;
	egressPoolUsed[type] -= psize;
}


uint64_t SwitchMmu::GetHdrmBytes(uint32_t port, uint32_t qIndex) {
	uint64_t thresh = Threshold(port, qIndex, "ingress", LOSSLESS);
	uint64_t hdrm;
	if (thresh > ingress_bytes[port][qIndex]) {
		hdrm = 0;
	}
	else {
		hdrm = ingress_bytes[port][qIndex] - thresh;
	}
	return hdrm;
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
