
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
# define PROTEAN 114

/*Congestion Control Algorithms*/
# define DCTCP 2
# define CUBIC 1

#define incast_prio 1
#define long_prio 2
#define fixed_prio 3

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
	QueueDiscContainer queues = ToRQueueDiscs[leafId];
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
InvokeAlgStats(Ptr<OutputStreamWrapper> stream, uint32_t BufferSize, uint32_t leafId, double nanodelay, uint32_t incastDegree1, uint32_t incastDegree2) {
	Ptr<SharedMemoryBuffer> sm = sharedMemoryLeaf[leafId];
	QueueDiscContainer queues = ToRQueueDiscs[leafId];
	Ptr<GenQueueDisc> p1, p2;
	for (uint32_t i = 0; i < queues.GetN(); i++) {
		Ptr<GenQueueDisc> genDisc = DynamicCast<GenQueueDisc>(queues.Get(i));
		// printf("%d\n", genDisc->getPortId());
		if (genDisc->getPortId() == (incastDegree1 + incastDegree2 + 1)) {
			//Port 1 stats here
			p1 = genDisc;

		}
		else if (genDisc->getPortId() == (incastDegree1 + incastDegree2)) {
			//Port 2 stats here
			p2 = genDisc;
		}
	}

	*stream->GetStream()
	        << Simulator::Now().GetNanoSeconds()
	        << " " << leafId
	        << " " << double(BufferSize) / 1e6
			<< " " << 100 * double(sm->GetOccupiedBuffer()) / BufferSize
			<< " " << 100 * double(sm->GetRemainingBuffer()) / BufferSize
			<< " " << sm->GetTotalDroppedPacketCount()
			<< " " << p1->getInstDqDt(incast_prio)
			<< " " << p1->getEWMADqDt(incast_prio)
			<< " " << p1->getMaxDqDt(incast_prio)
			<< " " << p1->GetQueueDiscClass (incast_prio)->GetQueueDisc ()->GetNBytes()
			<< " " << p1->getThresh (incast_prio)
			<< " " << 100 * p1->GetThroughputPort(nanodelay)
			<< " " << sm->GetDroppedPacketCount(p1->getPortId(), incast_prio)
			<< " " << p2->getInstDqDt(long_prio)
			<< " " << p2->getEWMADqDt(long_prio)
			<< " " << p2->getMaxDqDt(long_prio)
			<< " " << p2->GetQueueDiscClass (long_prio)->GetQueueDisc ()->GetNBytes()
			<< " " << p2->getThresh (long_prio)
			<< " " << 100 * p2->GetThroughputPort(nanodelay)
			<< " " << sm->GetDroppedPacketCount(p2->getPortId(), long_prio)
	        << std::endl;

	Simulator::Schedule(NanoSeconds(nanodelay), InvokeAlgStats, stream, BufferSize, leafId, nanodelay, incastDegree1, incastDegree2);
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

void install_applications (int txLeaf, int incastDegree1, int incastDegree2, NodeContainer* servers, double requestRate, struct cdf_table *cdfTable,
                           long &flowCount, int SERVER_COUNT, int LEAF_COUNT, double START_TIME, double END_TIME, double FLOW_LAUNCH_END_TIME, int numPrior)
{
	for (int txServer = 0; txServer < incastDegree1 + incastDegree2 ; txServer++) {
		uint64_t flowSize;


		uint32_t prior;
		Time startTime;
		//printf("start time with jitter: %lf\n", startTime.GetSeconds());
		uint32_t rxServer;
		uint16_t port;
		uint64_t cwnd;
		if (txServer < incastDegree1){
			rxServer = SERVER_COUNT - 1;
			port = rand_range(2222, 8888);
			startTime = Seconds(START_TIME) + MicroSeconds(2000);
			flowSize = 100000;
			cwnd = 72;
			prior = incast_prio;
		}
		else {
			rxServer = SERVER_COUNT - 2;
			port = rand_range(11111, 22222);
			startTime = Seconds(START_TIME);
			flowSize = 100000000;
			cwnd = 1000;
			prior = long_prio;
		}

		//printf("flowSize: %lu\n", flowSize);

		Ptr<Node> rxNode = servers[0].Get (rxServer);
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
		bulksend->SetAttribute("InitialCwnd", UintegerValue (cwnd));
		bulksend->SetAttribute("priority", UintegerValue(prior));
		bulksend->SetStartTime (startTime);
		// printf("startTime App: %lf\n", startTime);
		bulksend->SetStopTime (Seconds (END_TIME));
		servers[0].Get (txServer)->AddApplication(bulksend);

		PacketSinkHelper sink ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), port));
		ApplicationContainer sinkApp = sink.Install (servers[0].Get(rxServer));
		sinkApp.Get(0)->SetAttribute("TotalQueryBytes", UintegerValue(flowSize));
		sinkApp.Get(0)->SetAttribute("priority", UintegerValue(0)); // ack packets are prioritized
		sinkApp.Get(0)->SetAttribute("priorityCustom", UintegerValue(0)); // ack packets are prioritized
		sinkApp.Get(0)->SetAttribute("flowId", UintegerValue(flowCount));
		sinkApp.Get(0)->SetAttribute("senderPriority", UintegerValue(prior));
		flowCount += 1;
		sinkApp.Start (startTime);
		sinkApp.Stop (Seconds (END_TIME));
		sinkApp.Get(0)->TraceConnectWithoutContext("FlowFinish", MakeBoundCallback(&TraceMsgFinish, fctOutput));

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

	uint32_t printDelay = 1000; // 30 * 1e3;
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
	double Protean_beta = 0.25;
	bool per_packet_Protean = false;
	uint32_t incastDegree1 = 16;
	uint32_t incastDegree2 = 2;
	bool use_max_dqdt = false;
	bool use_normalized_dqdt = false;
	bool use_multi_prio_thresh = true;
	bool use_high_prio_shorts = true;
	double buildup_thresh = 3.75;

	cmd.AddValue ("DT_alpha", "alpha parameter in Dynamic Threshold", DT_alpha);
	cmd.AddValue ("Protean_beta", "beta parameter in Protean EWMA", Protean_beta);
	cmd.AddValue ("buildup_thresh", "Threshold for queue buildup", buildup_thresh);
	cmd.AddValue ("per_packet_Protean", "whether Protean calculations should be based on packets and unit time (as opposed to bytes and absolute time)", per_packet_Protean);
	cmd.AddValue ("incastDegree1", "incast degree for receiver 1", incastDegree1);
	cmd.AddValue ("incastDegree2", "incast degree for receiver 2", incastDegree2);
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

	algStats = algTraceHelper.CreateFileStream (algOutFile);

	*algStats->GetStream ()
		        << "time "
				<< "tor "
		        << "bufferSizeMB "
				<< "TotalUsedBytes "
				<< "RemainingBuff "
				<< "TotalDropCount "
				<< "P1_instDqDt "
				<< "P1_ewmaDqDt "
				<< "P1_maxDqDt "
				<< "P1_bytesUsed "
				<< "P1_threshold "
				<< "P1_throughput "
				<< "P1_dropCount "
				<< "P2_instDqDt "
				<< "P2_ewmaDqDt "
				<< "P2_maxDqDt "
				<< "P2_bytesUsed "
				<< "P2_threshold "
				<< "P2_throughput "
				<< "P2_dropCount "
	        	<< std::endl;

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

	double RTTBytes = (LEAF_SERVER_CAPACITY * 1e-6) * linkLatency * 4 / 8;
	uint32_t RTTPackets = RTTBytes / PACKET_SIZE + 1;
	baseRTTNano = linkLatency * 4 * 1e3;
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
	// Config::SetDefault ("ns3::TcpSocket::PersistTimeout", TimeValue (Seconds (20)));


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
	NodeContainer leaves;
	leaves.Create (LEAF_COUNT);
	NodeContainer servers[LEAF_COUNT];
	Ipv4InterfaceContainer serverIpv4[LEAF_COUNT];

	// std::string TcpMixNames[3] = {"ns3::TcpCubic", "ns3::TcpDctcp", "ns3::TcpWien"};
	// uint32_t mixRatio[2]={cubicMix,dctcpMix};
	for (uint32_t i = 0; i < LEAF_COUNT; i++) {
		servers[i].Create (SERVER_COUNT);
	}

	InternetStackHelper internet;
	Ipv4GlobalRoutingHelper globalRoutingHelper;
	internet.SetRoutingHelper (globalRoutingHelper);

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

	uint32_t leafPortId[LEAF_COUNT] = {0};

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

	// double oversubRatio = static_cast<double>(SERVER_COUNT * LEAF_SERVER_CAPACITY) / (SPINE_LEAF_CAPACITY * SPINE_COUNT * LINK_COUNT);

	// NS_LOG_INFO ("Over-subscription ratio: " << oversubRatio);
	// NS_LOG_INFO ("Initialize CDF table");
	struct cdf_table* cdfTable = new cdf_table ();
	init_cdf (cdfTable);
	load_cdf (cdfTable, cdfFileName.c_str ());
	NS_LOG_INFO ("Calculating request rate");
	double requestRate = load * LEAF_SERVER_CAPACITY * SERVER_COUNT / (8 * avg_cdf (cdfTable)) / SERVER_COUNT;
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

	install_applications(0, incastDegree1, incastDegree2, servers, requestRate, cdfTable, flowCount, SERVER_COUNT, LEAF_COUNT, START_TIME, END_TIME, FLOW_LAUNCH_END_TIME, nPrior);
	printf("FlowCount: %ld\n", flowCount/2);
	if (!torPrintall) {
		Simulator::Schedule(Seconds(START_TIME), InvokeToRStats, torStats, BufferSize, 0, printDelay);
	}
	else {
		Simulator::Schedule(Seconds(START_TIME), InvokePerPortToRStats, torStats, BufferSize, 0, printDelay);
	}

	Simulator::Schedule(Seconds(START_TIME), InvokeAlgStats, algStats, BufferSize, 0, printDelay, incastDegree1, incastDegree2);


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
