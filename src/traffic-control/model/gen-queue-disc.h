

#ifndef GEN_QUEUE_DISC_H
#define GEN_QUEUE_DISC_H

#include "ns3/queue-disc.h"
#include <array>

#include "unordered_map"
#include "ns3/simulator.h"
#include "shared-memory.h"


namespace ns3 {

class GenQueueDisc : public QueueDisc {
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  /**
   * \brief GenQueueDisc constructor
   */
  GenQueueDisc ();

  virtual ~GenQueueDisc();

  // Reasons for dropping packets
  static constexpr const char* LIMIT_EXCEEDED_DROP = "Queue disc limit exceeded";  //!< Packet dropped due to queue disc limit exceeded


  void setStrictPriority() {
    strict_priority = 1;
    round_robin=0;//Just to avoid clash
  }

  void setRoundRobin() {
    round_robin = 1;
    strict_priority = 0;//Just to avoid clash
  }

  double GetThroughputQueue(uint32_t p,double nanodelay);
  double GetThroughputPort(double nanodelay);
  double GetThroughputEnQueue(uint32_t p,double nanodelay);

  uint64_t GetBuffersize(uint32_t p);

  void setNPrior(uint32_t np) {
     nPrior = np;
     alphas = new double[nPrior];
     protean_alphas = new double[nPrior];

  }
  double *alphas;
  double *protean_alphas;

  uint32_t getNPrior() {
    return nPrior;
  }

  void setPortBw(double bw){portBW = bw;}

  void SetSharedMemory(Ptr<SharedMemoryBuffer> sm){
    sharedMemory=sm;

    // InitQueueThresholds
    for (uint32_t priority = 0; priority < 11; priority++){
      queue_thresh[priority] = DT_alpha * sharedMemory->GetSharedBufferSize();
    }

  }

  void SetBufferAlgorithm(uint32_t alg){
    bufferalg=alg;
  }

  void SetPortId(uint32_t port){portId=port;}
  uint32_t getPortId(){return portId;}

  void SetFabWindow(Time t){FabWindow=t;}
  void SetFabThreshold(uint32_t n){FabThreshold=n;}

  void SetName(std::string name){switchname=name;}

  bool DynamicThresholds(uint32_t priority, Ptr<Packet> packet);
  bool Protean(uint32_t priority, Ptr<Packet> packet);
  void UpdateDequeueRate(double nanodelay);
  void UpdateNofP();
  void InvokeUpdates(double nanodelay);

  bool FlowAwareBuffer(uint32_t priority, Ptr<Packet> packet);

  bool CompleteSharing(uint32_t priority, Ptr<Packet> packet);

  int DropAfd(double prob,uint32_t priority);
  void SetAfdWindow(Time t){AfdWindow=t;}
  void SetQrefAfd(uint32_t p, uint32_t ref);//{QRefAfd[p]=ref;}
  uint32_t GetQrefAfd(uint32_t p);//{return QRefAfd[p];}
  void SetDppWindow(Time t){DppWindow=t;}
  void SetDppThreshold(uint32_t n){DppThreshold=n;}
  bool IntelligentBuffer(uint32_t priority, Ptr<Packet> packet);

  bool AcceptPacket(uint32_t priority, Ptr<Packet> packet);

  void TrimPacket(Ptr<Packet> packetCopy);

  void ConfigDTAlpha(double alpha);
	void ConfigProteanBeta(double beta);
  void ConfigBuildupThresh(double thresh);
  void UsePerPacketProtean(bool use_ppp);
  void UseMaxDqDt(bool use_maxdqdt);
  void UseNormalizedDqDt (bool use_normalized);
  void UseMultiPrioThresh(bool use_smaller_thresh_for_multiprio);
  void UseHighPrioShorts(bool use_high_prio_shorts);
  void CalculateProteanThreshold(uint32_t priority, Ptr<Packet> packet);

  double getInstDqDt (uint32_t priority) {return dqdt_inst[priority];}

  double getMaxDqDt (uint32_t priority) {return dqdt_max[priority];}

  double getEWMADqDt (uint32_t priority) {return dqdt_prev[priority];}

  double getThresh (uint32_t priority) {return queue_thresh[priority];}

private:
  virtual bool DoEnqueue (Ptr<QueueDiscItem> item);
  virtual Ptr<QueueDiscItem> DoDequeue (void);
  virtual Ptr<const QueueDiscItem> DoPeek (void);
  virtual bool CheckConfig (void);
  virtual void InitializeParams (void);

  uint64_t droppedBytes[101];

  /*at enqueue*/
  Time firstSeen[101];
  Time lastAccepted[101];
  double numBytesSent[101];

  /*at dequeue*/
  Time firstSeenQueue[11];
  Time lastAcceptedQueue[11];
  double numBytesSentQueue[11];

  double DeqRate[11];
  double Deq[11];

  uint32_t nPrior;
  std::unordered_map<uint32_t,uint32_t> flowPrior;
  Time window;
  Time timeSinceLastChange;
  uint32_t new_index = 0;

  uint32_t dequeueIndex=0;
  uint32_t strict_priority;
  uint32_t round_robin;

  Ptr<SharedMemoryBuffer> sharedMemory;
  uint32_t bufferalg;
  uint32_t portId;
  uint32_t sat;
  std::string switchname; //optional

  std::unordered_map<uint32_t,std::pair<uint32_t,Time>> FlowCount; // FlowId --> <packetcounter, LastSeen>

  uint64_t bufferMax[11]={0};

  uint64_t updateInterval;
  bool firstTimeUpdate = true;

  uint64_t staticBuffer;
  uint64_t staticOccupancy=0;

  double alphaUnsched;

  Time FabWindow; // Needs to be set in experiment code.
  uint32_t FabThreshold; // Needs to be set in experiment code.
  Time AfdWindow; // Needs to be set in experiment code.
  uint32_t QRefAfd[11]; // Needs to be set in experiment code.
  Time DppWindow; // Needs to be set in experiment code.
  uint32_t DppThreshold; // Needs to be set in experiment code.

  Time timeSinceLastChangeAdf=Seconds(0);
  std::unordered_map<uint32_t,std::pair<uint32_t,uint32_t>> M; // FlowId --> <counterNow, Total count in last window>
  double MFair[11]={0};
  uint32_t Qold[11]={0};
  uint32_t DPPQueue=1;

  double a1=1.8;
  double a2=1.7;

  double portBW;// Needs to be set in experiment code via attribute.

  double nofP[11];

  bool is_homa;

  uint64_t txBytesInt=0;
  bool enableDPPQueue;

  double DT_alpha;
  double Protean_beta;
  double buildup_thresh;
	bool per_packet_Protean;
  bool use_max_dqdt;
  bool use_normalized_dqdt;
  bool use_multi_prio_thresh;
  bool use_high_prio_shorts;
  uint32_t egress_bytes[11];
  uint32_t egress_packets[11];
  double time_prev[11];
  double qlen_prev[11];
	double dqdt_prev[11];
  double dqdt_max[11];
  double dqdt_inst[11];
	double queue_thresh[11];
};

} // namespace ns3

#endif /* GEN_QUEUE_DISC_H */
