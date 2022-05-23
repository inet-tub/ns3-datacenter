#####
# For reference. These are the numbers for each algorithm, used in the source code.
# define DT 101
# define FAB 102
# define CS 103
# define IB 104
# define ABM 110

# define RENO 0
# define CUBIC 1
# define DCTCP 2
# define HPCC 3
# define POWERTCP 4
# define HOMA 5
# define TIMELY 6
# define THETAPOWERTCP 7
#####
# cmd.AddValue ("StartTime", "Start time of the simulation", START_TIME);
# cmd.AddValue ("EndTime", "End time of the simulation", END_TIME);
# cmd.AddValue ("FlowLaunchEndTime", "End time of the flow launch period", FLOW_LAUNCH_END_TIME);
# cmd.AddValue ("load", "Load of the network, 0.0 - 1.0", load);
# cmd.AddValue ("serverCount", "The Server count", SERVER_COUNT);
# cmd.AddValue ("spineCount", "The Spine count", SPINE_COUNT);
# cmd.AddValue ("leafCount", "The Leaf count", LEAF_COUNT);
# cmd.AddValue ("linkCount", "The Link count", LINK_COUNT);
# cmd.AddValue ("spineLeafCapacity", "Spine <-> Leaf capacity in Gbps", spineLeafCapacity);
# cmd.AddValue ("leafServerCapacity", "Leaf <-> Server capacity in Gbps", leafServerCapacity);
# cmd.AddValue ("linkLatency", "linkLatency in microseconds", linkLatency);
# cmd.AddValue("TcpProt","Tcp protocol",TcpProt);
# cmd.AddValue ("BufferSize", "BufferSize in Bytes",BufferSize);
# cmd.AddValue ("statBuf", "staticBuffer in fraction of Total buffersize",statBuf);
# cmd.AddValue ("algorithm", "Buffer Management algorithm", algorithm);
# cmd.AddValue("RedMinTh", "Min Threshold for RED in packets", RedMinTh);
# cmd.AddValue("RedMaxTh", "Max Threshold for RED in packets", RedMaxTh);
# cmd.AddValue ("request", "Query Size in Bytes", requestSize);
# cmd.AddValue("queryRequestRate","Query request rate (poisson arrivals)",queryRequestRate);
# cmd.AddValue ("nPrior", "number of priorities",nPrior);
# cmd.AddValue ("alphasFile", "alpha values file (should be exactly nPrior lines)", alphasFile);
# cmd.AddValue ("cdfFileName", "File name for flow distribution", cdfFileName);
# cmd.AddValue ("cdfName", "Name for flow distribution", cdfName);
# cmd.AddValue("alphaUpdateInterval","(Number of Rtts) update interval for alpha values in ABM",alphaUpdateInterval);
# cmd.AddValue ("fctOutFile", "File path for FCTs", fctOutFile);
# cmd.AddValue ("torOutFile", "File path for ToR statistic", torOutFile);
# cmd.AddValue ("rto", "min Retransmission timeout value in MicroSeconds", rto);

NS3="/home/vamsi/ABM-ns3/ns-3.35"
DIR="$NS3/examples/abm-evaluation"
DUMP_DIR="/home/vamsi/ABM-ns3/dump_sigcomm"

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


cd $NS3


N=0

# #MULTI 12 
SERVERS=32
LEAVES=5
SPINES=2
LINKS=4
SERVER_LEAF_CAP=10
LEAF_SPINE_CAP=10
LATENCY=10

N_PRIO=8
BURST_SIZES=0.3
BURST_SIZE=$(python3 -c "print($BURST_SIZES*$BUFFER)")
BURST_FREQ=2
DCTCPLOAD=0.1
POWERLOAD=0.0
TCP=1 #this wont be used anyway
for CUBICLOAD in 0.1 0.2 0.3 0.4 0.5 0.6;do
	for ALG in $DT $ABM;do
		FLOW_END_TIME=13 #$(python3 -c "print(10+3*0.8/$LOAD)")
		FLOWFILE="$DUMP_DIR/fcts-multi-$TCP-$ALG-$CUBICLOAD-$BURST_SIZES-$BURST_FREQ.fct"
		TORFILE="$DUMP_DIR/tor-multi-$TCP-$ALG-$CUBICLOAD-$BURST_SIZES-$BURST_FREQ.stat"
		while [[ $(( $(ps aux | grep evaluation-multi-optimized | wc -l)+$(ps aux | grep evaluation-optimized | wc -l) )) -gt 37 ]];do
			sleep 30;
			echo "waiting for cores, $N running..."
		done
		N=$(( $N+1 ))
		(time ./waf --run "evaluation-multi --loadCubic=$CUBICLOAD --loadDctcp=$DCTCPLOAD --loadPower=$POWERLOAD --StartTime=$START_TIME --EndTime=$END_TIME --FlowLaunchEndTime=$FLOW_END_TIME --serverCount=$SERVERS --spineCount=$SPINES --leafCount=$LEAVES --linkCount=$LINKS --spineLeafCapacity=$LEAF_SPINE_CAP --leafServerCapacity=$SERVER_LEAF_CAP --linkLatency=$LATENCY --BufferSize=$BUFFER --statBuf=$STATIC_BUFFER --algorithm=$ALG --RedMinTh=$RED_MIN --RedMaxTh=$RED_MAX --request=$BURST_SIZE --queryRequestRate=$BURST_FREQ --nPrior=$N_PRIO --alphasFile=$ALPHAFILE --cdfFileName=$CDFFILE --alphaUpdateInterval=$ALPHA_UPDATE_INT --fctOutFile=$FLOWFILE --torOutFile=$TORFILE"; echo "$FLOWFILE")&
		sleep 10
	done
done

# 30
SERVERS=32
LEAVES=2
SPINES=2
LINKS=4
SERVER_LEAF_CAP=10
LEAF_SPINE_CAP=10
LATENCY=10

N_PRIO=2
LOAD=0.4
BURST_FREQ=2

for TCP in $DCTCP $TIMELY $POWERTCP;do
	for BURST_SIZES in 0.125 0.25 0.375 0.5 0.75;do
		for ALG in $DT $ABM;do
			FLOW_END_TIME=13 #$(python3 -c "print(10+3*0.8/$LOAD)")
			while [[ $(( $(ps aux | grep evaluation-multi-optimized | wc -l)+$(ps aux | grep evaluation-optimized | wc -l) )) -gt 37 ]];do
				sleep 30;
				echo "waiting for cores, $N running..."
			done
			BURST_SIZE=$(python3 -c "print($BURST_SIZES*$BUFFER)")
			FLOWFILE="$DUMP_DIR/fcts-single-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ.fct"
			TORFILE="$DUMP_DIR/tor-single-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ.stat"
			N=$(( $N+1 ))
			(time ./waf --run "evaluation --load=$LOAD --StartTime=$START_TIME --EndTime=$END_TIME --FlowLaunchEndTime=$FLOW_END_TIME --serverCount=$SERVERS --spineCount=$SPINES --leafCount=$LEAVES --linkCount=$LINKS --spineLeafCapacity=$LEAF_SPINE_CAP --leafServerCapacity=$SERVER_LEAF_CAP --linkLatency=$LATENCY --TcpProt=$TCP --BufferSize=$BUFFER --statBuf=$STATIC_BUFFER --algorithm=$ALG --RedMinTh=$RED_MIN --RedMaxTh=$RED_MAX --request=$BURST_SIZE --queryRequestRate=$BURST_FREQ --nPrior=$N_PRIO --alphasFile=$ALPHAFILE --cdfFileName=$CDFFILE --alphaUpdateInterval=$ALPHA_UPDATE_INT --fctOutFile=$FLOWFILE --torOutFile=$TORFILE" ; echo "$FLOWFILE")&
			sleep 10
		done
	done
done