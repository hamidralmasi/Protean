
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <fstream>
#include <iomanip>
#include <map>
#include <ctime>
#include <set>
#include <unordered_map>

#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/traffic-control-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/gen-queue-disc.h"
#include "ns3/red-queue-disc.h"
#include "ns3/fq-pie-queue-disc.h"
#include "ns3/fq-codel-queue-disc.h"
#include "ns3/shared-memory.h"

# define PACKET_SIZE 1400
# define GIGA 1000000000

/*Buffer Management Algorithms*/
# define DT 101
# define CS 103
# define FAB 102
# define IB 104
# define PROTEAN 114

/*Congestion Control Algorithms*/
# define DCTCP 2
# define CUBIC 1

#define fixed_prio 1

#define PORT_END 65530

extern "C"
{
#include "cdf.h"
}


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("PROTEAN_EVALUATION");

uint32_t PORT_START[512] = {4444};

double alpha_values[8] = {1};
double protean_alpha_values[8] = {1};


Ptr<OutputStreamWrapper> fctOutput;
AsciiTraceHelper asciiTraceHelper;

Ptr<OutputStreamWrapper> torStats;
AsciiTraceHelper torTraceHelper;

Ptr<OutputStreamWrapper> algStats;
AsciiTraceHelper algTraceHelper;

Ptr<SharedMemoryBuffer> sharedMemoryLeaf[10];
QueueDiscContainer northQueues[10];
QueueDiscContainer ToRQueueDiscs[10];

double poission_gen_interval(double avg_rate)
{
	if (avg_rate > 0)
		return -logf(1.0 - (double)rand() / RAND_MAX) / avg_rate;
	else
		return 0;
}

template<typename T>
T rand_range (T min, T max)
{
	return min + ((double)max - min) * rand () / RAND_MAX;
}

double baseRTTNano;
double nicBw;
void TraceMsgFinish (Ptr<OutputStreamWrapper> stream, double size, double start, bool incast, uint32_t prior )
{
	//printf("Flow FINISHED\n");
	double fct, standalone_fct, slowdown;
	fct = Simulator::Now().GetNanoSeconds() - start;
	standalone_fct = baseRTTNano + size * 8.0 / nicBw;
	slowdown = fct / standalone_fct;

	*stream->GetStream ()
	        << Simulator::Now().GetNanoSeconds()
	        << " " << size
	        << " " << fct
	        << " " << standalone_fct
	        << " " << slowdown
	        << " " << baseRTTNano / 1e3
	        << " " << (start / 1e3 - Seconds(10).GetMicroSeconds())
	        << " " << prior
	        << " " << incast
	        << std::endl;
}

void
InvokeToRStats(Ptr<OutputStreamWrapper> stream, uint32_t BufferSize, uint32_t leafId, double nanodelay) {
	Ptr<SharedMemoryBuffer> sm = sharedMemoryLeaf[leafId];
	QueueDiscContainer queues = northQueues[leafId];
	double totalThroughput = 0;
	for (uint32_t i = 0; i < queues.GetN(); i++) {
		Ptr<GenQueueDisc> genDisc = DynamicCast<GenQueueDisc>(queues.Get(i));
		totalThroughput += genDisc->GetThroughputPort(nanodelay);
	}
	totalThroughput = totalThroughput / queues.GetN();
	*stream->GetStream()
	        << Simulator::Now().GetNanoSeconds()
	        << " " << leafId
	        << " " << double(BufferSize) / 1e6
	        << " " << 100 * double(sm->GetOccupiedBuffer()) / BufferSize
	        << " " << 100 * totalThroughput
	        << " " << 100 * double(sm->GetPerPriorityOccupied(0)) / BufferSize
	        << " " << 100 * double(sm->GetPerPriorityOccupied(1)) / BufferSize
	        << " " << 100 * double(sm->GetPerPriorityOccupied(2)) / BufferSize
	        << " " << 100 * double(sm->GetPerPriorityOccupied(3)) / BufferSize
	        << " " << 100 * double(sm->GetPerPriorityOccupied(4)) / BufferSize
	        << " " << 100 * double(sm->GetPerPriorityOccupied(5)) / BufferSize
	        << " " << 100 * double(sm->GetPerPriorityOccupied(6)) / BufferSize
	        << " " << 100 * double(sm->GetPerPriorityOccupied(7)) / BufferSize
	        << std::endl;

	Simulator::Schedule(NanoSeconds(nanodelay), InvokeToRStats, stream, BufferSize, leafId, nanodelay);
}

void
InvokeAlgStats(Ptr<OutputStreamWrapper> stream, uint32_t BufferSize, uint32_t leafId, double nanodelay) {
	Ptr<SharedMemoryBuffer> sm = sharedMemoryLeaf[leafId];
	QueueDiscContainer queues = ToRQueueDiscs[leafId];
	Ptr<GenQueueDisc> p1, p2, p3, p4;
	for (uint32_t i = 0; i < queues.GetN(); i++) {
		Ptr<GenQueueDisc> genDisc = DynamicCast<GenQueueDisc>(queues.Get(i));
		if (genDisc->getPortId() == 16) {
			p1 = genDisc;
		}
		else if (genDisc->getPortId() == 17) {
			p2 = genDisc;
		}
		else if (genDisc->getPortId() == 18) {
			p3 = genDisc;
		}
		else if (genDisc->getPortId() == 19) {
			p4 = genDisc;
		}
	}

	*stream->GetStream()
	        << Simulator::Now().GetNanoSeconds()
	        << " " << leafId
	        << " " << double(BufferSize) / 1e6
			<< " " << 100 * double(sm->GetOccupiedBuffer()) / BufferSize
			<< " " << 100 * double(sm->GetRemainingBuffer()) / BufferSize
			<< " " << sm->GetTotalDroppedPacketCount()
			<< " " << p1->getInstDqDt(1)
			<< " " << p1->getInstDqDt(2)
			<< " " << p1->getEWMADqDt(1)
			<< " " << p1->getEWMADqDt(2)
			<< " " << p1->getMaxDqDt(1)
			<< " " << p1->getMaxDqDt(2)
			<< " " << p1->GetQueueDiscClass (fixed_prio)->GetQueueDisc ()->GetNBytes()
			<< " " << p1->getThresh (fixed_prio)
			<< " " << 100 * p1->GetThroughputPort(nanodelay)
			<< " " << sm->GetDroppedPacketCount(p1->getPortId(), fixed_prio)
			<< " " << p2->getInstDqDt(1)
			<< " " << p2->getInstDqDt(2)
			<< " " << p2->getEWMADqDt(1)
			<< " " << p2->getEWMADqDt(2)
			<< " " << p2->getMaxDqDt(1)
			<< " " << p2->getMaxDqDt(2)
			<< " " << p2->GetQueueDiscClass (fixed_prio)->GetQueueDisc ()->GetNBytes()
			<< " " << p2->getThresh (fixed_prio)
			<< " " << 100 * p2->GetThroughputPort(nanodelay)
			<< " " << sm->GetDroppedPacketCount(p2->getPortId(), fixed_prio)
			<< " " << p3->getInstDqDt(1)
			<< " " << p3->getInstDqDt(2)
			<< " " << p3->getEWMADqDt(1)
			<< " " << p3->getEWMADqDt(2)
			<< " " << p3->getMaxDqDt(1)
			<< " " << p3->getMaxDqDt(2)
			<< " " << p3->GetQueueDiscClass (fixed_prio)->GetQueueDisc ()->GetNBytes()
			<< " " << p3->getThresh (fixed_prio)
			<< " " << 100 * p3->GetThroughputPort(nanodelay)
			<< " " << sm->GetDroppedPacketCount(p3->getPortId(), fixed_prio)
			<< " " << p4->getInstDqDt(1)
			<< " " << p4->getInstDqDt(2)
			<< " " << p4->getEWMADqDt(1)
			<< " " << p4->getEWMADqDt(2)
			<< " " << p4->getMaxDqDt(1)
			<< " " << p4->getMaxDqDt(2)
			<< " " << p4->GetQueueDiscClass (fixed_prio)->GetQueueDisc ()->GetNBytes()
			<< " " << p4->getThresh (fixed_prio)
			<< " " << 100 * p4->GetThroughputPort(nanodelay)
			<< " " << sm->GetDroppedPacketCount(p4->getPortId(), fixed_prio)
	        << std::endl;

	Simulator::Schedule(NanoSeconds(nanodelay), InvokeAlgStats, stream, BufferSize, leafId, nanodelay);
}

void
InvokePerPortToRStats(Ptr<OutputStreamWrapper> stream, uint32_t BufferSize, uint32_t leafId, double nanodelay) {
	Ptr<SharedMemoryBuffer> sm = sharedMemoryLeaf[leafId];
	QueueDiscContainer queues = northQueues[leafId];
	for (uint32_t i = 0; i < queues.GetN(); i++) {
		Ptr<GenQueueDisc> genDisc = DynamicCast<GenQueueDisc>(queues.Get(i));
		double totalThroughput = genDisc->GetThroughputPort(nanodelay);
		*stream->GetStream()
		        << " " << Simulator::Now().GetSeconds()
		        << " " << leafId
		        << " " << genDisc->getPortId()
		        << " " << double(BufferSize) / 1e6
		        << " " << 100 * double(genDisc->GetCurrentSize().GetValue()) / BufferSize
		        << " " << 100 * totalThroughput
		        << std::endl;
	}
	Simulator::Schedule(NanoSeconds(nanodelay), InvokePerPortToRStats, stream, BufferSize, leafId, nanodelay);
}

int tar = 0;
int get_target_leaf(int leafCount) {
	tar += 1;
	if (tar == leafCount) {
		tar = 0;
		return tar;
	}
	return tar;
}

void install_applications_incast (int incastLeaf, NodeContainer* servers, double requestRate, uint32_t requestSize, struct cdf_table *cdfTable,
                                  long &flowCount, int SERVER_COUNT, int LEAF_COUNT, double START_TIME, double END_TIME, double FLOW_LAUNCH_END_TIME, int numPrior, bool use_multiprio_thresh)
{
	int fan = SERVER_COUNT;
	uint64_t flowSize = double(requestSize) / double(fan);
	//printf("flowSize incast: %d\n", flowSize);
	uint32_t prior;
	if (use_multiprio_thresh)
		prior = 1; //rand_range(1, numPrior - 1);
	else
		prior = fixed_prio;

	for (int incastServer = 0; incastServer < SERVER_COUNT; incastServer++)
	{
		double startTime = START_TIME + poission_gen_interval (requestRate);
		while (startTime < FLOW_LAUNCH_END_TIME && startTime > START_TIME)
		{
			// Permutation demand matrix
			int txLeaf = incastLeaf + 1;
			if (txLeaf == LEAF_COUNT) {
				txLeaf = 0;
			}
			// int txLeaf=incastLeaf;
			// while (txLeaf==incastLeaf){
			//     txLeaf = get_target_leaf(LEAF_COUNT);
			// }

			for (uint32_t txServer = 0; txServer < fan; txServer++) {

				uint16_t port = PORT_START[ incastLeaf * SERVER_COUNT + incastServer]++;
				if (port > PORT_END) {
					port = 4444;
					PORT_START[incastLeaf * SERVER_COUNT + incastServer] = 4444;
				}
				Time startApp = (NanoSeconds (150) + MicroSeconds(rand_range(50, 100)));
				Ptr<Node> rxNode = servers[incastLeaf].Get (incastServer);
				Ptr<Ipv4> ipv4 = rxNode->GetObject<Ipv4> ();
				Ipv4InterfaceAddress rxInterface = ipv4->GetAddress (1, 0);
				Ipv4Address rxAddress = rxInterface.GetLocal ();

				InetSocketAddress ad (rxAddress, port);
				Address sinkAddress(ad);
				Ptr<BulkSendApplication> bulksend = CreateObject<BulkSendApplication>();
				bulksend->SetAttribute("Protocol", TypeIdValue(TcpSocketFactory::GetTypeId()));
				bulksend->SetAttribute ("SendSize", UintegerValue (flowSize));
				bulksend->SetAttribute ("MaxBytes", UintegerValue(flowSize));
				bulksend->SetAttribute("FlowId", UintegerValue(flowCount++));
				bulksend->SetAttribute("priorityCustom", UintegerValue(prior));
				bulksend->SetAttribute("Remote", AddressValue(sinkAddress));
				bulksend->SetAttribute("InitialCwnd", UintegerValue (flowSize / PACKET_SIZE + 1));
				bulksend->SetAttribute("priority", UintegerValue(prior));
				bulksend->SetAttribute("sendAt", TimeValue(Seconds (startTime)));
				// printf("startTime INC App: %lf\n", startTime);

				bulksend->SetStartTime (startApp);
				bulksend->SetStopTime (Seconds (END_TIME));
				servers[txLeaf].Get (txServer)->AddApplication(bulksend);

				PacketSinkHelper sink ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), port));
				ApplicationContainer sinkApp = sink.Install (servers[incastLeaf].Get(incastServer));
				sinkApp.Get(0)->SetAttribute("TotalQueryBytes", UintegerValue(flowSize));
				sinkApp.Get(0)->SetAttribute("recvAt", TimeValue(Seconds(startTime)));
				sinkApp.Get(0)->SetAttribute("priority", UintegerValue(0)); // ack packets are prioritized
				sinkApp.Get(0)->SetAttribute("priorityCustom", UintegerValue(0)); // ack packets are prioritized
				sinkApp.Get(0)->SetAttribute("senderPriority", UintegerValue(prior));
				sinkApp.Get(0)->SetAttribute("flowId", UintegerValue(flowCount));
				flowCount += 1;
				sinkApp.Start (startApp);
				sinkApp.Stop (Seconds (END_TIME));
				sinkApp.Get(0)->TraceConnectWithoutContext("FlowFinish", MakeBoundCallback(&TraceMsgFinish, fctOutput));
			}
			startTime += poission_gen_interval (requestRate);
		}
	}
}

void install_applications (int txLeaf, NodeContainer* servers, double requestRate, struct cdf_table *cdfTable,
                           long &flowCount, int SERVER_COUNT, int LEAF_COUNT, double START_TIME, double END_TIME, double FLOW_LAUNCH_END_TIME, int numPrior, bool use_multiprio_thresh)
{
	uint32_t prior;
	if (use_multiprio_thresh)
		prior = rand_range(2, numPrior);
	else
		prior = fixed_prio;

	for (int txServer = 0; txServer < SERVER_COUNT; txServer++)
	{
		double startTime = START_TIME + poission_gen_interval (requestRate);
		while (startTime < FLOW_LAUNCH_END_TIME && startTime > START_TIME)
		{
			// Permutation demand matrix
			int rxLeaf = txLeaf + 1;
			if (rxLeaf == LEAF_COUNT) {
				rxLeaf = 0;
			}
			// int rxLeaf=txLeaf;
			// while (txLeaf==rxLeaf){
			//     rxLeaf = get_target_leaf(LEAF_COUNT);
			// }

			uint32_t rxServer = rand_range(0, SERVER_COUNT);

			uint16_t port = PORT_START[rxLeaf * SERVER_COUNT + rxServer]++;
			if (port > PORT_END) {
				port = 4444;
				PORT_START[rxLeaf * SERVER_COUNT + rxServer] = 4444;
			}

			uint64_t flowSize = gen_random_cdf (cdfTable);
			while (flowSize == 0) {
				flowSize = gen_random_cdf (cdfTable);
			}

			//printf("flowSize: %lu\n", flowSize);

			Ptr<Node> rxNode = servers[rxLeaf].Get (rxServer);
			Ptr<Ipv4> ipv4 = rxNode->GetObject<Ipv4> ();
			Ipv4InterfaceAddress rxInterface = ipv4->GetAddress (1, 0);
			Ipv4Address rxAddress = rxInterface.GetLocal ();

			InetSocketAddress ad (rxAddress, port);
			Address sinkAddress(ad);
			Ptr<BulkSendApplication> bulksend = CreateObject<BulkSendApplication>();
			bulksend->SetAttribute("Protocol", TypeIdValue(TcpSocketFactory::GetTypeId()));
			bulksend->SetAttribute ("SendSize", UintegerValue (flowSize));
			bulksend->SetAttribute ("MaxBytes", UintegerValue(flowSize));
			bulksend->SetAttribute("FlowId", UintegerValue(flowCount++));
			bulksend->SetAttribute("priorityCustom", UintegerValue(prior));
			bulksend->SetAttribute("Remote", AddressValue(sinkAddress));
			bulksend->SetAttribute("InitialCwnd", UintegerValue (50));
			bulksend->SetAttribute("priority", UintegerValue(prior));
			bulksend->SetStartTime (Seconds(startTime));
			// printf("startTime App: %lf\n", startTime);
			bulksend->SetStopTime (Seconds (END_TIME));
			servers[txLeaf].Get (txServer)->AddApplication(bulksend);

			PacketSinkHelper sink ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), port));
			ApplicationContainer sinkApp = sink.Install (servers[rxLeaf].Get(rxServer));
			sinkApp.Get(0)->SetAttribute("TotalQueryBytes", UintegerValue(flowSize));
			sinkApp.Get(0)->SetAttribute("priority", UintegerValue(0)); // ack packets are prioritized
			sinkApp.Get(0)->SetAttribute("priorityCustom", UintegerValue(0)); // ack packets are prioritized
			sinkApp.Get(0)->SetAttribute("flowId", UintegerValue(flowCount));
			sinkApp.Get(0)->SetAttribute("senderPriority", UintegerValue(prior));
			flowCount += 1;
			sinkApp.Start (Seconds(startTime));
			sinkApp.Stop (Seconds (END_TIME));
			sinkApp.Get(0)->TraceConnectWithoutContext("FlowFinish", MakeBoundCallback(&TraceMsgFinish, fctOutput));
			startTime += poission_gen_interval (requestRate);
		}
	}
	// std::cout << "Finished installation of applications from leaf-"<< fromLeafId << std::endl;
}



int
main (int argc, char *argv[])
{
	CommandLine cmd;

	double START_TIME = 10;
	double FLOW_LAUNCH_END_TIME = 13;
	double END_TIME = 20;
	cmd.AddValue ("StartTime", "Start time of the simulation", START_TIME);
	cmd.AddValue ("EndTime", "End time of the simulation", END_TIME);
	cmd.AddValue ("FlowLaunchEndTime", "End time of the flow launch period", FLOW_LAUNCH_END_TIME);

	unsigned randomSeed = 8;
	cmd.AddValue ("randomSeed", "Random seed, 0 for random generated", randomSeed);

	double load = 0.6;
	cmd.AddValue ("load", "Load of the network, 0.0 - 1.0", load);

	uint32_t SERVER_COUNT = 32;
	uint32_t SPINE_COUNT = 2;
	uint32_t LEAF_COUNT = 2;
	uint32_t LINK_COUNT = 4;
	uint64_t spineLeafCapacity = 10; //Gbps
	uint64_t leafServerCapacity = 10; //Gbps
	double linkLatency = 10;
	cmd.AddValue ("serverCount", "The Server count", SERVER_COUNT);
	cmd.AddValue ("spineCount", "The Spine count", SPINE_COUNT);
	cmd.AddValue ("leafCount", "The Leaf count", LEAF_COUNT);
	cmd.AddValue ("linkCount", "The Link count", LINK_COUNT);
	cmd.AddValue ("spineLeafCapacity", "Spine <-> Leaf capacity in Gbps", spineLeafCapacity);
	cmd.AddValue ("leafServerCapacity", "Leaf <-> Server capacity in Gbps", leafServerCapacity);
	cmd.AddValue ("linkLatency", "linkLatency in microseconds", linkLatency);

	uint32_t TcpProt = DCTCP;
	cmd.AddValue("TcpProt", "Tcp protocol", TcpProt);

	uint32_t BufferSize = 1000 * 1000 * 9;
	double statBuf = 0; // fraction of buffer that is reserved
	cmd.AddValue ("BufferSize", "BufferSize in Bytes", BufferSize);
	cmd.AddValue ("statBuf", "staticBuffer in fraction of Total buffersize", statBuf);

	uint32_t algorithm = DT;
	cmd.AddValue ("algorithm", "Buffer Management algorithm", algorithm);

	/*RED Parameters*/
	uint32_t RedMinTh = 65;
	uint32_t RedMaxTh = 65;
	uint32_t UseEcn = 0;
	std::string ecnEnabled = "EcnDisabled";
	cmd.AddValue("RedMinTh", "Min Threshold for RED in packets", RedMinTh);
	cmd.AddValue("RedMaxTh", "Max Threshold for RED in packets", RedMaxTh);
	cmd.AddValue("UseEcn", "Ecn Enabled", UseEcn);

	std::string sched = "roundRobin";
	cmd.AddValue ("sched", "scheduling", sched);

	uint32_t requestSize = 0.2 * BufferSize;
	double queryRequestRate = 0; // at each server (per second)
	cmd.AddValue ("request", "Query Size in Bytes", requestSize);
	cmd.AddValue("queryRequestRate", "Query request rate (poisson arrivals)", queryRequestRate);

	uint32_t nPrior = 2; // number queues in switch ports
	cmd.AddValue ("nPrior", "number of priorities", nPrior);

	std::string alphasFile = "/home/halmas3/ns3-datacenter/simulator/ns-3.35/examples/Protean/alphas";
	std::string proteanAlphasFile = "/home/halmas3/ns3-datacenter/simulator/ns-3.35/examples/Protean/protean_alphas";
	std::string cdfFileName = "/home/halmas3/ns3-datacenter/simulator/ns-3.35/examples/Protean/websearch.txt";
	std::string cdfName = "WS";
	cmd.AddValue ("alphasFile", "alpha values file (should be exactly nPrior lines)", alphasFile);
	cmd.AddValue ("proteanAlphasFile", "Protean alpha values file (should be exactly nPrior lines)", proteanAlphasFile);
	cmd.AddValue ("cdfFileName", "File name for flow distribution", cdfFileName);
	cmd.AddValue ("cdfName", "Name for flow distribution", cdfName);

	uint32_t printDelay = 30 * 1e3;
	cmd.AddValue("printDelay", "printDelay in NanoSeconds", printDelay);

	double alphaUpdateInterval = 1;
	cmd.AddValue("alphaUpdateInterval", "(Number of Rtts) update interval for alpha values", alphaUpdateInterval);


	std::string fctOutFile = "./fcts.txt";
	cmd.AddValue ("fctOutFile", "File path for FCTs", fctOutFile);

	std::string algOutFile = "./alg.txt";
	cmd.AddValue ("algOutFile", "File path for FCTs", algOutFile);

	std::string torOutFile = "./tor.txt";
	cmd.AddValue ("torOutFile", "File path for ToR statistic", torOutFile);

	uint32_t rto = 10 * 1000; // in MicroSeconds, 10 milliseconds.
	cmd.AddValue ("rto", "min Retransmission timeout value in MicroSeconds", rto);

	uint32_t torPrintall = 0;
	cmd.AddValue ("torPrintall", "torPrintall", torPrintall);


	double DT_alpha = 1;
	double Protean_beta = 0.0625;
	bool per_packet_Protean = false;
	bool use_max_dqdt = false;
	bool use_normalized_dqdt = false;
	bool use_multi_prio_thresh = true;
	bool use_high_prio_shorts = true;
	double buildup_thresh = 3.75;
	cmd.AddValue ("DT_alpha", "alpha parameter in Dynamic Threshold", DT_alpha);
	cmd.AddValue ("Protean_beta", "beta parameter in Protean EWMA", Protean_beta);
	cmd.AddValue ("buildup_thresh", "Threshold for queue buildup", buildup_thresh);
	cmd.AddValue ("per_packet_Protean", "whether Protean calculations should be based on packets and unit time (as opposed to bytes and absolute time)", per_packet_Protean);
	cmd.AddValue ("useMaxDqDt", "use Max(Dq/Dt s) instead of EWMA", use_max_dqdt);
	cmd.AddValue ("useNormalizedDqDt", "use DqDt/Sum(DqDt) for positive ones instead of raw DqDt", use_normalized_dqdt);
	cmd.AddValue ("useMultiPrioThresh", "use smaller buildup threshold with multiple priorities", use_multi_prio_thresh);
	cmd.AddValue ("useHighPrioShorts", "prioritize short flows", use_high_prio_shorts);
	/*Parse CMD*/
	cmd.Parse (argc, argv);

	fctOutput = asciiTraceHelper.CreateFileStream (fctOutFile);

	*fctOutput->GetStream ()
	        << "time "
	        << "flowsize "
	        << "fct "
	        << "basefct "
	        << "slowdown "
	        << "basertt "
	        <<  "flowstart "
	        << "priority "
	        << "incast "
	        << std::endl;

	torStats = torTraceHelper.CreateFileStream (torOutFile);

	if (!torPrintall) {
		*torStats->GetStream ()
		        << "time "
		        << "tor "
		        << "bufferSizeMB "
		        << "occupiedBufferPct "
		        << "uplinkThroughput "
		        << "priority0 "
		        << "priority1 "
		        << "priority2 "
		        << "priority3 "
		        << "priority4 "
		        << "priority5 "
		        << "priority6 "
		        << "priority7 "
		        << std::endl;
	}
	else {
		*torStats->GetStream()
		        << "time "
		        << "tor "
		        << "portId "
		        << "bufferSizeMB "
		        << "PortOccBuffer "
		        << "PortThroughput "
		        << std::endl;
	}

	// algStats = algTraceHelper.CreateFileStream (algOutFile);

	// *algStats->GetStream ()
	// 	        << "time "
	// 			<< "tor "
	// 	        << "bufferSizeMB "
	// 			<< "TotalUsedBytes "
	// 			<< "RemainingBuff "
	// 			<< "TotalDropCount "
	// 			<< "P1_instDqDt_p1 "
	// 			<< "P1_instDqDt_p2 "
	// 			<< "P1_ewmaDqDt_p1 "
	// 			<< "P1_ewmaDqDt_p2 "
	// 			<< "P1_maxDqDt_p1 "
	// 			<< "P1_maxDqDt_p2 "
	// 			<< "P1_bytesUsed "
	// 			<< "P1_threshold "
	// 			<< "P1_throughput "
	// 			<< "P1_dropCount "
	// 			<< "P2_instDqDt_p1 "
	// 			<< "P2_instDqDt_p2 "
	// 			<< "P2_ewmaDqDt_p1 "
	// 			<< "P2_ewmaDqDt_p2 "
	// 			<< "P2_maxDqDt_p1 "
	// 			<< "P2_maxDqDt_p2 "
	// 			<< "P2_bytesUsed "
	// 			<< "P2_threshold "
	// 			<< "P2_throughput "
	// 			<< "P2_dropCount "
	// 			<< "P3_instDqDt_p1 "
	// 			<< "P3_instDqDt_p2 "
	// 			<< "P3_ewmaDqDt_p1 "
	// 			<< "P3_ewmaDqDt_p2 "
	// 			<< "P3_maxDqDt_p1 "
	// 			<< "P3_maxDqDt_p2 "
	// 			<< "P3_bytesUsed "
	// 			<< "P3_threshold "
	// 			<< "P3_throughput "
	// 			<< "P3_dropCount "
	// 			<< "P4_instDqDt_p1 "
	// 			<< "P4_instDqDt_p2 "
	// 			<< "P4_ewmaDqDt_p1 "
	// 			<< "P4_ewmaDqDt_p2 "
	// 			<< "P4_maxDqDt_p1 "
	// 			<< "P4_maxDqDt_p2 "
	// 			<< "P4_bytesUsed "
	// 			<< "P4_threshold "
	// 			<< "P4_throughput "
	// 			<< "P4_dropCount "
	//         	<< std::endl;

	uint32_t staticBuffer = (double) BufferSize * statBuf / (SERVER_COUNT + SPINE_COUNT * LINK_COUNT);
	BufferSize = BufferSize - staticBuffer; // BufferSize is the buffer pool which is available for sharing
	if (UseEcn) {
		ecnEnabled = "EcnEnabled";
	}
	else {
		ecnEnabled = "EcnDisabled";
	}

	/*Reading alpha values from file*/
	std::string line;
	std::fstream aFile;
	aFile.open(alphasFile);

	uint32_t p = 0;
	while ( getline( aFile, line ) && p < 8 ) { // hard coded to read only 8 alpha values.
		std::istringstream iss( line );
		double a;
		iss >> a;
		alpha_values[p] = a;
		// std::cout << "Alpha-"<< p << " = "<< a << " " << alpha_values[p]<< std::endl;
		p++;
	}
	aFile.close();

	std::string pline;
	std::fstream pFile;
	pFile.open(proteanAlphasFile);
	p = 0;
	while ( getline( pFile, pline ) && p < 8 ) { // hard coded to read only 8 alpha values.
		std::istringstream iss( pline );
		double a;
		iss >> a;
		protean_alpha_values[p] = a;
		// std::cout << "Protean Alpha-"<< p << " = "<< a << " " << protean_alpha_values[p]<< std::endl;
		p++;
	}
	pFile.close();

	uint64_t SPINE_LEAF_CAPACITY = spineLeafCapacity * GIGA;
	uint64_t LEAF_SERVER_CAPACITY = leafServerCapacity * GIGA;
	Time LINK_LATENCY = MicroSeconds(linkLatency);

	double RTTBytes = (LEAF_SERVER_CAPACITY * 1e-6) * linkLatency; // 8 links visited in roundtrip according to the topology, divided by 8 for bytes
	uint32_t RTTPackets = RTTBytes / PACKET_SIZE + 1;
	baseRTTNano = linkLatency * 8 * 1e3;
	nicBw = leafServerCapacity;
	// std::cout << "bandwidth " << spineLeafCapacity << "gbps" <<  " rtt " << linkLatency*8 << "us" << " BDP " << bdp/1e6 << std::endl;

	if (load < 0.0)
	{
		NS_LOG_ERROR ("Illegal Load value");
		return 0;
	}

	Config::SetDefault("ns3::GenQueueDisc::updateInterval", UintegerValue(alphaUpdateInterval * linkLatency * 8 * 1000));
	Config::SetDefault("ns3::GenQueueDisc::staticBuffer", UintegerValue(staticBuffer));
	Config::SetDefault("ns3::GenQueueDisc::BufferAlgorithm", UintegerValue(algorithm));
	Config::SetDefault("ns3::SharedMemoryBuffer::BufferSize", UintegerValue(BufferSize));
	Config::SetDefault ("ns3::FifoQueueDisc::MaxSize", QueueSizeValue (QueueSize ("100MB")));

	TrafficControlHelper tc;
	uint16_t handle;
	TrafficControlHelper::ClassIdList cid;

	/*General TCP Socket settings. Mostly used by various congestion control algorithms in common*/
	Config::SetDefault ("ns3::TcpSocket::ConnTimeout", TimeValue (MilliSeconds (10))); // syn retry interval
	Config::SetDefault ("ns3::TcpSocketBase::MinRto", TimeValue (MicroSeconds (rto)) );  //(MilliSeconds (5))
	Config::SetDefault ("ns3::TcpSocketBase::RTTBytes", UintegerValue ( RTTBytes ));  //(MilliSeconds (5))
	Config::SetDefault ("ns3::TcpSocketBase::ClockGranularity", TimeValue (NanoSeconds (10))); //(MicroSeconds (100))
	Config::SetDefault ("ns3::TcpSocketBase::ReTxThreshold", UintegerValue (3)); //(3)")
	Config::SetDefault ("ns3::RttEstimator::InitialEstimation", TimeValue (MicroSeconds (200))); //TimeValue (MicroSeconds (80))
	Config::SetDefault ("ns3::TcpSocket::SndBufSize", UintegerValue (1073725440)); //1073725440
	Config::SetDefault ("ns3::TcpSocket::RcvBufSize", UintegerValue (1073725440));
	Config::SetDefault ("ns3::TcpSocket::ConnCount", UintegerValue (6));  // Syn retry count
	Config::SetDefault ("ns3::TcpSocketBase::Timestamp", BooleanValue (true));
	Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (PACKET_SIZE));
	Config::SetDefault ("ns3::TcpSocket::DelAckCount", UintegerValue (0));
	Config::SetDefault ("ns3::TcpSocket::PersistTimeout", TimeValue (Seconds (20)));


	/*CC Configuration*/
	switch (TcpProt) {
	case DCTCP:
		Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (ns3::TcpDctcp::GetTypeId()));
		Config::SetDefault ("ns3::RedQueueDisc::UseEcn", BooleanValue (true));
		Config::SetDefault ("ns3::RedQueueDisc::QW", DoubleValue (1.0));
		Config::SetDefault ("ns3::RedQueueDisc::MinTh", DoubleValue (RedMinTh * PACKET_SIZE));
		Config::SetDefault ("ns3::RedQueueDisc::MaxTh", DoubleValue (RedMaxTh * PACKET_SIZE));
		Config::SetDefault ("ns3::RedQueueDisc::MaxSize", QueueSizeValue (QueueSize ("100MB"))); // This is just for initialization. The buffer management algorithm will take care of the rest.
		Config::SetDefault ("ns3::TcpSocketBase::UseEcn", StringValue ("On"));
		Config::SetDefault ("ns3::RedQueueDisc::LInterm", DoubleValue (0.0));
		Config::SetDefault ("ns3::RedQueueDisc::UseHardDrop", BooleanValue (false));
		Config::SetDefault ("ns3::RedQueueDisc::Gentle", BooleanValue (false));
		Config::SetDefault ("ns3::RedQueueDisc::MeanPktSize", UintegerValue (PACKET_SIZE));
		Config::SetDefault ("ns3::Ipv4GlobalRouting::FlowEcmpRouting", BooleanValue(true));
		UseEcn = 1;
		ecnEnabled = "EcnEnabled";
		Config::SetDefault("ns3::GenQueueDisc::nPrior", UintegerValue(nPrior));
		Config::SetDefault("ns3::GenQueueDisc::RoundRobin", UintegerValue(1));
		Config::SetDefault("ns3::GenQueueDisc::StrictPriority", UintegerValue(0));
		handle = tc.SetRootQueueDisc ("ns3::GenQueueDisc");
		cid = tc.AddQueueDiscClasses (handle, nPrior , "ns3::QueueDiscClass");
		for (uint32_t num = 0; num < nPrior; num++) {
			tc.AddChildQueueDisc (handle, cid[num], "ns3::RedQueueDisc", "MinTh", DoubleValue (RedMinTh * PACKET_SIZE), "MaxTh", DoubleValue (RedMaxTh * PACKET_SIZE));
		}
		break;
	case CUBIC:
		Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (ns3::TcpCubic::GetTypeId()));
		Config::SetDefault ("ns3::Ipv4GlobalRouting::FlowEcmpRouting", BooleanValue(true));
		Config::SetDefault("ns3::GenQueueDisc::nPrior", UintegerValue(nPrior));
		Config::SetDefault("ns3::GenQueueDisc::RoundRobin", UintegerValue(1));
		Config::SetDefault("ns3::GenQueueDisc::StrictPriority", UintegerValue(0));
		handle = tc.SetRootQueueDisc ("ns3::GenQueueDisc");
		cid = tc.AddQueueDiscClasses (handle, nPrior , "ns3::QueueDiscClass");
		for (uint32_t num = 0; num < nPrior; num++) {
			tc.AddChildQueueDisc (handle, cid[num], "ns3::FifoQueueDisc");
		}
		break;
	default:
		std::cout << "Error in CC configuration" << std::endl;
		return 0;
	}

	NodeContainer spines;
	spines.Create (SPINE_COUNT);
	NodeContainer leaves;
	leaves.Create (LEAF_COUNT);
	NodeContainer servers[LEAF_COUNT];
	Ipv4InterfaceContainer serverIpv4[LEAF_COUNT];

	// std::string TcpMixNames[3] = {"ns3::TcpCubic", "ns3::TcpDctcp", "ns3::TcpWien"};
	// uint32_t mixRatio[2]={cubicMix,dctcpMix};
	for (uint32_t i = 0; i < LEAF_COUNT; i++) {
		servers[i].Create (SERVER_COUNT);
		// if (multiQueueExp){
		// 	for (uint32_t k = 0; k < 3; k++){
		// 		for (uint32_t j = 0; j < SERVER_COUNT/3; j++){
		// 			TypeId tid = TypeId::LookupByName (TcpMixNames[k]);
		// 			std::stringstream nodeId;
		// 			nodeId << servers[i].Get (j)->GetId ();
		// 			std::string specificNode = "/NodeList/" + nodeId.str () + "/$ns3::TcpL4Protocol/SocketType";
		// 			Config::Set (specificNode, TypeIdValue (tid));
		// 		}
		// 	}
		// }
	}

	InternetStackHelper internet;
	Ipv4GlobalRoutingHelper globalRoutingHelper;
	internet.SetRoutingHelper (globalRoutingHelper);

	internet.Install(spines);
	internet.Install(leaves);
	for (uint32_t i = 0; i < LEAF_COUNT; i++) {
		internet.Install(servers[i]);
	}

	PointToPointHelper p2p;
	Ipv4AddressHelper ipv4;
	Ipv4InterfaceContainer serverNics[LEAF_COUNT][SERVER_COUNT];

	for (uint32_t leaf = 0; leaf < LEAF_COUNT; leaf++) {
		sharedMemoryLeaf[leaf] = CreateObject<SharedMemoryBuffer>();
		sharedMemoryLeaf[leaf]->SetAttribute("BufferSize", UintegerValue(BufferSize));
		sharedMemoryLeaf[leaf]->SetSharedBufferSize(BufferSize);
	}

	Ptr<SharedMemoryBuffer> sharedMemorySpine[SPINE_COUNT];
	for (uint32_t spine = 0; spine < SPINE_COUNT; spine++) {
		sharedMemorySpine[spine] = CreateObject<SharedMemoryBuffer>();
		sharedMemorySpine[spine]->SetAttribute("BufferSize", UintegerValue(BufferSize));
		sharedMemorySpine[spine]->SetSharedBufferSize(BufferSize);
	}

	uint32_t leafPortId[LEAF_COUNT] = {0};
	uint32_t spinePortId[SPINE_COUNT] = {0};


	/*Server <--> Leaf*/
	ipv4.SetBase ("10.1.0.0", "255.255.252.0");
	p2p.SetDeviceAttribute ("DataRate", DataRateValue (DataRate (LEAF_SERVER_CAPACITY)));
	p2p.SetChannelAttribute ("Delay", TimeValue(LINK_LATENCY));
	// p2p.SetQueue ("ns3::DropTailQueue", "MaxSize", StringValue ("10p"));
	for (uint32_t leaf = 0; leaf < LEAF_COUNT; leaf++) {
		ipv4.NewNetwork ();
		for (uint32_t server = 0; server < SERVER_COUNT; server++) {
			NodeContainer nodeContainer = NodeContainer (leaves.Get (leaf), servers[leaf].Get (server));
			NetDeviceContainer netDeviceContainer = p2p.Install (nodeContainer);
			QueueDiscContainer queueDiscs;
			queueDiscs = tc.Install(netDeviceContainer.Get(0));
			ToRQueueDiscs[leaf].Add(queueDiscs.Get(0));
			Ptr<GenQueueDisc> genDisc = DynamicCast<GenQueueDisc> (queueDiscs.Get (0));
			genDisc->SetPortId(leafPortId[leaf]++);
			switch (algorithm) {
			case DT:
				genDisc->setNPrior(nPrior); // IMPORTANT. This will also trigger "alphas = new ..."
				genDisc->setPortBw(leafServerCapacity);
				genDisc->SetSharedMemory(sharedMemoryLeaf[leaf]);
				genDisc->SetBufferAlgorithm(DT);
				for (uint32_t n = 0; n < nPrior; n++) {
					genDisc->alphas[n] = alpha_values[n];
				}
				break;
			case CS:
				genDisc->setNPrior(nPrior); // IMPORTANT. This will also trigger "alphas = new ..."
				genDisc->setPortBw(leafServerCapacity);
				genDisc->SetSharedMemory(sharedMemoryLeaf[leaf]);
				genDisc->SetBufferAlgorithm(CS);
				for (uint32_t n = 0; n < nPrior; n++) {
					genDisc->alphas[n] = alpha_values[n];
				}
				break;
			case PROTEAN:
				genDisc->setNPrior(nPrior); // IMPORTANT. This will also trigger "alphas = new ..."
				genDisc->setPortBw(leafServerCapacity);
				genDisc->SetSharedMemory(sharedMemoryLeaf[leaf]);
				genDisc->SetBufferAlgorithm(PROTEAN);
				genDisc->ConfigDTAlpha(DT_alpha);
				genDisc->ConfigProteanBeta(Protean_beta);
				genDisc->ConfigBuildupThresh(buildup_thresh);
				genDisc->UsePerPacketProtean(per_packet_Protean);
				genDisc->UseMaxDqDt(use_max_dqdt);
				genDisc->UseNormalizedDqDt(use_normalized_dqdt);
				genDisc->UseMultiPrioThresh(use_multi_prio_thresh);
				genDisc->UseHighPrioShorts(use_high_prio_shorts);
				for (uint32_t n = 0; n < nPrior; n++) {
					genDisc->alphas[n] = alpha_values[n];
					genDisc->protean_alphas[n] = protean_alpha_values[n];
				}
				break;
			case FAB:
				genDisc->setNPrior(nPrior); // IMPORTANT. This will also trigger "alphas = new ..."
				genDisc->setPortBw(leafServerCapacity);
				genDisc->SetSharedMemory(sharedMemoryLeaf[leaf]);
				genDisc->SetBufferAlgorithm(FAB);
				genDisc->SetFabWindow(MicroSeconds(5000));
				genDisc->SetFabThreshold(15 * PACKET_SIZE);
				for (uint32_t n = 0; n < nPrior; n++) {
					genDisc->alphas[n] = alpha_values[n];
				}
				break;
			case IB:
				genDisc->setNPrior(nPrior); // IMPORTANT. This will also trigger "alphas = new ..."
				genDisc->setPortBw(leafServerCapacity);
				genDisc->SetSharedMemory(sharedMemoryLeaf[leaf]);
				genDisc->SetBufferAlgorithm(IB);
				genDisc->SetAfdWindow(MicroSeconds(50));
				genDisc->SetDppWindow(MicroSeconds(5000));
				genDisc->SetDppThreshold(RTTPackets);
				for (uint32_t n = 0; n < nPrior; n++) {
					genDisc->alphas[n] = alpha_values[n];
					genDisc->SetQrefAfd(n, uint32_t(RTTBytes));
				}
				break;
			default:
				std::cout << "Error in buffer management configuration. Exiting!";
				return 0;
			}
			serverNics[leaf][server] = ipv4.Assign(netDeviceContainer.Get(1));
			ipv4.Assign(netDeviceContainer.Get(0));
		}
	}

	std::vector<InetSocketAddress> clients[LEAF_COUNT];
	for (uint32_t leaf = 0; leaf < LEAF_COUNT; leaf++) {
		for (uint32_t leafnext = 0; leafnext < LEAF_COUNT ; leafnext++) {
			if (leaf == leafnext) {
				continue;
			}
			for (uint32_t server = 0; server < SERVER_COUNT; server++) {
				clients[leaf].push_back(InetSocketAddress (serverNics[leafnext][server].GetAddress (0), 1000 + leafnext * LEAF_COUNT + server));
			}
		}
	}

	/*Leaf <--> Spine*/
	p2p.SetDeviceAttribute ("DataRate", DataRateValue (DataRate (SPINE_LEAF_CAPACITY)));
	p2p.SetChannelAttribute ("Delay", TimeValue(LINK_LATENCY));
	// p2p.SetQueue ("ns3::DropTailQueue", "MaxSize", StringValue ("10p"));
	for (uint32_t leaf = 0; leaf < LEAF_COUNT; leaf++) {
		for (uint32_t spine = 0; spine < SPINE_COUNT; spine++) {
			for (uint32_t link = 0; link < LINK_COUNT; link++) {
				ipv4.NewNetwork ();
				NodeContainer nodeContainer = NodeContainer (leaves.Get (leaf), spines.Get (spine));
				NetDeviceContainer netDeviceContainer = p2p.Install (nodeContainer);
				QueueDiscContainer queueDiscs = tc.Install(netDeviceContainer);
				northQueues[leaf].Add(queueDiscs.Get(0));
				ToRQueueDiscs[leaf].Add(queueDiscs.Get(0));
				Ptr<GenQueueDisc> genDisc[2];
				genDisc[0] = DynamicCast<GenQueueDisc> (queueDiscs.Get (0));
				genDisc[0]->SetSharedMemory(sharedMemoryLeaf[leaf]);
				genDisc[1] = DynamicCast<GenQueueDisc> (queueDiscs.Get (1));
				genDisc[1]->SetSharedMemory(sharedMemorySpine[spine]);
				genDisc[0]->SetPortId(leafPortId[leaf]++);
				if (leaf == 0){
					//printf("PortID: %d\n", genDisc[0]->getPortId());
				}
				genDisc[1]->SetPortId(spinePortId[spine]++);
				for (uint32_t i = 0; i < 2; i++) {
					switch (algorithm) {
					case DT:
						genDisc[i]->setNPrior(nPrior); // IMPORTANT. This will also trigger "alphas = new ..."
						genDisc[i]->setPortBw(leafServerCapacity);
						genDisc[i]->SetSharedMemory(sharedMemoryLeaf[leaf]);
						genDisc[i]->SetBufferAlgorithm(DT);
						for (uint32_t n = 0; n < nPrior; n++) {
							genDisc[i]->alphas[n] = alpha_values[n];
						}
						break;
					case CS:
						genDisc[i]->setNPrior(nPrior); // IMPORTANT. This will also trigger "alphas = new ..."
						genDisc[i]->setPortBw(leafServerCapacity);
						genDisc[i]->SetSharedMemory(sharedMemoryLeaf[leaf]);
						genDisc[i]->SetBufferAlgorithm(CS);
						for (uint32_t n = 0; n < nPrior; n++) {
							genDisc[i]->alphas[n] = alpha_values[n];
						}
						break;
					case PROTEAN:
						genDisc[i]->setNPrior(nPrior); // IMPORTANT. This will also trigger "alphas = new ..."
						genDisc[i]->setPortBw(leafServerCapacity);
						genDisc[i]->SetSharedMemory(sharedMemoryLeaf[leaf]);
						genDisc[i]->SetBufferAlgorithm(PROTEAN);
						genDisc[i]->ConfigDTAlpha(DT_alpha);
						genDisc[i]->ConfigProteanBeta(Protean_beta);
						genDisc[i]->ConfigBuildupThresh(buildup_thresh);
						genDisc[i]->UsePerPacketProtean(per_packet_Protean);
						genDisc[i]->UseMaxDqDt(use_max_dqdt);
						genDisc[i]->UseNormalizedDqDt(use_normalized_dqdt);
						genDisc[i]->UseMultiPrioThresh(use_multi_prio_thresh);
						genDisc[i]->UseHighPrioShorts(use_high_prio_shorts);
						for (uint32_t n = 0; n < nPrior; n++) {
							genDisc[i]->alphas[n] = alpha_values[n];
							genDisc[i]->protean_alphas[n] = protean_alpha_values[n];

						}
						break;
					case FAB:
						genDisc[i]->setNPrior(nPrior); // IMPORTANT. This will also trigger "alphas = new ..."
						genDisc[i]->setPortBw(leafServerCapacity);
						genDisc[i]->SetSharedMemory(sharedMemoryLeaf[leaf]);
						genDisc[i]->SetBufferAlgorithm(FAB);
						genDisc[i]->SetFabWindow(MicroSeconds(5000));
						genDisc[i]->SetFabThreshold(15 * PACKET_SIZE);
						for (uint32_t n = 0; n < nPrior; n++) {
							genDisc[i]->alphas[n] = alpha_values[n];
						}
						break;
					case IB:
						genDisc[i]->setNPrior(nPrior); // IMPORTANT. This will also trigger "alphas = new ..."
						genDisc[i]->setPortBw(leafServerCapacity);
						genDisc[i]->SetSharedMemory(sharedMemoryLeaf[leaf]);
						genDisc[i]->SetBufferAlgorithm(IB);
						genDisc[i]->SetAfdWindow(MicroSeconds(50));
						genDisc[i]->SetDppWindow(MicroSeconds(5000));
						genDisc[i]->SetDppThreshold(RTTPackets);
						for (uint32_t n = 0; n < nPrior; n++) {
							genDisc[i]->alphas[n] = alpha_values[n];
							genDisc[i]->SetQrefAfd(n, uint32_t(RTTBytes));
						}
						break;
					default:
						std::cout << "Error in buffer management configuration. Exiting!";
						return 0;
					}
				}
				Ipv4InterfaceContainer interfaceContainer = ipv4.Assign (netDeviceContainer);
			}
		}
	}
	double oversubRatio = static_cast<double>(SERVER_COUNT * LEAF_SERVER_CAPACITY) / (SPINE_LEAF_CAPACITY * SPINE_COUNT * LINK_COUNT);
	NS_LOG_INFO ("Over-subscription ratio: " << oversubRatio);
	NS_LOG_INFO ("Initialize CDF table");
	struct cdf_table* cdfTable = new cdf_table ();
	init_cdf (cdfTable);
	load_cdf (cdfTable, cdfFileName.c_str ());
	NS_LOG_INFO ("Calculating request rate");
	double requestRate = load * LEAF_SERVER_CAPACITY * SERVER_COUNT / oversubRatio / (8 * avg_cdf (cdfTable)) / SERVER_COUNT;
	printf("Request rate: %lf\n", requestRate);
	NS_LOG_INFO ("Average request rate: " << requestRate << " per second");
	NS_LOG_INFO ("Initialize random seed: " << randomSeed);
	if (randomSeed == 0)
	{
		srand ((unsigned)time (NULL));
	}
	else
	{
		srand (randomSeed);
	}
	double QUERY_START_TIME = START_TIME;

	long flowCount = 0;

	for (uint32_t i = 0; i < SERVER_COUNT * LEAF_COUNT; i++)
	{
		PORT_START[i] = 4444;
	}

	for (int fromLeafId = 0; fromLeafId < LEAF_COUNT; fromLeafId ++)
	{
		install_applications(fromLeafId, servers, requestRate, cdfTable, flowCount, SERVER_COUNT, LEAF_COUNT, START_TIME, END_TIME, FLOW_LAUNCH_END_TIME, nPrior, use_multi_prio_thresh);
		if (queryRequestRate > 0 && requestSize > 0) {
			install_applications_incast(fromLeafId, servers, queryRequestRate, requestSize, cdfTable, flowCount, SERVER_COUNT, LEAF_COUNT, QUERY_START_TIME, END_TIME, FLOW_LAUNCH_END_TIME, nPrior, use_multi_prio_thresh);
		}
	}
	printf("Number of flows: %ld\n", flowCount / 2);
	if (!torPrintall) {
		Simulator::Schedule(Seconds(START_TIME), InvokeToRStats, torStats, BufferSize, 0, printDelay);
	}
	else {
		Simulator::Schedule(Seconds(START_TIME), InvokePerPortToRStats, torStats, BufferSize, 0, printDelay);
	}
	if (algorithm == PROTEAN){
		// Simulator::Schedule(Seconds(START_TIME), InvokeAlgStats, algStats, BufferSize, 0, printDelay);
	}

	// AsciiTraceHelper ascii;
//    p2p.EnableAsciiAll (ascii.CreateFileStream ("eval.tr"));
	Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
	// NS_LOG_UNCOND("Running the Simulation...!");
	std::cout << "Running the Simulation...!" << std::endl;
	Simulator::Stop (Seconds (END_TIME));
	Simulator::Run ();
	Simulator::Destroy ();
	free_cdf (cdfTable);
	return 0;
}
