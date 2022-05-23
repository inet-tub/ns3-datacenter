# NS3="/home/vamsi/ABM-ns3/ns-3.35"
# DIR="$NS3/examples/abm-evaluation"
# DUMP_DIR="/home/vamsi/ABM-ns3/dump_all"
source config.sh
DIR="$NS3/examples/ABM"
DUMP_DIR="$DIR/dump_sigcomm"
mkdir $DUMP_DIR

DT=101
FAB=102
CS=103
IB=104
ABM=110

RENO=0
CUBIC=1
DCTCP=2
HPCC=3
POWERTCP=4
HOMA=5
TIMELY=6
THETAPOWERTCP=7

BUF_ALGS=($DT $FAB $CS $IB $ABM)
TCP_ALGS=($CUBIC $DCTCP $TIMELY $POWERTCP)

SERVERS=32
LEAVES=2
SPINES=2
LINKS=4
SERVER_LEAF_CAP=10
LEAF_SPINE_CAP=10
LATENCY=10

RED_MIN=65
RED_MAX=65

N_PRIO=2

ALPHAFILE="$DIR/alphas"
CDFFILE="$DIR/websearch.txt"
CDFNAME="WS"

ALPHA_UPDATE_INT=1 # 1 RTT


STATIC_BUFFER=0
# BUFFER=$(( 1000*1000*9  ))
BUFFER_PER_PORT_PER_GBPS=9.6 # https://baiwei0427.github.io/papers/bcc-ton.pdf (Trident 2)
BUFFER=$(python3 -c "print(int($BUFFER_PER_PORT_PER_GBPS*1024*($SERVERS+$LINKS*$SPINES)*$SERVER_LEAF_CAP))")

TCP=$CUBIC

START_TIME=10
END_TIME=24
FLOW_END_TIME=13

# SINGLE QUEUE - ALGS vs LOAD
BURST_SIZES=0.3
BURST_SIZE=$(python3 -c "print($BURST_SIZES*$BUFFER)")
BURST_FREQ=2

echo "short999fct short99fct short95fct shortavgfct shortmedfct incast999fct incast99fct incast95fct incastavgfct incastmedfct long999fct long99fct long95fct longavgfct longmedfct avgTh medTh bufmax buf999 buf99 buf95 avgBuf medBuf load burst alg tcp scenario nprio"

for ALG in ${BUF_ALGS[@]};do
	for LOAD in 0.2 0.4 0.6 0.8;do
		FLOWFILE="$DUMP_DIR/fcts-single-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ.fct"
		TORFILE="$DUMP_DIR/tor-single-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ.stat"
		python3 parseData-singleQ.py $FLOWFILE $TORFILE $LEAF_SPINE_CAP $(( $LATENCY*8 )) $LOAD $BURST_SIZES $ALG $TCP single $N_PRIO
	done
done

# SINGLE QUEUE - ALGS vs BURST_SIZE
LOAD=0.4
BURST_FREQ=2
for BURST_SIZES in 0.125 0.25 0.375 0.5 0.75;do
	for ALG in ${BUF_ALGS[@]};do
		FLOWFILE="$DUMP_DIR/fcts-single-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ.fct"
		TORFILE="$DUMP_DIR/tor-single-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ.stat"
		python3 parseData-singleQ.py $FLOWFILE $TORFILE $LEAF_SPINE_CAP $(( $LATENCY*8 )) $LOAD $BURST_SIZES $ALG $TCP single $N_PRIO
	done
done

# SINGLE QUEUE - TCPS vs ALGS vs BURST_SIZE
LOAD=0.4
BURST_FREQ=2
for TCP in $DCTCP $TIMELY $POWERTCP;do
	for BURST_SIZES in 0.125 0.25 0.375 0.5 0.75;do
		for ALG in $DT $ABM;do
			FLOWFILE="$DUMP_DIR/fcts-single-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ.fct"
			TORFILE="$DUMP_DIR/tor-single-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ.stat"
			python3 parseData-singleQ.py $FLOWFILE $TORFILE $LEAF_SPINE_CAP $(( $LATENCY*8 )) $LOAD $BURST_SIZES $ALG $TCP single $N_PRIO
		done
	done
done

# MULTI QUEUE
BURST_SIZES=0.3
BURST_FREQ=2
TCP=1
TCPS=($CUBIC $DCTCP $THETAPOWERTCP)
PRIO=(1 2 3)
for LOAD in 0.1 0.2 0.3 0.4 0.5 0.6;do
	for ALG in $DT $ABM;do
		flows="$DUMP_DIR/fcts-multi-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ.fct"
		TORFILE="$DUMP_DIR/tor-multi-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ.stat"
		for INDEX in 0 1 2;do
			cat $flows | head -n1 > /tmp/fcts
			cat $flows | awk '{if($8=='${PRIO[$INDEX]}') print $0}' >> /tmp/fcts
			FLOWFILE="/tmp/fcts"
			python3 parseData-singleQ.py $FLOWFILE $TORFILE $LEAF_SPINE_CAP $(( $LATENCY*8 )) $LOAD $BURST_SIZES $ALG ${TCPS[$INDEX]} multi $N_PRIO
		done
	done
done

# INTERVALS

LOAD=0.4
TCP=$DCTCP
ALG=$ABM
N_PRIO=8
BURST_FREQ=2
BURST_SIZES=0.5
BURST_SIZE=$(python3 -c "print($BURST_SIZES*$BUFFER)")
ALG=$ABM
for ALPHA_UPDATE_INT in 1 10 100 1000 10000;do
	FLOWFILE="$DUMP_DIR/fcts-intervalCubic-$ALPHA_UPDATE_INT.fct"
	TORFILE="$DUMP_DIR/tor-intervalCubic-$ALPHA_UPDATE_INT.stat"
	python3 parseData-singleQ.py $FLOWFILE $TORFILE $LEAF_SPINE_CAP $(( $LATENCY*8 )) $LOAD $BURST_SIZES $ALG $TCP $ALPHA_UPDATE_INT $N_PRIO
done

ALG=$DT
FLOWFILE="$DUMP_DIR/fcts-intervalCubic-dt.fct"
TORFILE="$DUMP_DIR/tor-intervalCubic-dt.stat"
python3 parseData-singleQ.py $FLOWFILE $TORFILE $LEAF_SPINE_CAP $(( $LATENCY*8 )) $LOAD $BURST_SIZES $ALG $TCP dt $N_PRIO

LOAD=0.4
BURST_SIZES=0.25
BURST_FREQ=2
for BUFFER_PER_PORT_PER_GBPS in 9.6 8 7 6 5.12 3.44;do
	for ALG in $DT $ABM $IB;do
		for TCP in $DCTCP $POWERTCP;do
			FLOWFILE="$DUMP_DIR/fcts-buffer-$TCP-$ALG-$BUFFER_PER_PORT_PER_GBPS.fct"
			TORFILE="$DUMP_DIR/tor-buffer-$TCP-$ALG-$BUFFER_PER_PORT_PER_GBPS.stat"
			python3 parseData-singleQ.py $FLOWFILE $TORFILE $LEAF_SPINE_CAP $(( $LATENCY*8 )) $LOAD $BURST_SIZES $ALG $TCP $BUFFER_PER_PORT_PER_GBPS $N_PRIO
		done
	done
done


LOAD=0.4
BURST_FREQ=2

BURST_SIZES=0.25

for TCP in $CUBIC $DCTCP;do
	for N_PRIO in 2 3 4 5 6 7 8;do
		for ALG in $DT $ABM;do
			FLOWFILE="$DUMP_DIR/fcts-unimulti-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ-$N_PRIO.fct"
			TORFILE="$DUMP_DIR/tor-unimulti-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ-$N_PRIO.stat"
			python3 parseData-singleQ.py $FLOWFILE $TORFILE $LEAF_SPINE_CAP $(( $LATENCY*8 )) $LOAD $BURST_SIZES $ALG $TCP multiqueue $N_PRIO
		done
	done
done
