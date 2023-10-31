source config.sh
DIR="$NS3/examples/Queueing"
DUMP_DIR="$DIR/dump_queueing"

if [ ! -d "$DUMP_DIR" ];then
	mkdir $DUMP_DIR
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

ALG=$DT
TCP_ALGS=($RENO $CUBIC $DCTCP $TIMELY $POWERTCP)

SERVERS=16
LEAVES=2
SPINES=4
LINKS=4
SERVER_LEAF_CAP=10
LEAF_SPINE_CAP=10
LATENCY=3

RED_MIN=65
RED_MAX=65

N_PRIO=2

ALPHAFILE="$DIR/alphas"
CDFFILES=("$DIR/websearch.csv" "$DIR/hadoop.csv" "$DIR/datamining.csv") 
CDFNAMES=("WS" "HP" "DM")

ALPHA_UPDATE_INT=1 # 1 RTT


STATIC_BUFFER=0
# BUFFER=$(( 1000*1000*9  ))
BUFFER_PER_PORT_PER_GBPS=5.14 # https://baiwei0427.github.io/papers/bcc-ton.pdf (Tomahawk)
BUFFER=$(python3 -c "print(int($BUFFER_PER_PORT_PER_GBPS*1024*($SERVERS+$LINKS*$SPINES)*$SERVER_LEAF_CAP))")

START_TIME=10
END_TIME=24
FLOW_END_TIME=20

ENABLE_STATS=0
ENABLE_LQD_TRCING=0
LQDOUTDIR="/tmp/lf"
RF_FILE="/tmp/rf" #rfModelFile
FLOWFILE="/tmp/ff"
TORFILE="/tmp/tf"
ERROR_PROB=0 # errorProb
AVERAGE_INTERVAL=1 # in RTT
RANDOM_SEED=8

ENABLE_QUEUELOG=1

cd $NS3


LOADS=(0.2 0.4 0.6 0.8)
BURSTS=(0)
BURST_FREQ=0

EXP=$1
N=0

BURST_SIZES=0
ALG=$DT
for INDEX in 0 1 2;do
	CDFFILE=${CDFFILES[$INDEX]}
	CDFNAME=${CDFNAMES[$INDEX]}
	for TCP in ${TCP_ALGS[@]};do
		for LOAD in ${LOADS[@]};do
			BURST_SIZE=$(python3 -c "print($BURST_SIZES*$BUFFER)")
			ARRIVAL_FILE="$DUMP_DIR/arrival-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ-$CDFNAME.log"
			DEPARTURE_FILE="$DUMP_DIR/departure-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ-$CDFNAME.log"
			N=$(( $N+1 ))
			echo "$N $ARRIVAL_FILE"
			while [[ $(( $(ps aux | grep queueing-logs-optimized | wc -l) )) -gt $N_CORES ]];do
				sleep 30;
				echo "running $N experiment(s)..."
			done
			if [[ $EXP == 1 ]];then
				(time ./waf --run "queueing-logs --enableQueueLog=$ENABLE_QUEUELOG --arrivalOutFile=$ARRIVAL_FILE --departureOutFile=$DEPARTURE_FILE --rfModelFile=$RF_FILE --errorProb=$ERROR_PROB --load=$LOAD --StartTime=$START_TIME --EndTime=$END_TIME --FlowLaunchEndTime=$FLOW_END_TIME --serverCount=$SERVERS --spineCount=$SPINES --leafCount=$LEAVES --linkCount=$LINKS --spineLeafCapacity=$LEAF_SPINE_CAP --leafServerCapacity=$SERVER_LEAF_CAP --linkLatency=$LATENCY --TcpProt=$TCP --BufferSize=$BUFFER --statBuf=$STATIC_BUFFER --algorithm=$ALG --RedMinTh=$RED_MIN --RedMaxTh=$RED_MAX --request=$BURST_SIZE --queryRequestRate=$BURST_FREQ --nPrior=$N_PRIO --alphasFile=$ALPHAFILE --cdfFileName=$CDFFILE --alphaUpdateInterval=$ALPHA_UPDATE_INT --fctOutFile=$FLOWFILE --torOutFile=$TORFILE --averageIntervalNano=$AVERAGE_INTERVAL --randomSeed=$RANDOM_SEED --enableStats=$ENABLE_STATS --enableLqdTracing=$ENABLE_LQD_TRCING"; echo "$FLOWFILE")&
				sleep 10
			fi
		done
	done
done