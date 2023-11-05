source config.sh
DIR="$NS3/examples/Protean"
DUMP_DIR="$DIR/motivation"
mkdir -p $DUMP_DIR

DT=101
CS=103
PROTEAN=114

DCTCP=2
CUBIC=1

BUF_ALGS=($DT $CS $PROTEAN)
TCP_ALGS=($CUBIC $DCTCP)

# #MULTI 12
SERVERS=20
LEAVES=1
SPINES=2
LINKS=1
SERVER_LEAF_CAP=10
LEAF_SPINE_CAP=10
LATENCY=10
INCAST_DEGREE1=16
INCAST_DEGREE2=2
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

BUFFER=3000000
echo "Buffer: $BUFFER"
TCP=$CUBIC

START_TIME=10
FLOW_END_TIME=10.1
END_TIME=11
# END_TIME=10.015


cd $NS3


N=0


N_PRIO=8
BURST_SIZES=0.3
BURST_SIZE=$(python3 -c "print($BURST_SIZES*$BUFFER)")
BURST_FREQ=1

DT_ALPHA=50
PROTEAN_BETA=0.125
PER_PACKET_PROTEAN=0
USE_MAX_DQDT=0
USE_NORMALIZED_DQDT=0
USE_MULTI_PRIO=1
USE_HIGH_PRIO_SHORTS=1
BUILDUP_THRESH=2.25
# 0.001 0.005 0.01 0.05 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 0.95 0.99
LOAD=0.6
# for ALG in $DT $PROTEAN;do
# 	FLOWFILE="$DUMP_DIR/fcts-single-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ-$PROTEAN_BETA-$USE_NORMALIZED_DQDT-$USE_MAX_DQDT-$USE_MULTI_PRIO-$BUILDUP_THRESH.fct"
# 	TORFILE="$DUMP_DIR/tor-single-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ-$PROTEAN_BETA-$USE_NORMALIZED_DQDT-$USE_MAX_DQDT-$USE_MULTI_PRIO-$BUILDUP_THRESH.stat"
# 	ALGFILE="$DUMP_DIR/alg-single-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ-$PROTEAN_BETA-$USE_NORMALIZED_DQDT-$USE_MAX_DQDT-$USE_MULTI_PRIO-$BUILDUP_THRESH.stat"

# 	while [[ $(ps aux | grep evaluation-optimized | wc -l) -gt 72 ]];do
# 		sleep 30;
# 		echo "waiting for cores, $N running..."
# 	done
# 	N=$(( $N+1 ))
# 	(./waf --run "protean-evaluation-motivation --DT_alpha=$DT_ALPHA --Protean_beta=$PROTEAN_BETA --per_packet_Protean=$PER_PACKET_PROTEAN --load=$LOAD --StartTime=$START_TIME --EndTime=$END_TIME --FlowLaunchEndTime=$FLOW_END_TIME --serverCount=$SERVERS --spineCount=$SPINES --leafCount=$LEAVES --linkCount=$LINKS --spineLeafCapacity=$LEAF_SPINE_CAP --leafServerCapacity=$SERVER_LEAF_CAP --linkLatency=$LATENCY --TcpProt=$TCP --BufferSize=$BUFFER --statBuf=$STATIC_BUFFER --algorithm=$ALG --RedMinTh=$RED_MIN --RedMaxTh=$RED_MAX --request=$BURST_SIZE --queryRequestRate=$BURST_FREQ --nPrior=$N_PRIO --alphasFile=$ALPHAFILE --cdfFileName=$CDFFILE --alphaUpdateInterval=$ALPHA_UPDATE_INT --fctOutFile=$FLOWFILE --torOutFile=$TORFILE --algOutFile=$ALGFILE --useMaxDqDt=$USE_MAX_DQDT --useNormalizedDqDt=$USE_NORMALIZED_DQDT --useMultiPrioThresh=$USE_MULTI_PRIO --useHighPrioShorts=$USE_HIGH_PRIO_SHORTS --buildup_thresh=$BUILDUP_THRESH --proteanAlphasFile=$PROTEAN_ALPHAFILE --incastDegree1=$INCAST_DEGREE1 --incastDegree2=$INCAST_DEGREE2"; echo "$FLOWFILE")&
# 	sleep 10
# done

# wait


for ALG in $DT $PROTEAN;do
	FLOWFILE="$DUMP_DIR/fcts-single-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ-$PROTEAN_BETA-$USE_NORMALIZED_DQDT-$USE_MAX_DQDT-$USE_MULTI_PRIO-$BUILDUP_THRESH.fct"
	TORFILE="$DUMP_DIR/tor-single-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ-$PROTEAN_BETA-$USE_NORMALIZED_DQDT-$USE_MAX_DQDT-$USE_MULTI_PRIO-$BUILDUP_THRESH.stat"
	ALGFILE="$DUMP_DIR/alg-single-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ-$PROTEAN_BETA-$USE_NORMALIZED_DQDT-$USE_MAX_DQDT-$USE_MULTI_PRIO-$BUILDUP_THRESH.stat"

	# while [[ $(ps aux | grep evaluation-optimized | wc -l) -gt 72 ]];do
	# 	sleep 30;
	# 	echo "waiting for cores, $N running..."
	# done
	N=$(( $N+1 ))
	python3 /home/evl/halmas3/data/buffer_sizing_v2/simulator/ns-3.35/examples/Protean/plot_motivation.py $ALGFILE $ALG
	sleep 10
done
