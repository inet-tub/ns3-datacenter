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


START_TIME=10
END_TIME=24
FLOW_END_TIME=13


cd $NS3

SERVERS=32
LEAVES=2
SPINES=2
LINKS=4
SERVER_LEAF_CAP=10
LEAF_SPINE_CAP=10
LATENCY=10

#5
LOAD=0.4
TCP_ALGS=($DCTCP $POWERTCP)
N_PRIO=2
BURST_FREQ=2
ALPHA_UPDATE_INT=1

BUFFER_PER_PORT_PER_GBPS=9.6 # https://baiwei0427.github.io/papers/bcc-ton.pdf (Trident 2)
BUFFER=$(python3 -c "print(int($BUFFER_PER_PORT_PER_GBPS*1024*($SERVERS+$LINKS*$SPINES)*$SERVER_LEAF_CAP))")
BURST_SIZES=0.25
BURST_SIZE=$(python3 -c "print($BURST_SIZES*$BUFFER)")


################################################################
# DT, ABM and IB under different Buffer Sizes. 
# Using Cubic and DCTCP, at 40% websearch load and incast workload at Burst size of 25% of initial buffer size and request rate = 2
###############################################################

for BUFFER_PER_PORT_PER_GBPS in 9.6 8 7 6 5.12 3.44;do
	BUFFER=$(python3 -c "print(int($BUFFER_PER_PORT_PER_GBPS*1024*($SERVERS+$LINKS*$SPINES)*$SERVER_LEAF_CAP))")
	for ALG in $DT $ABM $IB;do
		for TCP in ${TCP_ALGS[@]};do
			FLOW_END_TIME=13 #$(python3 -c "print(10+3*0.8/$LOAD)")
			while [[ $(( $(ps aux | grep evaluation-multi-optimized | wc -l)+$(ps aux | grep evaluation-optimized | wc -l) )) -gt $N_CORES ]];do
				sleep 30;
				echo "waiting for cores, $N running..."
			done
			FLOWFILE="$DUMP_DIR/fcts-buffer-$TCP-$ALG-$BUFFER_PER_PORT_PER_GBPS.fct"
			TORFILE="$DUMP_DIR/tor-buffer-$TCP-$ALG-$BUFFER_PER_PORT_PER_GBPS.stat"
			N=$(( $N+1 ))
			(time ./waf --run "abm-evaluation --load=$LOAD --StartTime=$START_TIME --EndTime=$END_TIME --FlowLaunchEndTime=$FLOW_END_TIME --serverCount=$SERVERS --spineCount=$SPINES --leafCount=$LEAVES --linkCount=$LINKS --spineLeafCapacity=$LEAF_SPINE_CAP --leafServerCapacity=$SERVER_LEAF_CAP --linkLatency=$LATENCY --TcpProt=$TCP --BufferSize=$BUFFER --statBuf=$STATIC_BUFFER --algorithm=$ALG --RedMinTh=$RED_MIN --RedMaxTh=$RED_MAX --request=$BURST_SIZE --queryRequestRate=$BURST_FREQ --nPrior=$N_PRIO --alphasFile=$ALPHAFILE --cdfFileName=$CDFFILE --alphaUpdateInterval=$ALPHA_UPDATE_INT --fctOutFile=$FLOWFILE --torOutFile=$TORFILE" ; echo "$FLOWFILE")&
			sleep 10
		done
	done
done