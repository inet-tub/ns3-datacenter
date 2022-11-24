source config.sh
DIR="$NS3/examples/queueing-devel"
DUMP_DIR="$DIR/dump_queueing"
RES_DIR="$DIR/results_queueing"
WORKLOADS="$NS3/workloads"
mkdir $DUMP_DIR
mkdir $RES_DIR

######## THESE ARE HARD CODED VALUES. DO NOT CHANGE ############
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
###############################################################

BUF_ALGS=($DT $FAB $CS $IB $ABM)
TCP_ALGS=($CUBIC $DCTCP $TIMELY $POWERTCP)

START_TIME=10
END_TIME=24
FLOW_END_TIME=13

ALPHAFILE="$DIR/alphas"

CDFFILES[0]="$WORKLOADS/websearch.csv"
CDFFILES[1]="$WORKLOADS/datamining.csv"
CDFFILES[2]="$WORKLOADS/hadoop.csv"

CDFNAMES[0]="WS"
CDFNAMES[1]="DM"
CDFNAMES[2]="HP"

# Oversubscription = 1; 64 Servers; 8 ToRs, 2 Spines.
SERVERS=8
LEAVES=8
SPINES=2
LINKS=4
SERVER_LEAF_CAP=10 #Gbps
LEAF_SPINE_CAP=10 #Gbps
LATENCY=10 # Microseconds, rtt=80us

RED_MIN=65 # packets
RED_MAX=65 # packets

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
for CDFINDEX in 0 1 2;do
	CDFFILE=${CDFFILES[$CDFINDEX]}
	CDFNAME=${CDFNAMES[$CDFINDEX]}
	for TCP in ${TCP_ALGS[@]};do

		while [[ $(ps aux | grep 'queueing-optimized' | wc -l) -gt $N_CORES ]];do
			sleep 30;
			echo "waiting for cores, $N running..."
		done

		echo "timeNANO arrival nodeId portId type pktSize unsched qIndex" > $RES_DIR/arrival-${TCP_NAMES[$TCP]}-$CDFNAME-$LOAD.res
		echo "timeNANO arrival nodeId portId type pktSize unsched qIndex" > $RES_DIR/departure-${TCP_NAMES[$TCP]}-$CDFNAME-$LOAD.res

		FLOWFILE="/dev/null"
		TORFILE="/dev/null"
		RXFILE=$DUMP_DIR/arrival-${TCP_NAMES[$TCP]}-$CDFNAME-$LOAD.dat
		TXFILE=$DUMP_DIR/departure-${TCP_NAMES[$TCP]}-$CDFNAME-$LOAD.dat
		cat $RXFILE | awk '{if($1>10100000000 && $8==1) print $0}' >> $RES_DIR/arrival-${TCP_NAMES[$TCP]}-$CDFNAME-$LOAD.res
		cat $TXFILE | awk '{if($1>10100000000 && $8==1) print $0}' >> $RES_DIR/departure-${TCP_NAMES[$TCP]}-$CDFNAME-$LOAD.res
	done
done