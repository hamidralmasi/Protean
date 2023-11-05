
#ifndef SRC_TRAFFIC_CONTROL_MODEL_SHARED_MEMORY_H_BKP_
#define SRC_TRAFFIC_CONTROL_MODEL_SHARED_MEMORY_H_BKP_

#include "ns3/simulator.h"
#include "ns3/object.h"
//#include "ns3/queue-disc.h"

#include "unordered_map"

namespace ns3 {

class SharedMemoryBuffer : public Object{
public:
	static TypeId GetTypeId (void);
	SharedMemoryBuffer();
	virtual ~SharedMemoryBuffer();

	void SetSharedBufferSize(uint32_t size);
	uint32_t GetSharedBufferSize();

	uint32_t GetOccupiedBuffer(){return OccupiedBuffer;}
	uint32_t GetRemainingBuffer(){return RemainingBuffer;}
	double GetNofP(uint32_t priority);
	bool EnqueueBuffer(uint32_t size);
	void DequeueBuffer(uint32_t size);

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

	uint32_t GetPerPriorityOccupied(uint32_t priority){return OccupiedBufferPriority[priority];}

	void incrementDroppedPacketCount (uint32_t port, uint32_t queue);
	u_int32_t GetDroppedPacketCount (uint32_t port, uint32_t queue);
	u_int32_t GetDroppedPacketCount (uint32_t port);
	u_int32_t GetTotalDroppedPacketCount (void);
	void SetDqDt (uint32_t port, uint32_t queue, double dqdt_curr);
	double GetDqDt (uint32_t port, uint32_t queue);
	void SetMaxDqDt (double dqdt_max);
	double GetMaxDqDt ();

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
	uint32_t maxports=100;
	uint32_t maxpriority=8;

	std::unordered_map<uint32_t,uint32_t> PriorityToGroupMap;

	std::vector<std::pair<uint32_t,Time>> Deq[100][8];
	double sumBytes[100][8];
	Time tDiff[100][8];
	uint64_t MaxRate;
	Time timestamp[100][8];

	uint32_t num_dropped_packets[100][8];
	double dqdt[100][8];
	double max_dqdt;

};
}



#endif /* SRC_TRAFFIC_CONTROL_MODEL_SHARED_MEMORY_H_BKP_ */
