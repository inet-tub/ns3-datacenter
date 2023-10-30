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

BUF_ALGS=($DT $ABM $LQD $CREDENCE)
TCP_ALGS=($DCTCP $TIMELY $POWERTCP)

# Faster test case
# SERVERS=16
# LEAVES=2
# SPINES=4
# LINKS=1
# SERVER_LEAF_CAP=10
# LEAF_SPINE_CAP=10
# LATENCY=3

SERVERS=16
LEAVES=16
SPINES=4
LINKS=1
SERVER_LEAF_CAP=10
LEAF_SPINE_CAP=10
LATENCY=3

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
ENABLE_LQD_TRCING=0
LQDOUTDIR="$NS3/examples/Credence/lqd-logs"
RF_FILE="$NS3/examples/Credence/rf_models/model-2-0.8-0.75-2-WS-4-" #rfModelFile
ERROR_PROB=0 # errorProb
AVERAGE_INTERVAL=1 # in RTT
RANDOM_SEED=8

cd $NS3


LOADS=(0.2 0.4 0.6 0.8)
BURSTS=(0.125 0.25 0.375 0.5 0.625 0.75 0.875 1.0)
BURST_FREQ=2

EXP=$1
N=0


# # 12 simulations
# BURST_SIZES=0.5
# TCP=$DCTCP
# ERROR_PROB=0
# for ALG in ${BUF_ALGS[@]};do
# 	for LOAD in ${LOADS[@]};do
# 		if [[ $LOAD == "0.4" ]];then
# 			continue
# 		fi
# 		BURST_SIZE=$(python3 -c "print($BURST_SIZES*$BUFFER)")
# 		FLOWFILE="$DUMP_DIR/fct-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ.fct"
# 		TORFILE="$DUMP_DIR/tor-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ.stat"
# 		N=$(( $N+1 ))
# 		echo "$N $FLOWFILE"
# 		while [[ $(( $(ps aux | grep credence-evaluation-optimized | wc -l) )) -gt $N_CORES ]];do
# 			sleep 30;
# 			echo "running $N experiment(s)..."
# 		done
# 		if [[ $EXP == 1 ]];then
# 			(time ./waf --run "credence-evaluation --rfModelFile=$RF_FILE --errorProb=$ERROR_PROB --load=$LOAD --StartTime=$START_TIME --EndTime=$END_TIME --FlowLaunchEndTime=$FLOW_END_TIME --serverCount=$SERVERS --spineCount=$SPINES --leafCount=$LEAVES --linkCount=$LINKS --spineLeafCapacity=$LEAF_SPINE_CAP --leafServerCapacity=$SERVER_LEAF_CAP --linkLatency=$LATENCY --TcpProt=$TCP --BufferSize=$BUFFER --statBuf=$STATIC_BUFFER --algorithm=$ALG --RedMinTh=$RED_MIN --RedMaxTh=$RED_MAX --request=$BURST_SIZE --queryRequestRate=$BURST_FREQ --nPrior=$N_PRIO --alphasFile=$ALPHAFILE --cdfFileName=$CDFFILE --alphaUpdateInterval=$ALPHA_UPDATE_INT --fctOutFile=$FLOWFILE --torOutFile=$TORFILE --averageIntervalNano=$AVERAGE_INTERVAL --randomSeed=$RANDOM_SEED --enableStats=$ENABLE_STATS --enableLqdTracing=$ENABLE_LQD_TRCING"; echo "$FLOWFILE")&
# 			sleep 10
# 		fi
# 	done
# done

# # 32 simulations
# LOAD=0.4
# TCP=$DCTCP
# ERROR_PROB=0
# for ALG in ${BUF_ALGS[@]};do
# 	for BURST_SIZES in ${BURSTS[@]};do
# 		BURST_SIZE=$(python3 -c "print($BURST_SIZES*$BUFFER)")
# 		FLOWFILE="$DUMP_DIR/fct-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ.fct"
# 		TORFILE="$DUMP_DIR/tor-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ.stat"
# 		N=$(( $N+1 ))
# 		echo "$N $FLOWFILE"
# 		while [[ $(( $(ps aux | grep credence-evaluation-optimized | wc -l) )) -gt $N_CORES ]];do
# 			sleep 30;
# 			echo "running $N experiment(s)..."
# 		done
# 		if [[ $EXP == 1 ]];then
# 			(time ./waf --run "credence-evaluation --rfModelFile=$RF_FILE --errorProb=$ERROR_PROB --load=$LOAD --StartTime=$START_TIME --EndTime=$END_TIME --FlowLaunchEndTime=$FLOW_END_TIME --serverCount=$SERVERS --spineCount=$SPINES --leafCount=$LEAVES --linkCount=$LINKS --spineLeafCapacity=$LEAF_SPINE_CAP --leafServerCapacity=$SERVER_LEAF_CAP --linkLatency=$LATENCY --TcpProt=$TCP --BufferSize=$BUFFER --statBuf=$STATIC_BUFFER --algorithm=$ALG --RedMinTh=$RED_MIN --RedMaxTh=$RED_MAX --request=$BURST_SIZE --queryRequestRate=$BURST_FREQ --nPrior=$N_PRIO --alphasFile=$ALPHAFILE --cdfFileName=$CDFFILE --alphaUpdateInterval=$ALPHA_UPDATE_INT --fctOutFile=$FLOWFILE --torOutFile=$TORFILE --averageIntervalNano=$AVERAGE_INTERVAL --randomSeed=$RANDOM_SEED --enableStats=$ENABLE_STATS --enableLqdTracing=$ENABLE_LQD_TRCING"; echo "$FLOWFILE")&
# 			sleep 10
# 		fi
# 	done
# done


# # 2 simulations
# LOAD=0.4
# BURST_SIZES=0.5
# TCP=$DCTCP
# ALG=$CREDENCE
# ERRORS=(0.001 0.005)
# for ERROR_PROB in ${ERRORS[@]};do
# 	BURST_SIZE=$(python3 -c "print($BURST_SIZES*$BUFFER)")
# 	FLOWFILE="$DUMP_DIR/fct-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ-$ERROR_PROB.fct"
# 	TORFILE="$DUMP_DIR/tor-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ-$ERROR_PROB.stat"
# 	N=$(( $N+1 ))
# 	echo "$N $FLOWFILE"
# 	while [[ $(( $(ps aux | grep credence-evaluation-optimized | wc -l) )) -gt $N_CORES ]];do
# 		sleep 30;
# 		echo "running $N experiment(s)..."
# 	done
# 	if [[ $EXP == 1 ]];then
# 		(time ./waf --run "credence-evaluation --rfModelFile=$RF_FILE --errorProb=$ERROR_PROB --load=$LOAD --StartTime=$START_TIME --EndTime=$END_TIME --FlowLaunchEndTime=$FLOW_END_TIME --serverCount=$SERVERS --spineCount=$SPINES --leafCount=$LEAVES --linkCount=$LINKS --spineLeafCapacity=$LEAF_SPINE_CAP --leafServerCapacity=$SERVER_LEAF_CAP --linkLatency=$LATENCY --TcpProt=$TCP --BufferSize=$BUFFER --statBuf=$STATIC_BUFFER --algorithm=$ALG --RedMinTh=$RED_MIN --RedMaxTh=$RED_MAX --request=$BURST_SIZE --queryRequestRate=$BURST_FREQ --nPrior=$N_PRIO --alphasFile=$ALPHAFILE --cdfFileName=$CDFFILE --alphaUpdateInterval=$ALPHA_UPDATE_INT --fctOutFile=$FLOWFILE --torOutFile=$TORFILE --averageIntervalNano=$AVERAGE_INTERVAL --randomSeed=$RANDOM_SEED --enableStats=$ENABLE_STATS --enableLqdTracing=$ENABLE_LQD_TRCING"; echo "$FLOWFILE")&
# 		sleep 10
# 	fi
# done


# # 24 simulations
# LOAD=0.4
# TCP_ALGS=($POWERTCP)
# BUF_ALGS=($CREDENCE $DT $ABM)
# ERROR_PROB=0
# for ALG in ${BUF_ALGS[@]};do
# 	for BURST_SIZES in ${BURSTS[@]};do
# 		for TCP in ${TCP_ALGS[@]};do
# 			BURST_SIZE=$(python3 -c "print($BURST_SIZES*$BUFFER)")
# 			FLOWFILE="$DUMP_DIR/fct-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ.fct"
# 			TORFILE="$DUMP_DIR/tor-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ.stat"
# 			N=$(( $N+1 ))
# 			echo "$N $FLOWFILE"
# 			while [[ $(( $(ps aux | grep credence-evaluation-optimized | wc -l) )) -gt $N_CORES ]];do
# 				sleep 30;
# 				echo "running $N experiment(s)..."
# 			done
# 			if [[ $EXP == 1 ]];then
# 				(time ./waf --run "credence-evaluation --rfModelFile=$RF_FILE --errorProb=$ERROR_PROB --load=$LOAD --StartTime=$START_TIME --EndTime=$END_TIME --FlowLaunchEndTime=$FLOW_END_TIME --serverCount=$SERVERS --spineCount=$SPINES --leafCount=$LEAVES --linkCount=$LINKS --spineLeafCapacity=$LEAF_SPINE_CAP --leafServerCapacity=$SERVER_LEAF_CAP --linkLatency=$LATENCY --TcpProt=$TCP --BufferSize=$BUFFER --statBuf=$STATIC_BUFFER --algorithm=$ALG --RedMinTh=$RED_MIN --RedMaxTh=$RED_MAX --request=$BURST_SIZE --queryRequestRate=$BURST_FREQ --nPrior=$N_PRIO --alphasFile=$ALPHAFILE --cdfFileName=$CDFFILE --alphaUpdateInterval=$ALPHA_UPDATE_INT --fctOutFile=$FLOWFILE --torOutFile=$TORFILE --averageIntervalNano=$AVERAGE_INTERVAL --randomSeed=$RANDOM_SEED --enableStats=$ENABLE_STATS --enableLqdTracing=$ENABLE_LQD_TRCING"; echo "$FLOWFILE")&
# 				sleep 10
# 			fi
# 		done
# 	done
# done


###################################################################################################################################################################

# 10 simulations
LOAD=0.4
TCP=$DCTCP
BURST_SIZES=0.5
ERROR_PROB=0
BUF_ALGS=($ABM $CREDENCE)
for ALG in ${BUF_ALGS[@]};do
	for LATENCY in 8 4 3 2 1;do
		BURST_SIZE=$(python3 -c "print($BURST_SIZES*$BUFFER)")
		FLOWFILE="$DUMP_DIR/fct-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ-$LATENCY.fct"
		TORFILE="$DUMP_DIR/tor-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ-$LATENCY.stat"
		N=$(( $N+1 ))
		echo "$N $FLOWFILE"
		while [[ $(( $(ps aux | grep credence-evaluation-optimized | wc -l) )) -gt $N_CORES ]];do
			sleep 30;
			echo "running $N experiment(s)..."
		done
		if [[ $EXP == 1 ]];then
			(time ./waf --run "credence-evaluation --rfModelFile=$RF_FILE --errorProb=$ERROR_PROB --load=$LOAD --StartTime=$START_TIME --EndTime=$END_TIME --FlowLaunchEndTime=$FLOW_END_TIME --serverCount=$SERVERS --spineCount=$SPINES --leafCount=$LEAVES --linkCount=$LINKS --spineLeafCapacity=$LEAF_SPINE_CAP --leafServerCapacity=$SERVER_LEAF_CAP --linkLatency=$LATENCY --TcpProt=$TCP --BufferSize=$BUFFER --statBuf=$STATIC_BUFFER --algorithm=$ALG --RedMinTh=$RED_MIN --RedMaxTh=$RED_MAX --request=$BURST_SIZE --queryRequestRate=$BURST_FREQ --nPrior=$N_PRIO --alphasFile=$ALPHAFILE --cdfFileName=$CDFFILE --alphaUpdateInterval=$ALPHA_UPDATE_INT --fctOutFile=$FLOWFILE --torOutFile=$TORFILE --averageIntervalNano=$AVERAGE_INTERVAL --randomSeed=$RANDOM_SEED --enableStats=$ENABLE_STATS --enableLqdTracing=$ENABLE_LQD_TRCING"; echo "$FLOWFILE")&
			sleep 10
		fi
	done
done

# 3 simulations
LOAD=0.4
BURST_SIZES=0.5
TCP=$DCTCP
ALG=$CREDENCE
ERRORS=(0.01 0.05 0.1)
for ERROR_PROB in ${ERRORS[@]};do
	BURST_SIZE=$(python3 -c "print($BURST_SIZES*$BUFFER)")
	FLOWFILE="$DUMP_DIR/fct-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ-$ERROR_PROB.fct"
	TORFILE="$DUMP_DIR/tor-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ-$ERROR_PROB.stat"
	N=$(( $N+1 ))
	echo "$N $FLOWFILE"
	while [[ $(( $(ps aux | grep credence-evaluation-optimized | wc -l) )) -gt $N_CORES ]];do
		sleep 30;
		echo "running $N experiment(s)..."
	done
	if [[ $EXP == 1 ]];then
		(time ./waf --run "credence-evaluation --rfModelFile=$RF_FILE --errorProb=$ERROR_PROB --load=$LOAD --StartTime=$START_TIME --EndTime=$END_TIME --FlowLaunchEndTime=$FLOW_END_TIME --serverCount=$SERVERS --spineCount=$SPINES --leafCount=$LEAVES --linkCount=$LINKS --spineLeafCapacity=$LEAF_SPINE_CAP --leafServerCapacity=$SERVER_LEAF_CAP --linkLatency=$LATENCY --TcpProt=$TCP --BufferSize=$BUFFER --statBuf=$STATIC_BUFFER --algorithm=$ALG --RedMinTh=$RED_MIN --RedMaxTh=$RED_MAX --request=$BURST_SIZE --queryRequestRate=$BURST_FREQ --nPrior=$N_PRIO --alphasFile=$ALPHAFILE --cdfFileName=$CDFFILE --alphaUpdateInterval=$ALPHA_UPDATE_INT --fctOutFile=$FLOWFILE --torOutFile=$TORFILE --averageIntervalNano=$AVERAGE_INTERVAL --randomSeed=$RANDOM_SEED --enableStats=$ENABLE_STATS --enableLqdTracing=$ENABLE_LQD_TRCING"; echo "$FLOWFILE")&
		sleep 10
	fi
done

# 48 simulations
LOAD=0.4
TCP_ALGS=($TIMELY $HPCC)
BUF_ALGS=($CREDENCE $DT $ABM)
ERROR_PROB=0
for ALG in ${BUF_ALGS[@]};do
	for BURST_SIZES in ${BURSTS[@]};do
		for TCP in ${TCP_ALGS[@]};do
			BURST_SIZE=$(python3 -c "print($BURST_SIZES*$BUFFER)")
			FLOWFILE="$DUMP_DIR/fct-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ.fct"
			TORFILE="$DUMP_DIR/tor-$TCP-$ALG-$LOAD-$BURST_SIZES-$BURST_FREQ.stat"
			N=$(( $N+1 ))
			echo "$N $FLOWFILE"
			while [[ $(( $(ps aux | grep credence-evaluation-optimized | wc -l) )) -gt $N_CORES ]];do
				sleep 30;
				echo "running $N experiment(s)..."
			done
			if [[ $EXP == 1 ]];then
				(time ./waf --run "credence-evaluation --rfModelFile=$RF_FILE --errorProb=$ERROR_PROB --load=$LOAD --StartTime=$START_TIME --EndTime=$END_TIME --FlowLaunchEndTime=$FLOW_END_TIME --serverCount=$SERVERS --spineCount=$SPINES --leafCount=$LEAVES --linkCount=$LINKS --spineLeafCapacity=$LEAF_SPINE_CAP --leafServerCapacity=$SERVER_LEAF_CAP --linkLatency=$LATENCY --TcpProt=$TCP --BufferSize=$BUFFER --statBuf=$STATIC_BUFFER --algorithm=$ALG --RedMinTh=$RED_MIN --RedMaxTh=$RED_MAX --request=$BURST_SIZE --queryRequestRate=$BURST_FREQ --nPrior=$N_PRIO --alphasFile=$ALPHAFILE --cdfFileName=$CDFFILE --alphaUpdateInterval=$ALPHA_UPDATE_INT --fctOutFile=$FLOWFILE --torOutFile=$TORFILE --averageIntervalNano=$AVERAGE_INTERVAL --randomSeed=$RANDOM_SEED --enableStats=$ENABLE_STATS --enableLqdTracing=$ENABLE_LQD_TRCING"; echo "$FLOWFILE")&
				sleep 10
			fi
		done
	done
done