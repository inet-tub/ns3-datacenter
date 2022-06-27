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


cd $NS3


N=0


# 30
SERVERS=32
LEAVES=2
SPINES=2
LINKS=4
SERVER_LEAF_CAP=10
LEAF_SPINE_CAP=10
LATENCY=10


LOAD=0.4
BURST_FREQ=2

BURST_SIZES=0.25

################################################
# Performance of Cubic and DCTCP
# Comparing DT and ABM
# All traffic is split across N_PRIO number of queues at each port. 
################################################

for TCP in $CUBIC $DCTCP;do
	for N_PRIO in 2 3 4 5 6 7 8;do
		for ALG in $DT $ABM;do
			FLOW_END_TIME=13 #$(python3 -c "print(10+3*0.8/$LOAD)")
			while [[ $(( $(ps aux | grep evaluation-multi-optimized | wc -l)+$(ps aux | grep evaluation-optimized | wc -l) )) -gt $N_CORES ]];do
				sleep 30;
				echo "waiting for cores, $N running..."
			done
			BURST_SIZE=$(python3 -c "print($BURST_SIZES*$BUFFER)")
			FLOWFILE="$DUMP_DIR/fcts-unimulti-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ-$N_PRIO.fct"
			TORFILE="$DUMP_DIR/tor-unimulti-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ-$N_PRIO.stat"
			N=$(( $N+1 ))
			(time ./waf --run "abm-evaluation-nprio --load=$LOAD --StartTime=$START_TIME --EndTime=$END_TIME --FlowLaunchEndTime=$FLOW_END_TIME --serverCount=$SERVERS --spineCount=$SPINES --leafCount=$LEAVES --linkCount=$LINKS --spineLeafCapacity=$LEAF_SPINE_CAP --leafServerCapacity=$SERVER_LEAF_CAP --linkLatency=$LATENCY --TcpProt=$TCP --BufferSize=$BUFFER --statBuf=$STATIC_BUFFER --algorithm=$ALG --RedMinTh=$RED_MIN --RedMaxTh=$RED_MAX --request=$BURST_SIZE --queryRequestRate=$BURST_FREQ --nPrior=$N_PRIO --alphasFile=$ALPHAFILE --cdfFileName=$CDFFILE --alphaUpdateInterval=$ALPHA_UPDATE_INT --fctOutFile=$FLOWFILE --torOutFile=$TORFILE" ; echo "$FLOWFILE")&
			sleep 5
		done
	done
done
echo "DONE $N"