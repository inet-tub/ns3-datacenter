source config.sh
DIR="$NS3/examples/Credence"
LQD_DIR="$DIR/lqd-logs"
DUMP_DIR="$DIR/dump_nsdi"

if [ ! -d "$DUMP_DIR" ];then
	mkdir $DUMP_DIR
fi
if [ ! -d "$LQD_DIR" ];then
	mkdir $LQD_DIR
fi

DT=101
FAB=102
CS=103
IB=104
ABM=110
LQD=111
FOLLOWLQD=112
CREDENCE=666

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

SERVERS=16
LEAVES=2
SPINES=4
LINKS=1
SERVER_LEAF_CAP=10
LEAF_SPINE_CAP=10
LATENCY=4

RED_MIN=65
RED_MAX=65

N_PRIO=2

ALPHAFILE="$DIR/alphas"
CDFFILE="$DIR/websearch.csv"
CDFNAME="WS"

ALPHA_UPDATE_INT=1 # 1 RTT


STATIC_BUFFER=0
# BUFFER=$(( 1000*1000*9  ))
BUFFER_PER_PORT_PER_GBPS=5.14 # https://baiwei0427.github.io/papers/bcc-ton.pdf (Tomahawk)
BUFFER=$(python3 -c "print(int($BUFFER_PER_PORT_PER_GBPS*1024*($SERVERS+$LINKS*$SPINES)*$SERVER_LEAF_CAP))")

START_TIME=10
END_TIME=24
FLOW_END_TIME=13

ENABLE_STATS=1
ENABLE_LQD_TRCING=1
LQDOUTDIR="$NS3/examples/Credence/lqd-logs"
AVERAGE_INTERVAL=1 # in RTT
RANDOM_SEED=8


cd $NS3


N=0

LOADS=(0.2 0.4 0.6 0.8)
BURSTS=(0.5 0.75 0.875 1.0 1.25)
BURST_FREQ=2

LOAD=0.8
TCP=$DCTCP
ALG=$LQD

for BURST_SIZES in ${BURSTS[@]};do
	BURST_SIZE=$(python3 -c "print($BURST_SIZES*$BUFFER)")
	FLOWFILE="$DUMP_DIR/fct-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ.fct"
	TORFILE="$DUMP_DIR/tor-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ.stat"
	LQDOUTFILE="$LQDOUTDIR/lqdtrace-$TCP-$LOAD-$BURST_SIZES-$BURST_FREQ-$CDFNAME-"
	N=$(( $N+1 ))
	(time ./waf --run "credence-evaluation --load=$LOAD --StartTime=$START_TIME --EndTime=$END_TIME --FlowLaunchEndTime=$FLOW_END_TIME --serverCount=$SERVERS --spineCount=$SPINES --leafCount=$LEAVES --linkCount=$LINKS --spineLeafCapacity=$LEAF_SPINE_CAP --leafServerCapacity=$SERVER_LEAF_CAP --linkLatency=$LATENCY --TcpProt=$TCP --BufferSize=$BUFFER --statBuf=$STATIC_BUFFER --algorithm=$ALG --RedMinTh=$RED_MIN --RedMaxTh=$RED_MAX --request=$BURST_SIZE --queryRequestRate=$BURST_FREQ --nPrior=$N_PRIO --alphasFile=$ALPHAFILE --cdfFileName=$CDFFILE --alphaUpdateInterval=$ALPHA_UPDATE_INT --fctOutFile=$FLOWFILE --torOutFile=$TORFILE --lqdOutFile=$LQDOUTFILE --averageIntervalNano=$AVERAGE_INTERVAL --randomSeed=$RANDOM_SEED --enableStats=$ENABLE_STATS --enableLqdTracing=$ENABLE_LQD_TRCING"; echo "$FLOWFILE")&
	sleep 10
done

while [[ $(( $(ps aux | grep credence-evaluation | wc -l) )) -gt 1 ]];do
	sleep 30;
	echo "running $N experiment(s)..."
done