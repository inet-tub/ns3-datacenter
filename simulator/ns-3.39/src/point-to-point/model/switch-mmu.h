#ifndef SWITCH_MMU_H
#define SWITCH_MMU_H

#include <unordered_map>
#include <ns3/node.h>

namespace ns3 {

class Packet;

class SwitchMmu: public Object {
public:
	static const uint32_t pCnt = 257;	// Number of ports used
	static const uint32_t qCnt = 8;	// Number of queues/priorities used

	static TypeId GetTypeId (void);

	SwitchMmu(void);

	bool CheckIngressAdmission(uint32_t port, uint32_t qIndex, uint32_t psize, uint32_t type, uint32_t unsched);
	bool CheckEgressAdmission(uint32_t port, uint32_t qIndex, uint32_t psize, uint32_t type, uint32_t unsched);
	void UpdateIngressAdmission(uint32_t port, uint32_t qIndex, uint32_t psize, uint32_t type, uint32_t unsched);
	void UpdateEgressAdmission(uint32_t port, uint32_t qIndex, uint32_t psize, uint32_t type);
	void RemoveFromIngressAdmission(uint32_t port, uint32_t qIndex, uint32_t psize, uint32_t type);
	void RemoveFromEgressAdmission(uint32_t port, uint32_t qIndex, uint32_t psize, uint32_t type);

	bool CheckShouldPause(uint32_t port, uint32_t qIndex);
	bool CheckShouldResume(uint32_t port, uint32_t qIndex);
	void SetPause(uint32_t port, uint32_t qIndex);
	void SetResume(uint32_t port, uint32_t qIndex);

	bool ShouldSendCN(uint32_t ifindex, uint32_t qIndex);

	void ConfigEcn(uint32_t port, uint32_t _kmin, uint32_t _kmax, double _pmax);

	void SetBufferModel(std::string model){bufferModel = model;}

	void SetBufferPool(uint64_t b);

	void SetIngressPool(uint64_t b);

	void SetSharedPool(uint64_t b);

	void SetEgressPoolAll(uint64_t b);

	void SetEgressLossyPool(uint64_t b);

	void SetEgressLosslessPool(uint64_t b);

	void SetReserved(uint64_t b, uint32_t port, uint32_t q, std::string inout);
	void SetReserved(uint64_t b, std::string inout);

	void SetAlphaIngress(double value, uint32_t port, uint32_t q);
	void SetAlphaIngress(double value);

	void SetAlphaEgress(double value, uint32_t port, uint32_t q);
	void SetAlphaEgress(double value);

	void SetHeadroom(uint64_t b, uint32_t port, uint32_t q);
	void SetHeadroom(uint64_t b);

	void SetXon(uint64_t b, uint32_t port, uint32_t q);
	void SetXon(uint64_t b);

	void SetXonOffset(uint64_t b, uint32_t port, uint32_t q);
	void SetXonOffset(uint64_t b);

	void SetIngressLossyAlg(uint32_t alg);

	void SetIngressLosslessAlg(uint32_t alg);

	void SetEgressLossyAlg(uint32_t alg);

	void SetEgressLosslessAlg(uint32_t alg);

	void SetGamma(double value);

	uint64_t Threshold(uint32_t port, uint32_t qIndex, std::string inout, uint32_t type, uint32_t alphaPrio);

	uint64_t DynamicThreshold(uint32_t port, uint32_t qIndex, std::string inout, uint32_t type);

	uint64_t GetHdrmBytes(uint32_t port, uint32_t qIndex);

	uint64_t GetIngressReservedUsed();

	uint64_t GetIngressReservedUsed(uint32_t port, uint32_t qIndex);

	uint64_t GetIngressSharedUsed();

	void setCongested(uint32_t portId, uint32_t qIndex, std::string inout, double satLevel);

	double GetNofP(std::string inout, uint32_t qIndex);

	double getDequeueRate(uint32_t port, uint32_t qIndex, std::string inout);

	void updateDequeueRates();

	uint64_t ActiveBufferManagement(uint32_t port, uint32_t qIndex, std::string inout, uint32_t type, uint32_t unsched);

	uint64_t FlowAwareBuffer(uint32_t port, uint32_t qIndex, std::string inout, uint32_t type, uint32_t unsched);

	void SetABMalphaHigh(double alpha){alphaHigh = alpha;};

	void SetABMdequeueUpdateNS(double time){updateIntervalNS = time;}

	void SetPortCount(uint32_t pc){portCount = pc;}

	uint64_t ReverieThreshold(uint32_t port, uint32_t qIndex, uint32_t type, uint32_t unsched);

	void UpdateLpfCounters();


	// config
	uint32_t node_id;
	uint32_t kmin[pCnt], kmax[pCnt];
	double pmax[pCnt];

	// Buffer model
	std::string bufferModel;

	// Buffer pools
	uint64_t bufferPool;
	uint64_t ingressPool ;
	uint64_t egressPool[2];
	uint64_t egressPoolAll;
	uint64_t sharedPool;
	uint64_t xoffTotal;
	uint64_t totalIngressReserved;

	// aggregate run time
	uint64_t totalUsed;
	uint64_t egressPoolUsed[2];
	uint64_t xoffTotalUsed;
	uint64_t totalIngressReservedUsed;
	uint64_t sharedPoolUsed;


	// buffer configuration.
	uint64_t reserveIngress[pCnt][qCnt];
	uint64_t reserveEgress[pCnt][qCnt];
	double 	 alphaEgress[pCnt][qCnt];
	double 	 alphaIngress[pCnt][qCnt];
	uint64_t xoff[pCnt][qCnt];
	uint64_t xon[pCnt][qCnt];
	uint64_t xon_offset[pCnt][qCnt];

	// per queue run time
	uint64_t ingress_bytes[pCnt][qCnt];
	uint64_t hdrm_bytes[pCnt][qCnt];
	uint32_t paused[pCnt][qCnt];
	uint64_t egress_bytes[pCnt][qCnt];
	uint64_t xoffUsed[pCnt][qCnt];
	uint64_t ingressLpf_bytes[pCnt][qCnt];
	uint64_t egressLpf_bytes[pCnt][qCnt];

	// Buffer Sharing algorithm
	uint32_t ingressAlg[2];
	uint32_t egressAlg[2];

	// ABM realted variables
	double NofPIngress[qCnt];
	double NofPEgress[qCnt];
	double congestedIngress[pCnt][qCnt];
	double congestedEgress[pCnt][qCnt];
	double dequeueRateIngress[pCnt][qCnt];
	double dequeueRateEgress[pCnt][qCnt];
	uint64_t txBytesIngress[pCnt][qCnt];
	uint64_t txBytesEgress[pCnt][qCnt];
	uint64_t bandwidth[pCnt];
	uint32_t congestionIndicator;
	double alphaHigh;
	double updateIntervalNS;
	uint32_t dequeueUpdatedOnce;
	uint32_t portCount;

	double Reveriegamma;
	uint32_t lpfUpdatedOnce;

};

} /* namespace ns3 */

#endif /* SWITCH_MMU_H */

