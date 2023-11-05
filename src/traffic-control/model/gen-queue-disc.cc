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


# define DT 101
# define FAB 102
# define CS 103
# define IB 104
# define PROTEAN 114

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("GenQueueDisc");

NS_OBJECT_ENSURE_REGISTERED (GenQueueDisc);

TypeId GenQueueDisc::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::GenQueueDisc")
    .SetParent<QueueDisc> ()
    .SetGroupName ("TrafficControl")
    .AddConstructor<GenQueueDisc> ()
    .AddAttribute ("nPrior","number of queues", UintegerValue (5),
                                     MakeUintegerAccessor (&GenQueueDisc::nPrior),
                                        MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("sat","saturation detection",
                    UintegerValue (20*1400),
                    MakeUintegerAccessor (&GenQueueDisc::sat),
                    MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("BufferAlgorithm","BufferAlgorithm",
                    UintegerValue (DT),
                    MakeUintegerAccessor (&GenQueueDisc::bufferalg),
                    MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("enableDPPQueue","whether to use extra priority queue or not. This concerns IB algorithm. Turn this off in single queue setting.",
                    BooleanValue (false),
                    MakeBooleanAccessor (&GenQueueDisc::enableDPPQueue),
                    MakeBooleanChecker())
    .AddAttribute ("alphaUnsched","alphaUnsched",
                    DoubleValue (1024),
                    MakeDoubleAccessor (&GenQueueDisc::alphaUnsched),
                    MakeDoubleChecker<double> ())
    .AddAttribute ("portBW","portBW in Gbps",
                    DoubleValue (10),
                    MakeDoubleAccessor (&GenQueueDisc::portBW),
                    MakeDoubleChecker<double> ())
    .AddAttribute ("staticBuffer","static buffer",
                              UintegerValue (0),
                              MakeUintegerAccessor (&GenQueueDisc::staticBuffer),
                              MakeUintegerChecker<uint64_t> ())
    .AddAttribute ("RoundRobin","round robin scheduling",
                              UintegerValue (1),
                              MakeUintegerAccessor (&GenQueueDisc::round_robin),
                              MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("StrictPriority","strict priority scheduling",
                              UintegerValue (0),
                              MakeUintegerAccessor (&GenQueueDisc::strict_priority),
                              MakeUintegerChecker<uint32_t> ())
  ;
  return tid;
}

GenQueueDisc::GenQueueDisc ()
  : QueueDisc (QueueDiscSizePolicy::MULTIPLE_QUEUES, QueueSizeUnit::BYTES)
{
  NS_LOG_FUNCTION (this);
  alphas = nullptr;
  protean_alphas = nullptr;
  for (uint32_t i=0;i<11;i++){
    firstSeen[i]= Seconds(0);
    lastAccepted[i]=ns3::Simulator::Now();
    numBytesSent[i]=0;
    firstSeenQueue[i]= Seconds(0);
    lastAcceptedQueue[i]=ns3::Simulator::Now();
    numBytesSentQueue[i]=0;
    droppedBytes[i]=0;
    DeqRate[i]=1;
    Deq[i]=0;
    MFair[i]=1000*1000*4;
    QRefAfd[i]=1000*15;
    nofP[i] = 0;
    DPPQueue = 1;

    time_prev[i] = 0;
    qlen_prev[i] = 0;
    dqdt_prev[i] = 0;
    dqdt_max[i] = 0;
    egress_bytes[i] = 0;
    egress_packets[i] = 0;
    dqdt_inst[i] = 0;

  }


  DT_alpha = 1;
  Protean_beta = 0.25;

  // initialize time_prev and qlen_prev to zero for all ports and queues
  // memset(time_prev, 0, sizeof(time_prev));
  // memset(qlen_prev, 0, sizeof(qlen_prev));
  // memset(dqdt_prev, 0, sizeof(dqdt_prev));
  // memset(dqdt_max, 0, sizeof(dqdt_max));
  // memset(egress_bytes, 0, sizeof(egress_bytes));
  // memset(egress_packets, 0, sizeof(egress_packets));
  // memset(dqdt_inst, 0, sizeof(dqdt_inst));

}

GenQueueDisc::~GenQueueDisc ()
{
  NS_LOG_FUNCTION (this);
  if (alphas)
    delete alphas;
  if (protean_alphas)
    delete protean_alphas;
}

uint64_t
GenQueueDisc::GetBuffersize(uint32_t p){
  uint64_t temp = bufferMax[p];
  bufferMax[p]=0;
  return temp;
}

double
GenQueueDisc::GetThroughputEnQueue(uint32_t p, double nanodelay){
    double th = 1e9*8*numBytesSent[p]/nanodelay;
    numBytesSent[p]=0;
  return th;
}


bool GenQueueDisc::DynamicThresholds(uint32_t priority, Ptr<Packet> packet){

  double remaining = sharedMemory->GetRemainingBuffer();

  uint64_t maxSize = alphas[priority]*remaining;
  queue_thresh[priority] = maxSize;
  // uint64_t maxSize = DT_alpha*remaining;
  if (maxSize> UINT32_MAX)
    maxSize = UINT32_MAX-1500;

  uint32_t qSize = GetQueueDiscClass (priority)->GetQueueDisc ()->GetNBytes();
  if ( ((qSize + packet->GetSize()) >  maxSize) || (sharedMemory->GetRemainingBuffer() < packet->GetSize())  ){
    sharedMemory->incrementDroppedPacketCount(getPortId(), priority);
    // printf("time: %lu\t DT Drop# %d\t from port %d\t using %d\t bytes when shared_used_bytes was: \t %d and threshold was: \t%lu\n", Simulator::Now().GetTimeStep(), sharedMemory->GetTotalDroppedPacketCount(), getPortId(), qSize, sharedMemory->GetOccupiedBuffer(), maxSize);
    return false; // drop
  }
  else{
    return true;
  }

}


void GenQueueDisc::ConfigDTAlpha(double alpha){
  DT_alpha = alpha;
}
void GenQueueDisc::ConfigProteanBeta(double beta){
  Protean_beta = beta;
}
void GenQueueDisc::ConfigBuildupThresh(double thresh){
  buildup_thresh = thresh;
}

void GenQueueDisc::UsePerPacketProtean(bool use_ppp){
  per_packet_Protean = use_ppp;
}
void GenQueueDisc::UseMaxDqDt(bool use_maxdqdt){
  use_max_dqdt = use_maxdqdt;
}
void GenQueueDisc::UseNormalizedDqDt(bool use_normalized){
  use_normalized_dqdt = use_normalized;
}
void GenQueueDisc::UseMultiPrioThresh(bool use_smaller_thresh_for_multiprio){
  use_multi_prio_thresh = use_smaller_thresh_for_multiprio;
}
void GenQueueDisc::UseHighPrioShorts(bool use_high_prio_for_shorts){
  use_high_prio_shorts = use_high_prio_for_shorts;
}
bool GenQueueDisc::Protean(uint32_t priority, Ptr<Packet> packet){

  if (use_high_prio_shorts){
    bool found;
    uint32_t unsched = 0;
    UnSchedTag tag;
    found = packet->PeekPacketTag (tag);
    if(found){
      unsched=tag.GetValue();
    }
    if (unsched){
      return true;
    }
  }

  uint32_t used = GetQueueDiscClass (priority)->GetQueueDisc ()->GetNBytes();

  if (packet->GetSize() + used > queue_thresh[priority]){
    sharedMemory->incrementDroppedPacketCount(getPortId(), priority);
  	// printf("time: %lu\t Protean Drop# %d\t from port %d\t using %d\t bytes when shared_used_bytes was: \t %d remaining was: \t %d and DT_alpha was \t %lf and dqdt was: \t%lf and threshold was: \t%lf\n", Simulator::Now().GetTimeStep(), sharedMemory->GetTotalDroppedPacketCount(), getPortId(), used, sharedMemory->GetOccupiedBuffer(), sharedMemory->GetRemainingBuffer(), DT_alpha, dqdt_prev[priority], queue_thresh[priority]);
  	return false;
  }
  return true;
}

void GenQueueDisc::CalculateProteanThreshold(uint32_t priority, Ptr<Packet> packet) {
    double time_curr = Simulator::Now().GetNanoSeconds();
		double delta_t, delta_q, dqdt_curr;
    double protean_alpha = DT_alpha;

    if (use_multi_prio_thresh){
      DT_alpha = alphas[priority];
      protean_alpha = protean_alphas[priority];
    }
    else{
      protean_alpha = protean_alphas[0];
      DT_alpha = alphas[0];
    }
    delta_t = time_curr - time_prev[priority];
    time_prev[priority] = time_curr;

		if (!per_packet_Protean){
      egress_bytes[priority] = GetQueueDiscClass (priority)->GetQueueDisc ()->GetNBytes();

			delta_q = (egress_bytes[priority] - qlen_prev[priority]);
      if (delta_t == 0) {
        //printf("delta_t is: %lf delta_q is: %lf for port: %d\n", delta_t, delta_q, getPortId());
        dqdt_curr = 0;
        //buildup_thresh = queue_thresh[priority];
      } else {
        dqdt_curr = delta_q / delta_t;
      }
			//printf("Buffer buildup thresh: %lf\n", buildup_thresh);
			qlen_prev[priority] = egress_bytes[priority];
		} else {
      egress_packets[priority] = GetQueueDiscClass (priority)->GetQueueDisc ()->GetNPackets();
			delta_q = egress_packets[priority] - qlen_prev[priority];
			if (delta_q > 1){
				//printf("delta_q: %lf, dqdt_curr is: %lf, thresh is: %lf\n", delta_q, delta_q / delta_t, 1/delta_t);
			}
			dqdt_curr = delta_q / delta_t;
			buildup_thresh = 1 / delta_t;
			qlen_prev[priority] = egress_packets[priority];
		}

    dqdt_inst[priority] = dqdt_curr;


    dqdt_curr = dqdt_curr * Protean_beta + (1 - Protean_beta) * dqdt_prev[priority];
    ////////////////////////////////////////////////////////////////////////////////
    // Doing Max(EWMA), if want instantaneous, move this block above

    if (dqdt_curr > dqdt_max[priority]) {
      dqdt_max[priority] = dqdt_curr;
    }

    if (dqdt_curr > sharedMemory->GetMaxDqDt()) {
      // printf("Max ewma dqdt: %lf\n", dqdt_curr);
      sharedMemory->SetMaxDqDt(dqdt_curr);
    }
    if (use_max_dqdt){
      // dqdt_curr = dqdt_max[priority];
      dqdt_curr = sharedMemory->GetMaxDqDt();
    }

    ////////////////////////////////////////////////////////////////////////////////
    dqdt_prev[priority] = dqdt_curr;


    sharedMemory->SetDqDt(getPortId(), priority, dqdt_curr);

    double sum_positive_dqdt = 0;
    for (uint32_t i = 0; i < 100; i++) {
      for (uint32_t j = 0; j < 8; j++) {
        double dqdt = sharedMemory->GetDqDt(i, j);
        if (dqdt > buildup_thresh) {
          sum_positive_dqdt += dqdt;
        }
      }
    }

		// if (port == 1 && qIndex == 3 && dqdt_curr > buildup_thresh && dqdt_curr < 1){
		// 	//printf("%lf\n", dqdt_curr);
		// }
    if (bufferalg == PROTEAN){
      if (dqdt_curr > buildup_thresh)
      {
        // printf("Greater than thresh: %lf, port: %d, \t Qind: %d \t dqdt_curr: %lf\t DT_alpha: %lf\tbuffer_size: %d\tshared_used_bytes: %d\n", buildup_thresh,getPortId(), priority, dqdt_curr, DT_alpha, sharedMemory->GetSharedBufferSize(), sharedMemory->GetOccupiedBuffer());
        if (use_normalized_dqdt)
        {
          if (sum_positive_dqdt > 0)
          {
            queue_thresh[priority] = dqdt_curr * protean_alpha * sharedMemory->GetRemainingBuffer() / sum_positive_dqdt;
          }
          else
          {
              queue_thresh[priority] = dqdt_curr * protean_alpha * sharedMemory->GetRemainingBuffer();
          }
        }
        else
        {
          queue_thresh[priority] = dqdt_curr * protean_alpha * sharedMemory->GetRemainingBuffer();
        }
      }
      else  // falling back to DT
      {
        // printf("Less than thresh:%lf, port: %d, \t Qind: %d \t dqdt_curr: %lf\t DT_alpha: %lf\tbuffer_size: %d\tshared_used_bytes: %d\n", buildup_thresh,getPortId(), priority, dqdt_curr, DT_alpha, sharedMemory->GetSharedBufferSize(), sharedMemory->GetOccupiedBuffer());
        queue_thresh[priority] =  protean_alpha * sharedMemory->GetRemainingBuffer();
      }
    }
}

void
GenQueueDisc::UpdateDequeueRate(double nanodelay){ // delay in NANOSECONDS. Pay attention here.
  // double num=0;
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
  for (uint32_t p=0;p<nPrior;p++){
    double th = 8*Deq[p]/nanodelay/portBW; // portBW should be in Gbps
    if (th < 1.0/double(nPrior) || th > 1){
      th = 1;
    }
    DeqRate[p] = th;
    Deq[p] = 0;
  }
}

void GenQueueDisc::UpdateNofP(){
  for (uint32_t i=0; i< nPrior; i++){
    nofP[i] = sharedMemory->GetNofP(i);
  }
}


void GenQueueDisc::InvokeUpdates(double nanodelay){
  UpdateDequeueRate(nanodelay);
  UpdateNofP();
  Simulator::Schedule(NanoSeconds(nanodelay),&GenQueueDisc::InvokeUpdates,this,nanodelay);
}

bool GenQueueDisc::FlowAwareBuffer(uint32_t priority, Ptr<Packet> packet){

  double alpha;

  /* Find flow-id if exists */
  bool found;
  uint32_t flowId = 0;
  FlowIdTag tag;
  found = packet->PeekPacketTag (tag);
  if(found){
    flowId=tag.GetFlowId();
  }

  /* Find the flow entry */
  if(FlowCount.find(flowId) == FlowCount.end()){
    FlowCount[flowId].first=0;
    FlowCount[flowId].second=Simulator::Now();
  }

  /* If the flow did not appear in the last FabWindow duration, reset its bytes counter to zero. */
  if(Simulator::Now()-FlowCount[flowId].second>FabWindow){
    FlowCount[flowId].first=0;
  }

  /* Per-flow counters - increment bytes count and last updated time. */
  FlowCount[flowId].first+=packet->GetSize();
  FlowCount[flowId].second=Simulator::Now();

  /* If the flow sent less than FabThreshold no.of bytes in the last FabWindow, then prioritize these packets */
  if(FlowCount[flowId].first<FabThreshold){
    alpha = alphaUnsched; // alphaUnsched is usually set to a high value i.e., these packets are prioritized.
  }
  else{
    alpha=alphas[priority];
  }

  double remaining = sharedMemory->GetRemainingBuffer();
  uint64_t maxSize = alpha*remaining;
  if (maxSize> UINT32_MAX)
    maxSize = UINT32_MAX-1500;


  uint32_t qSize = GetQueueDiscClass (priority)->GetQueueDisc ()->GetNBytes();
  if ( ((qSize + packet->GetSize()) >  maxSize) || (sharedMemory->GetRemainingBuffer() < packet->GetSize())  ){
    return false; // drop
  }
  else{
    return true;
  }

}

bool GenQueueDisc::CompleteSharing(uint32_t priority, Ptr<Packet> packet){
  if(sharedMemory->GetRemainingBuffer() < packet->GetSize()){
    sharedMemory->incrementDroppedPacketCount(getPortId(), priority);
    // printf("Total CS dropped packets: %d\n", sharedMemory->GetTotalDroppedPacketCount());
    return false;// drop
  }
  else{
    return true;
  }
}

void
GenQueueDisc::SetQrefAfd(uint32_t p, uint32_t ref){
  QRefAfd[p]=ref;
}
uint32_t
GenQueueDisc::GetQrefAfd(uint32_t p){
  return QRefAfd[p];
}

int
GenQueueDisc::DropAfd(double prob,uint32_t priority){
  uint32_t qsize = GetQueueDiscClass (priority)->GetQueueDisc ()->GetNBytes();
  double x = double(rand())/RAND_MAX;
  // 150*1024 is the recommended value for 10Gbps links https://www.cisco.com/c/en/us/products/collateral/switches/nexus-9000-series-switches/white-paper-c11-738488.html
  return ((x<prob) && (qsize>150*1024));
}


bool GenQueueDisc::IntelligentBuffer(uint32_t priority, Ptr<Packet> packet){
  bool accept;
  if(Simulator::Now() > AfdWindow + timeSinceLastChangeAdf){
    for(auto it=M.begin();it!=M.end();++it){
      it->second.first=it->second.second;
      it->second.second=1; //1 just to avoid divide by zero errors
    }
    for(uint32_t i=0;i<nPrior;i++){
      uint32_t Qnow = GetQueueDiscClass (i)->GetQueueDisc ()->GetNBytes();
      MFair[i]=MFair[i]-a1*((double)Qnow - (double)QRefAfd[i])+a2*((double)Qold[i] - (double)QRefAfd[i]); // a1 and a2 --> 1.8 and 1.7
      if(MFair[i]<0)
        MFair[i]=0;

      Qold[i]=Qnow;
    }
    timeSinceLastChangeAdf=Simulator::Now();
  }

  bool found;
  uint32_t flowId = 0;
  FlowIdTag tag;
  found = packet->PeekPacketTag (tag);
  if(found){flowId=tag.GetFlowId();}

  if(FlowCount.find(flowId) == FlowCount.end()){
      FlowCount[flowId].first=0;
      FlowCount[flowId].second=Simulator::Now();
  }

  //DPP
  if(Simulator::Now()-FlowCount[flowId].second>DppWindow)
    FlowCount[flowId].first=0;

  FlowCount[flowId].first+=1;
  FlowCount[flowId].second=Simulator::Now();

  if(FlowCount[flowId].first<DppThreshold && enableDPPQueue){ // Short flows are sent to queue-0 which is a priority queue.
    DPPQueue=0;
    accept = DynamicThresholds(DPPQueue,packet);
  }
  else{
    M[priority].second += packet->GetSize();

    if(!M[priority].first){
      M[priority].first=1; // Just to avoid divide by zero.
    }
    double dropP = 1.0-(double(std::min(15*M[priority].first,uint32_t(MFair[priority])))/(15*M[priority].first));
    if(dropP<0){
      dropP=0;
    }

    DPPQueue = priority;
    accept = (DynamicThresholds(DPPQueue,packet) && !DropAfd(DPPQueue,dropP));
  }
  return accept;
}



bool GenQueueDisc::AcceptPacket(uint32_t priority, Ptr<Packet> packet){
  bool accept;
  switch (bufferalg){
    case DT:
      accept = DynamicThresholds(priority,packet);
      break;
    case FAB:
      accept = FlowAwareBuffer(priority,packet);
      break;
    case CS:
      accept = CompleteSharing(priority,packet);
      break;
    case IB:
      accept = IntelligentBuffer(priority,packet);
      break;
    case PROTEAN:
      accept = Protean(priority,packet);
      break;
    default:
      accept = DynamicThresholds(priority,packet);
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

  uint32_t p=0;

  bool found;
  MyPriorityTag a;
  found = packet->PeekPacketTag(a);
  if(found)p=a.GetPriority();

  if (uint32_t(p)>=nPrior)
    p = uint32_t(nPrior-1);
  /* Arrival Statistics*/
  numBytesSent[p]+=item->GetSize();
  uint64_t sizenow = GetQueueDiscClass (p)->GetQueueDisc ()->GetNBytes();
  if (bufferMax[p] < sizenow){
    bufferMax[p]=sizenow;
  }
  /*Check if we can use the reserved space*/
  if (GetCurrentSize().GetValue() + item->GetSize() <= staticBuffer){
    bool ret = GetQueueDiscClass (p)->GetQueueDisc ()->Enqueue (item);

    if(firstSeen[p]==Seconds(0)){
      firstSeen[p]=Simulator::Now();
    }
    lastAccepted[p]=Simulator::Now();
    return ret;
  }

  /*Check if the packet can be put in the shared buffer*/
  bool enqueue = AcceptPacket(p,packet);
  if (!enqueue) {

      NS_LOG_LOGIC ("Queue disc limit exceeded -- dropping packet");
      // std::cout << " maxSize " << maxSize << " remaining " << sharedMemory->GetRemainingBuffer() << " packetSize " << item->GetSize() << " priority " << uint32_t(p) << " alpha " << alphas[p] << " thresh " << uint64_t (alphas[p]*(sharedMemory->GetRemainingBuffer())) << " deq " << DeqRate[p] << " N " << sharedMemory->GetNofP(p) << std::endl;
      DropBeforeEnqueue (item, LIMIT_EXCEEDED_DROP);
      return false;
  }

  /*If algorithm is Intelligent Buffer, it may change the queue to zero (DPP prioritizes short flows to separate queue)*/
  if (bufferalg==IB && enableDPPQueue){
    p = DPPQueue;
  }

  /*increment shared buffer occupancy*/
  bool retval;
  if(!sharedMemory->EnqueueBuffer(item->GetSize())) {
    DropBeforeEnqueue (item, LIMIT_EXCEEDED_DROP);
    retval = false;
  }
  else{
    sharedMemory->PerPriorityStatEnq(item->GetSize(),p);
    retval = GetQueueDiscClass (p)->GetQueueDisc ()->Enqueue (item);
  }

  if (!retval)
    {
      NS_LOG_WARN ("Packet enqueue failed. Check the size of the internal queues");
    }
  else{
    if(firstSeen[p]==Seconds(0)){
      firstSeen[p]=Simulator::Now();
    }
    lastAccepted[p]=Simulator::Now();
  }

  NS_LOG_LOGIC ("Number packets p " << p << ": " << GetQueueDiscClass (p)->GetQueueDisc ()->GetNPackets ());

  return retval;
}


double
GenQueueDisc::GetThroughputQueue(uint32_t p, double nanodelay){
    double th = 8*numBytesSentQueue[p]/nanodelay/portBW;
    numBytesSentQueue[p]=0;
    return th;
}

double
GenQueueDisc::GetThroughputPort(double nanodelay){ // delay must be in nanoseconds
    double th = 8*numBytesSentQueue[10]/nanodelay/portBW;
    numBytesSentQueue[10]=0;
    return th;
}

Ptr<QueueDiscItem>
GenQueueDisc::DoDequeue (void)
{
  NS_LOG_FUNCTION (this);

  Ptr<QueueDiscItem> item;
  /* Round robin scheduling. Nothing fancy here. More scheduling algorithms to be added later. */
  if (round_robin){
    for (uint32_t i = 0; i < GetNQueueDiscClasses(); i++)
      {
        if ((item = GetQueueDiscClass (dequeueIndex)->GetQueueDisc ()->Dequeue ()) != 0)
          {

            Ptr<Packet> packet = item->GetPacket();

            uint32_t p = dequeueIndex;
            CalculateProteanThreshold(p, packet);

            numBytesSentQueue[p]+=item->GetSize();

            // 10 is used for aggregate. Assuming that the actual number of queues are less than 10.
            numBytesSentQueue[10]+=item->GetSize();

            Deq[p]+=item->GetSize();
            if (GetCurrentSize().GetValue() + packet->GetSize() > staticBuffer){
              sharedMemory->DequeueBuffer(item->GetSize());
              sharedMemory->PerPriorityStatDeq(item->GetSize(),p);
            }

            dequeueIndex++;
            if (dequeueIndex>=GetNQueueDiscClasses())
              dequeueIndex=0;

            FeedbackTag Int;
            bool found;
            found = packet->PeekPacketTag(Int);
            if(found){
              Int.setTelemetryQlenDeq(Int.getHopCount(), GetQueueDiscClass (p)->GetQueueDisc ()->GetNBytes()); // queue length at dequeue
              Int.setTelemetryTsDeq(Int.getHopCount(), Simulator::Now().GetNanoSeconds()); // timestamp at dequeue
              Int.setTelemetryBw(Int.getHopCount(), portBW*1e9);
              Int.setTelemetryTxBytes(Int.getHopCount(), txBytesInt);
              Int.incrementHopCount(); // Incrementing hop count at Dequeue. Don't do this at enqueue.
              packet->ReplacePacketTag(Int); // replacing the tag with new values
              // std::cout << "found " << Int.getHopCount() << std::endl;
            }
            txBytesInt+=packet->GetSize();

            return item;
          }
        Deq[dequeueIndex]+=1472;

        dequeueIndex++;
        if (dequeueIndex>=GetNQueueDiscClasses())
          dequeueIndex=0;
      }
  }
  else{
    /*Strict priority scheduling*/
    for (uint32_t i = 0; i < GetNQueueDiscClasses(); i++)
      {
        if ((item = GetQueueDiscClass (i)->GetQueueDisc ()->Dequeue ()) != 0)
          {

            Ptr<Packet> packet = item->GetPacket();

            uint32_t p = i;

            numBytesSentQueue[p]+=item->GetSize();

            // 10 is used for aggregate. Assuming that the actual number of queues are less than 10.
            numBytesSentQueue[10]+=item->GetSize();

            Deq[p]+=item->GetSize();
            if (GetCurrentSize().GetValue() + packet->GetSize() > staticBuffer){
              sharedMemory->DequeueBuffer(item->GetSize());
              sharedMemory->PerPriorityStatDeq(item->GetSize(),p);
            }

            FeedbackTag Int;
            bool found;
            found = packet->PeekPacketTag(Int);
            if(found){
              Int.setTelemetryQlenDeq(Int.getHopCount(), GetQueueDiscClass (p)->GetQueueDisc ()->GetNBytes()); // queue length at dequeue
              Int.setTelemetryTsDeq(Int.getHopCount(), Simulator::Now().GetNanoSeconds()); // timestamp at dequeue
              Int.setTelemetryBw(Int.getHopCount(), portBW*1e9);
              Int.setTelemetryTxBytes(Int.getHopCount(), txBytesInt);
              Int.incrementHopCount(); // Incrementing hop count at Dequeue. Don't do this at enqueue.
              packet->ReplacePacketTag(Int); // replacing the tag with new values
              // std::cout << "found " << Int.getHopCount() << std::endl;
            }
            txBytesInt+=packet->GetSize();

            return item;
          }
        Deq[i]+=1472;
      }
  }
  NS_LOG_LOGIC ("Queue empty");
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
