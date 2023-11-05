source config.sh
DIR="$NS3/examples/Protean"
DUMP_DIR="$DIR/simple"
mkdir -p $DUMP_DIR

DT=101
CS=103
PROTEAN=114

DCTCP=2
CUBIC=1

BUF_ALGS=($DT $CS $PROTEAN)
TCP_ALGS=($CUBIC $DCTCP)


RED_MIN=65
RED_MAX=65


ALPHAFILE="$DIR/alphas"
CDFFILE="$DIR/websearch.txt"
CDFNAME="WS"

ALPHA_UPDATE_INT=1 # 1 RTT

STATIC_BUFFER=0
# BUFFER=$(( 1000*1000*9  ))
BUFFER_PER_PORT_PER_GBPS=9.6 # https://baiwei0427.github.io/papers/bcc-ton.pdf (Trident 2)
# BUFFER=$(python3 -c "print(int($BUFFER_PER_PORT_PER_GBPS*1024*($SERVERS+$LINKS*$SPINES)*$SERVER_LEAF_CAP))")
BUFFER=2000000
echo "Buffer: $BUFFER"
TCP=$CUBIC

START_TIME=10
FLOW_END_TIME=10.1
END_TIME=10.03
# END_TIME=10.015


cd $NS3


N=0

# #MULTI 12
SERVERS=22
LEAVES=1
SPINES=2
LINKS=1
SERVER_LEAF_CAP=10
LEAF_SPINE_CAP=10
LATENCY=10

N_PRIO=1
BURST_SIZES=0.3
BURST_SIZE=$(python3 -c "print($BURST_SIZES*$BUFFER)")
BURST_FREQ=1

DT_ALPHA=50
PROTEAN_BETA=1
PER_PACKET_PROTEAN=0
USE_MAX_DQDT=0
USE_NORMALIZED_DQDT=0
# 0.001 0.005 0.01 0.05 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 0.95 0.99
# for LOAD in 0.002;do
#     for PROTEAN_BETA in 0.03125 0.0625 0.125 0.25 0.5 0.75;do
# 		for ALG in $PROTEAN;do
# 			FLOWFILE="$DUMP_DIR/fcts-single-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ-$PROTEAN_BETA.fct"
# 			TORFILE="$DUMP_DIR/tor-single-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ-$PROTEAN_BETA.stat"
# 			ALGFILE="$DUMP_DIR/alg-single-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ-$PROTEAN_BETA.stat"

# 			while [[ $(ps aux | grep evaluation-optimized | wc -l) -gt 72 ]];do
# 				sleep 30;
# 				echo "waiting for cores, $N running..."
# 			done
# 			N=$(( $N+1 ))
# 			(./waf --run "protean-evaluation-simple --DT_alpha=$DT_ALPHA --Protean_beta=$PROTEAN_BETA --per_packet_Protean=$PER_PACKET_PROTEAN --load=$LOAD --StartTime=$START_TIME --EndTime=$END_TIME --FlowLaunchEndTime=$FLOW_END_TIME --serverCount=$SERVERS --spineCount=$SPINES --leafCount=$LEAVES --linkCount=$LINKS --spineLeafCapacity=$LEAF_SPINE_CAP --leafServerCapacity=$SERVER_LEAF_CAP --linkLatency=$LATENCY --TcpProt=$TCP --BufferSize=$BUFFER --statBuf=$STATIC_BUFFER --algorithm=$ALG --RedMinTh=$RED_MIN --RedMaxTh=$RED_MAX --request=$BURST_SIZE --queryRequestRate=$BURST_FREQ --nPrior=$N_PRIO --alphasFile=$ALPHAFILE --cdfFileName=$CDFFILE --alphaUpdateInterval=$ALPHA_UPDATE_INT --fctOutFile=$FLOWFILE --torOutFile=$TORFILE --algOutFile=$ALGFILE --useMaxDqDt=$USE_MAX_DQDT --useNormalizedDqDt=$USE_NORMALIZED_DQDT"; echo "$FLOWFILE")&
# 			sleep 10
# 		done
# 	done
# done

# wait

for LOAD in 0.002;do
    for PROTEAN_BETA in 0.03125 0.0625 0.125 0.25 0.5 0.75;do
		for ALG in $PROTEAN;do
			FLOWFILE="$DUMP_DIR/fcts-single-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ-$PROTEAN_BETA.fct"
			TORFILE="$DUMP_DIR/tor-single-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ-$PROTEAN_BETA.stat"
			ALGFILE="$DUMP_DIR/alg-single-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ-$PROTEAN_BETA.stat"

			while [[ $(ps aux | grep evaluation-optimized | wc -l) -gt 72 ]];do
				sleep 30;
				echo "waiting for cores, $N running..."
			done
			N=$(( $N+1 ))
			python3 /home/evl/halmas3/data/buffer_sizing_v2/simulator/ns-3.35/examples/Protean/plot.py $ALGFILE
			sleep 10
		done
	done
done
