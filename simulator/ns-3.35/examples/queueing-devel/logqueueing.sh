source config.sh
DIR="$NS3/examples/queueing-devel"
DUMP_DIR="$DIR/dump_queueing"
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

TCP_NAMES[0]="Reno"
TCP_NAMES[1]="Cubic"
TCP_NAMES[2]="Dctcp"
TCP_NAMES[3]="HPCC"
TCP_NAMES[4]="PowerTCP"
TCP_NAMES[5]="Homa"
TCP_NAMES[6]="Timely"
TCP_NAMES[7]="Theta-PowerTCP"

BUF_ALGS=($DT $FAB $CS $IB $ABM)
TCP_ALGS=($CUBIC $DCTCP $TIMELY $POWERTCP)

START_TIME=10
END_TIME=24
FLOW_END_TIME=13

ALPHAFILE="$DIR/alphas"
CDFFILE="$DIR/websearch.txt"
CDFNAME="WS"

SERVERS=32
LEAVES=2
SPINES=2
LINKS=4
SERVER_LEAF_CAP=10
LEAF_SPINE_CAP=10
LATENCY=10

RED_MIN=65
RED_MAX=65

STATIC_BUFFER=0
BUFFER_PER_PORT_PER_GBPS=9.6
BUFFER=$(python3 -c "print(int($BUFFER_PER_PORT_PER_GBPS*1024*($SERVERS+$LINKS*$SPINES)*$SERVER_LEAF_CAP))")

N_PRIO=2

cd $NS3

LOAD=0.8
BURST_SIZES=0
BURST_SIZE=0
BURST_FREQ=0

ALG=$DT
for TCP in ${TCP_ALGS[@]};do

	while [[ $(ps aux | grep 'queueing-optimized' | wc -l) -gt $N_CORES ]];do
		sleep 30;
		echo "waiting for cores, $N running..."
	done

	FLOWFILE=""
	TORFILE=""
	RXFILE=$DUMP_DIR/arrival-$TCP.dat
	TXFILE=$DUMP_DIR/departure-$TCP.dat
	echo "runnig queueing simulation with ${TCP_NAMES[$TCP]}"
	(time ./waf --run "queueing --load=$LOAD --StartTime=$START_TIME --EndTime=$END_TIME --FlowLaunchEndTime=$FLOW_END_TIME --serverCount=$SERVERS --spineCount=$SPINES --leafCount=$LEAVES --linkCount=$LINKS --spineLeafCapacity=$LEAF_SPINE_CAP --leafServerCapacity=$SERVER_LEAF_CAP --linkLatency=$LATENCY --TcpProt=$TCP --BufferSize=$BUFFER --statBuf=$STATIC_BUFFER --algorithm=$ALG --RedMinTh=$RED_MIN --RedMaxTh=$RED_MAX --request=$BURST_SIZE --queryRequestRate=$BURST_FREQ --nPrior=$N_PRIO --fctOutFile=$FLOWFILE --torOutFile=$TORFILE --rxOutFile=$RXFILE --txOutFile=$TXFILE --alphasFile=$ALPHAFILE --cdfFileName=$CDFFILE" ; echo "$RXFILE")&
done
