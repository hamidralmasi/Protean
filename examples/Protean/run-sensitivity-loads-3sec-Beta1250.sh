source config.sh
DIR="$NS3/examples/Protean"
DUMP_DIR="$DIR/Sensitivity-Load-3sec/results"
mkdir -p $DUMP_DIR

DT=101
CS=103
FAB=102
IB=104
PROTEAN=114

CUBIC=1
DCTCP=2

BUF_ALGS=($DT $CS $PROTEAN)
TCP_ALGS=($DCTCP)

# SERVERS=32
# LEAVES=2
# SPINES=2
# LINKS=4
# SERVER_LEAF_CAP=10
# LEAF_SPINE_CAP=10
# LATENCY=10

# # #MULTI 12
SERVERS=16
LEAVES=4
SPINES=4
LINKS=1
SERVER_LEAF_CAP=10
LEAF_SPINE_CAP=10
LATENCY=10

RED_MIN=65
RED_MAX=65


ALPHAFILE="$DIR/alphas"
PROTEAN_ALPHAFILE="$DIR/protean_alphas"
CDFFILE="$DIR/websearch.txt"
CDFNAME="WS"

ALPHA_UPDATE_INT=1 # 1 RTT

STATIC_BUFFER=0
# BUFFER=$(( 1000*1000*9  ))
BUFFER_PER_PORT_PER_GBPS=9.6 # https://baiwei0427.github.io/papers/bcc-ton.pdf (Trident 2)
BUFFER=$(python3 -c "print(int($BUFFER_PER_PORT_PER_GBPS*1024*($SERVERS+$LINKS*$SPINES)*$SERVER_LEAF_CAP))")
TCP=$DCTCP

START_TIME=10
FLOW_END_TIME=13
END_TIME=20


cd $NS3


N=0



N_PRIO=3
BURST_SIZES=0.5
BURST_SIZE=$(python3 -c "print($BURST_SIZES*$BUFFER)")
BURST_FREQ=2

DT_ALPHA=50
PROTEAN_BETA=1
PER_PACKET_PROTEAN=0
USE_MAX_DQDT=0
USE_NORMALIZED_DQDT=0
USE_MULTI_PRIO=0
USE_HIGH_PRIO_SHORTS=0
BUILDUP_THRESH=3.75
# Betas: 0.001 0.005 0.01 0.05 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 0.95 0.99
# Load 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9

for BUFFER_PER_PORT_PER_GBPS in 9.6;do
	BUFFER=$(python3 -c "print(int($BUFFER_PER_PORT_PER_GBPS*1024*($SERVERS+$LINKS*$SPINES)*$SERVER_LEAF_CAP))")
	for TCP in $DCTCP;do
		for USE_HIGH_PRIO_SHORTS in 1;do
			for USE_MULTI_PRIO in 1; do
				for BURST_SIZES in 0.5;do
					for LOAD in 0.2 0.4 0.6 0.8;do
						# for USE_MAX_DQDT in 0 1; do
							# for USE_NORMALIZED_DQDT in 0 1;do
								for PROTEAN_BETA in 0.125;do
									for BURST_FREQ in 1 2 3;do
										for BUILDUP_THRESH in 0.625 1.25 2.5 3.75 5;do
											for ALG in $PROTEAN;do
												BURST_SIZE=$(python3 -c "print($BURST_SIZES*$BUFFER)")
												FLOWFILE="$DUMP_DIR/fcts-single-large-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ-$PROTEAN_BETA-$USE_NORMALIZED_DQDT-$USE_MAX_DQDT-$USE_MULTI_PRIO-$USE_HIGH_PRIO_SHORTS-$BUILDUP_THRESH-$BUFFER_PER_PORT_PER_GBPS.fct"
												TORFILE="$DUMP_DIR/tor-single-large-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ-$PROTEAN_BETA-$USE_NORMALIZED_DQDT-$USE_MAX_DQDT-$USE_MULTI_PRIO-$USE_HIGH_PRIO_SHORTS-$BUILDUP_THRESH-$BUFFER_PER_PORT_PER_GBPS.stat"
												ALGFILE="$DUMP_DIR/alg-single-large-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ-$PROTEAN_BETA-$USE_NORMALIZED_DQDT-$USE_MAX_DQDT-$USE_MULTI_PRIO-$USE_HIGH_PRIO_SHORTS-$BUILDUP_THRESH-$BUFFER_PER_PORT_PER_GBPS.stat"
												while [[ $(ps aux | grep evaluation-optimized | wc -l) -gt 72 ]];do
													sleep 30;
													echo "waiting for cores, $N running..."
												done
												N=$(( $N+1 ))
												(./waf --run "protean-evaluation --DT_alpha=$DT_ALPHA --Protean_beta=$PROTEAN_BETA --per_packet_Protean=$PER_PACKET_PROTEAN --load=$LOAD --StartTime=$START_TIME --EndTime=$END_TIME --FlowLaunchEndTime=$FLOW_END_TIME --serverCount=$SERVERS --spineCount=$SPINES --leafCount=$LEAVES --linkCount=$LINKS --spineLeafCapacity=$LEAF_SPINE_CAP --leafServerCapacity=$SERVER_LEAF_CAP --linkLatency=$LATENCY --TcpProt=$TCP --BufferSize=$BUFFER --statBuf=$STATIC_BUFFER --algorithm=$ALG --RedMinTh=$RED_MIN --RedMaxTh=$RED_MAX --request=$BURST_SIZE --queryRequestRate=$BURST_FREQ --nPrior=$N_PRIO --alphasFile=$ALPHAFILE --cdfFileName=$CDFFILE --alphaUpdateInterval=$ALPHA_UPDATE_INT --fctOutFile=$FLOWFILE --torOutFile=$TORFILE --algOutFile=$ALGFILE --useMaxDqDt=$USE_MAX_DQDT --useNormalizedDqDt=$USE_NORMALIZED_DQDT --useMultiPrioThresh=$USE_MULTI_PRIO --useHighPrioShorts=$USE_HIGH_PRIO_SHORTS --buildup_thresh=$BUILDUP_THRESH --proteanAlphasFile=$PROTEAN_ALPHAFILE"; echo "$FLOWFILE")&
												sleep 10
											done
										done
									done
								done
							# done
						# done
					done
				done
			done
		done
	done
done
# for USE_MULTI_PRIO in 0 1; do
# 	for BURST_SIZES in 0.5;do
# 		for LOAD in 0.6;do
# 			for USE_MAX_DQDT in 0 1; do
# 				for USE_NORMALIZED_DQDT in 0 1;do
# 					for PROTEAN_BETA in 0.2;do
# 						for BURST_FREQ in 1;do
# 							for ALG in $PROTEAN;do
# 								FLOWFILE="$DUMP_DIR/fcts-single-large-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ-$PROTEAN_BETA-$USE_NORMALIZED_DQDT-$USE_MAX_DQDT-$USE_MULTI_PRIO.fct"
# 								TORFILE="$DUMP_DIR/tor-single-large-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ-$PROTEAN_BETA-$USE_NORMALIZED_DQDT-$USE_MAX_DQDT-$USE_MULTI_PRIO.stat"
# 								ALGFILE="$DUMP_DIR/alg-single-large-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ-$PROTEAN_BETA-$USE_NORMALIZED_DQDT-$USE_MAX_DQDT-$USE_MULTI_PRIO.stat"
# 								while [[ $(ps aux | grep evaluation-optimized | wc -l) -gt 16 ]];do
# 									sleep 30;
# 									echo "waiting for cores, $N running..."
# 								done
# 								N=$(( $N+1 ))
# 								(./waf --run "protean-evaluation --DT_alpha=$DT_ALPHA --Protean_beta=$PROTEAN_BETA --per_packet_Protean=$PER_PACKET_PROTEAN --load=$LOAD --StartTime=$START_TIME --EndTime=$END_TIME --FlowLaunchEndTime=$FLOW_END_TIME --serverCount=$SERVERS --spineCount=$SPINES --leafCount=$LEAVES --linkCount=$LINKS --spineLeafCapacity=$LEAF_SPINE_CAP --leafServerCapacity=$SERVER_LEAF_CAP --linkLatency=$LATENCY --TcpProt=$TCP --BufferSize=$BUFFER --statBuf=$STATIC_BUFFER --algorithm=$ALG --RedMinTh=$RED_MIN --RedMaxTh=$RED_MAX --request=$BURST_SIZE --queryRequestRate=$BURST_FREQ --nPrior=$N_PRIO --alphasFile=$ALPHAFILE --cdfFileName=$CDFFILE --alphaUpdateInterval=$ALPHA_UPDATE_INT --fctOutFile=$FLOWFILE --torOutFile=$TORFILE --algOutFile=$ALGFILE --useMaxDqDt=$USE_MAX_DQDT --useNormalizedDqDt=$USE_NORMALIZED_DQDT --useMultiPrioThresh=$USE_MULTI_PRIO --proteanAlphasFile=$PROTEAN_ALPHAFILE"; echo "$FLOWFILE")&
# 								sleep 10
# 							done
# 						done
# 					done
# 				done
# 			done
# 		done
# 	done
# done
# sleep 30;

# for USE_MULTI_PRIO in 1; do
# 	for BURST_SIZES in 0.5;do
# 		for LOAD in 0.6;do
# 			for USE_MAX_DQDT in 0; do
# 				for USE_NORMALIZED_DQDT in 0;do
# 					for PROTEAN_BETA in 0.2;do
# 						for BURST_FREQ in 1;do
# 							for ALG in $PROTEAN;do
# 								FLOWFILE="$DUMP_DIR/fcts-single-large-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ-$PROTEAN_BETA-$USE_NORMALIZED_DQDT-$USE_MAX_DQDT-$USE_MULTI_PRIO.fct"
# 								TORFILE="$DUMP_DIR/tor-single-large-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ-$PROTEAN_BETA-$USE_NORMALIZED_DQDT-$USE_MAX_DQDT-$USE_MULTI_PRIO.stat"
# 								ALGFILE="$DUMP_DIR/alg-single-large-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ-$PROTEAN_BETA-$USE_NORMALIZED_DQDT-$USE_MAX_DQDT-$USE_MULTI_PRIO.stat"
# 								while [[ $(ps aux | grep evaluation-optimized | wc -l) -gt 16 ]];do
# 									sleep 30;
# 									echo "waiting for cores, $N running..."
# 								done
# 								N=$(( $N+1 ))
# 								(./waf --run "protean-evaluation --DT_alpha=$DT_ALPHA --Protean_beta=$PROTEAN_BETA --per_packet_Protean=$PER_PACKET_PROTEAN --load=$LOAD --StartTime=$START_TIME --EndTime=$END_TIME --FlowLaunchEndTime=$FLOW_END_TIME --serverCount=$SERVERS --spineCount=$SPINES --leafCount=$LEAVES --linkCount=$LINKS --spineLeafCapacity=$LEAF_SPINE_CAP --leafServerCapacity=$SERVER_LEAF_CAP --linkLatency=$LATENCY --TcpProt=$TCP --BufferSize=$BUFFER --statBuf=$STATIC_BUFFER --algorithm=$ALG --RedMinTh=$RED_MIN --RedMaxTh=$RED_MAX --request=$BURST_SIZE --queryRequestRate=$BURST_FREQ --nPrior=$N_PRIO --alphasFile=$ALPHAFILE --cdfFileName=$CDFFILE --alphaUpdateInterval=$ALPHA_UPDATE_INT --fctOutFile=$FLOWFILE --torOutFile=$TORFILE --algOutFile=$ALGFILE --useMaxDqDt=$USE_MAX_DQDT --useNormalizedDqDt=$USE_NORMALIZED_DQDT --useMultiPrioThresh=$USE_MULTI_PRIO --proteanAlphasFile=$PROTEAN_ALPHAFILE"; echo "$FLOWFILE")&
# 								sleep 10
# 							done
# 						done
# 					done
# 				done
# 			done
# 		done
# 	done
# done

# wait

# for LOAD in 0.6;do
#     for PROTEAN_BETA in 0.2;do
# 		for ALG in ${BUF_ALGS[@]};do
# 			FLOWFILE="$DUMP_DIR/fcts-single-large-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ-$PROTEAN_BETA.fct"
# 			TORFILE="$DUMP_DIR/tor-single-large-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ-$PROTEAN_BETA.stat"
# 			ALGFILE="$DUMP_DIR/alg-single-large-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ-$PROTEAN_BETA.stat"

# 			while [[ $(ps aux | grep evaluation-optimized | wc -l) -gt 15 ]];do
# 				sleep 30;
# 				echo "waiting for cores, $N running..."
# 			done
# 			N=$(( $N+1 ))
# 			python3 /home/halmas3/ns3-datacenter/simulator/ns-3.35/examples/Protean/plot.py $ALGFILE
# 			sleep 10
# 		done
# 	done
# done
