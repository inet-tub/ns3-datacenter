source config.sh
DIR=$(pwd)
DUMP_DIR=$DIR/dump_vpi
RESULTS_DIR=$DIR/results_vpi

if [ ! -d "$DUMP_DIR" ];then
	mkdir $DUMP_DIR
fi
if [ ! -d "$RESULTS_DIR" ];then
	mkdir $RESULTS_DIR
fi

cd $NS3

DT=101
FAB=102
ABM=110
REVERIE=111

DCQCNCC=1
INTCC=3
TIMELYCC=7
DCTCPCC=8
PINTCC=10

FLOW_ECMP=0
RANDOM_ECMP=1
SOURCE_ROUTING=2

ALL_TO_ALL=666
ALL_REDUCE=667

RING=777
TREE=778


NUM=0

# BUFFER_ALGS=($DT $FAB $ABM "reverie")
BUFFER_ALG=$DT
BUFFERMODEL="sonic"

egresslossyFrac=1.0

START_TIME=1
END_TIME=3
BUFFER_PER_PORT_PER_GBPS=5.12 # in KiloBytes per port per Gbps
BUFFERSIZE=$(python3 -c "print(32*100*1000*$BUFFER_PER_PORT_PER_GBPS)") # in Bytes
ALPHAFILE=$DIR/alphas

EXP=$1

rto=1000 # number of RTTs (change this later. For now, PFC is enabled).

# Default values
QP_WINDOW=65535
QP_RANDOMIZE="false"
MULTI_PATH="false"
ROUTING=$FLOW_ECMP
COLLECTIVE=$ALL_TO_ALL
ALG=$RING


TRANSFER_SIZES=(8000 16000 32000 64000 128000)
QP_WINDOWS=(2 4 8 16 32 64 128 256)
############################################################################

RDMACC=$DCTCPCC

QP_WINDOW=256

for TRANSFER_SIZE in ${TRANSFER_SIZES[@]};do

	echo "transferSize,multiPath,multiPathRand,singlePath,singlePathRand"
	
	# for MULTI_PATH in "true" "false";do

	# 	if [[ $MULTI_PATH == "true" ]];then
	# 		ROUTING=$RANDOM_ECMP
	# 	else
	# 		ROUTING=$FLOW_ECMP
	# 	fi

	# 	for QP_WINDOW in ${QP_WINDOWS[@]};do


	# 		for QP_RANDOMIZE in "true" "false";do

	MULTI_PATH="true"
	ROUTING="RANDOM_ECMP"
	QP_RANDOMIZE="false"

	FCTFILE=$DUMP_DIR/evaluation-$RDMACC-$MULTI_PATH-$ROUTING-$QP_RANDOMIZE-$QP_WINDOW-$TRANSFER_SIZE.fct
	TORFILE=$DUMP_DIR/evaluation-$RDMACC-$MULTI_PATH-$ROUTING-$QP_RANDOMIZE-$QP_WINDOW-$TRANSFER_SIZE.tor
	DUMPFILE=$DUMP_DIR/evaluation-$RDMACC-$MULTI_PATH-$ROUTING-$QP_RANDOMIZE-$QP_WINDOW-$TRANSFER_SIZE.out
	PFCFILE=$DUMP_DIR/evaluation-$RDMACC-$MULTI_PATH-$ROUTING-$QP_RANDOMIZE-$QP_WINDOW-$TRANSFER_SIZE.pfc

	COMPLETION=$(cat $FCTFILE| tail -n1 | awk '{print $1}')
	COMPLETION_TIME[0]=$(python3 -c "print($COMPLETION-1)")
	###################################################################################

	MULTI_PATH="true"
	ROUTING="RANDOM_ECMP"
	QP_RANDOMIZE="true"

	FCTFILE=$DUMP_DIR/evaluation-$RDMACC-$MULTI_PATH-$ROUTING-$QP_RANDOMIZE-$QP_WINDOW-$TRANSFER_SIZE.fct
	TORFILE=$DUMP_DIR/evaluation-$RDMACC-$MULTI_PATH-$ROUTING-$QP_RANDOMIZE-$QP_WINDOW-$TRANSFER_SIZE.tor
	DUMPFILE=$DUMP_DIR/evaluation-$RDMACC-$MULTI_PATH-$ROUTING-$QP_RANDOMIZE-$QP_WINDOW-$TRANSFER_SIZE.out
	PFCFILE=$DUMP_DIR/evaluation-$RDMACC-$MULTI_PATH-$ROUTING-$QP_RANDOMIZE-$QP_WINDOW-$TRANSFER_SIZE.pfc

	COMPLETION=$(cat $FCTFILE| tail -n1 | awk '{print $1}')
	COMPLETION_TIME[1]=$(python3 -c "print($COMPLETION-1)")
	###################################################################################

	MULTI_PATH="false"
	ROUTING="FLOW_ECMP"
	QP_RANDOMIZE="false"

	FCTFILE=$DUMP_DIR/evaluation-$RDMACC-$MULTI_PATH-$ROUTING-$QP_RANDOMIZE-$QP_WINDOW-$TRANSFER_SIZE.fct
	TORFILE=$DUMP_DIR/evaluation-$RDMACC-$MULTI_PATH-$ROUTING-$QP_RANDOMIZE-$QP_WINDOW-$TRANSFER_SIZE.tor
	DUMPFILE=$DUMP_DIR/evaluation-$RDMACC-$MULTI_PATH-$ROUTING-$QP_RANDOMIZE-$QP_WINDOW-$TRANSFER_SIZE.out
	PFCFILE=$DUMP_DIR/evaluation-$RDMACC-$MULTI_PATH-$ROUTING-$QP_RANDOMIZE-$QP_WINDOW-$TRANSFER_SIZE.pfc

	COMPLETION=$(cat $FCTFILE| tail -n1 | awk '{print $1}')
	COMPLETION_TIME[2]=$(python3 -c "print($COMPLETION-1)")
	###################################################################################

	MULTI_PATH="false"
	ROUTING="FLOW_ECMP"
	QP_RANDOMIZE="true"

	FCTFILE=$DUMP_DIR/evaluation-$RDMACC-$MULTI_PATH-$ROUTING-$QP_RANDOMIZE-$QP_WINDOW-$TRANSFER_SIZE.fct
	TORFILE=$DUMP_DIR/evaluation-$RDMACC-$MULTI_PATH-$ROUTING-$QP_RANDOMIZE-$QP_WINDOW-$TRANSFER_SIZE.tor
	DUMPFILE=$DUMP_DIR/evaluation-$RDMACC-$MULTI_PATH-$ROUTING-$QP_RANDOMIZE-$QP_WINDOW-$TRANSFER_SIZE.out
	PFCFILE=$DUMP_DIR/evaluation-$RDMACC-$MULTI_PATH-$ROUTING-$QP_RANDOMIZE-$QP_WINDOW-$TRANSFER_SIZE.pfc

	COMPLETION=$(cat $FCTFILE| tail -n1 | awk '{print $1}')
	COMPLETION_TIME[3]=$(python3 -c "print($COMPLETION-1)")
	###################################################################################

	echo "$TRANSFER_SIZE,${COMPLETION_TIME[0]},${COMPLETION_TIME[1]},${COMPLETION_TIME[2]},${COMPLETION_TIME[3]}"

	NUM=$(( $NUM+1  ))
	# 		done
	# 	done
	# done
done

echo "Total $NUM experiments"
