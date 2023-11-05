source config.sh
DIR="$NS3/examples/Protean"
DUMP_DIR="$DIR/results"
mkdir -p $DUMP_DIR

DT=101
CS=103
FAB=102
IB=104
PROTEAN=114

CUBIC=1
DCTCP=2

BUF_ALGS=($DT $CS $FAB $IB $PROTEAN)
TCP_ALGS=($CUBIC $DCTCP)

SERVERS=16
LEAVES=4
SPINES=4
LINKS=1
SERVER_LEAF_CAP=10
LEAF_SPINE_CAP=10
LATENCY=10



STATIC_BUFFER=0
# BUFFER=$(( 1000*1000*9  ))
BUFFER_PER_PORT_PER_GBPS=9.6 # https://baiwei0427.github.io/papers/bcc-ton.pdf (Trident 2)
BUFFER=$(python3 -c "print(int($BUFFER_PER_PORT_PER_GBPS*1024*($SERVERS+$LINKS*$SPINES)*$SERVER_LEAF_CAP))")

# TCP=$CUBIC
TCP=$DCTCP


N_PRIO=3
BURST_SIZES=0.5
BURST_SIZE=$(python3 -c "print($BURST_SIZES*$BUFFER)")
BURST_FREQ=1


DT_ALPHA=50
PROTEAN_BETA=1
PER_PACKET_PROTEAN=0
USE_MAX_DQDT=0
USE_NORMALIZED_DQDT=0
USE_MULTI_PRIO=0
USE_HIGH_PRIO_SHORTS=0
BUILDUP_THRESH=3.75

echo "short999fct short99fct short95fct shortavgfct shortmedfct incast999fct incast99fct incast95fct incastavgfct incastmedfct long999fct long99fct long95fct longavgfct longmedfct goodput avgTh medTh bufmax buf999 buf99 buf95 avgBuf medBuf load burst alg tcp scenario nprio"

# Betas: 0.001 0.005 0.01 0.05 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 0.95 0.99
# Load
for BUFFER_PER_PORT_PER_GBPS in 19.2 9.6 5.12 3.44;do
	BUFFER=$(python3 -c "print(int($BUFFER_PER_PORT_PER_GBPS*1024*($SERVERS+$LINKS*$SPINES)*$SERVER_LEAF_CAP))")
	for TCP in $DCTCP;do
		for USE_HIGH_PRIO_SHORTS in 1;do
			for USE_MULTI_PRIO in 1; do
				for BURST_SIZES in 0.125 0.25 0.375 0.5 0.625 0.75;do
					for LOAD in 0.4 0.5 0.6 0.7 0.8;do
						# for USE_MAX_DQDT in 0 1; do
							# for USE_NORMALIZED_DQDT in 0 1;do
								for PROTEAN_BETA in 0.125;do
									for BURST_FREQ in 1;do
										for BUILDUP_THRESH in 2.25;do
											for ALG in $DT $CS $PROTEAN;do
												echo "BUFFER_PER_PORT_PER_GBPS: $BUFFER_PER_PORT_PER_GBPS"
												echo "BURST_SIZE: " $BURST_SIZES
												echo "LOAD: " $LOAD
												echo "BURST_FREQ: " $BURST_FREQ
												echo "ALG: " $ALG
												echo "TCP: " $TCP
												echo "USE_MAX_DQDT: " $USE_MAX_DQDT
												echo "USE_NORMALIZED_DQDT: " $USE_NORMALIZED_DQDT
												echo "USE_MULTI_PRIO: " $USE_MULTI_PRIO
												echo "PROTEAN_BETA: " $PROTEAN_BETA
												echo "BUILDUP_THRESH: " $BUILDUP_THRESH
												echo ""
												BURST_SIZE=$(python3 -c "print($BURST_SIZES*$BUFFER)")
												FLOWFILE="$DUMP_DIR/fcts-single-large-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ-$PROTEAN_BETA-$USE_NORMALIZED_DQDT-$USE_MAX_DQDT-$USE_MULTI_PRIO-$USE_HIGH_PRIO_SHORTS-$BUILDUP_THRESH-$BUFFER_PER_PORT_PER_GBPS.fct"
												TORFILE="$DUMP_DIR/tor-single-large-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ-$PROTEAN_BETA-$USE_NORMALIZED_DQDT-$USE_MAX_DQDT-$USE_MULTI_PRIO-$USE_HIGH_PRIO_SHORTS-$BUILDUP_THRESH-$BUFFER_PER_PORT_PER_GBPS.stat"
												python3 parseData-singleQ.py $FLOWFILE $TORFILE $LEAF_SPINE_CAP $(( $LATENCY*8 )) $LOAD $BURST_SIZES $ALG $TCP single $N_PRIO
												echo ""
												echo "###################################################"
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
