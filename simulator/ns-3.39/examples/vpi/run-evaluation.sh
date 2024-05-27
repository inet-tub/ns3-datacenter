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


TRANSFER_SIZES=(8000 16000 32000 64000 128000 256000)
QP_WINDOWS=(8 16 32 64 128 256)
############################################################################

RDMACC=$DCTCPCC

for TRANSFER_SIZE in ${TRANSFER_SIZES[@]};do
	
	for ROUTING in $FLOW_ECMP $RANDOM_ECMP $SOURCE_ROUTING;do

		if [[ $ROUTING == $RANDOM_ECMP ]];then
			MULTI_PATH="true"
		else
			MULTI_PATH="false"
		fi

		for QP_WINDOW in ${QP_WINDOWS[@]};do

			for QP_RANDOMIZE in "true" "false";do

				while [[ $(ps aux | grep single-vs-multi-path-optimized | wc -l) -gt $N_CORES ]];do
					sleep 30;
					echo "waiting for cores, $N_CORES running..."
				done
				FCTFILE=$DUMP_DIR/evaluation-$RDMACC-$MULTI_PATH-$ROUTING-$QP_RANDOMIZE-$QP_WINDOW-$TRANSFER_SIZE.fct
				TORFILE=$DUMP_DIR/evaluation-$RDMACC-$MULTI_PATH-$ROUTING-$QP_RANDOMIZE-$QP_WINDOW-$TRANSFER_SIZE.tor
				DUMPFILE=$DUMP_DIR/evaluation-$RDMACC-$MULTI_PATH-$ROUTING-$QP_RANDOMIZE-$QP_WINDOW-$TRANSFER_SIZE.out
				PFCFILE=$DUMP_DIR/evaluation-$RDMACC-$MULTI_PATH-$ROUTING-$QP_RANDOMIZE-$QP_WINDOW-$TRANSFER_SIZE.pfc
				echo $FCTFILE
				if [[ $EXP == 1 ]];then
					(time ./waf --run "single-vs-multi-path \
					    --bufferalgIngress=$BUFFER_ALG \
					    --bufferalgEgress=$BUFFER_ALG \
					    --rdmacc=$RDMACC \
					    --enableEcn=true \
					    --egressLossyShare=$egresslossyFrac \
					    --bufferModel=$BUFFERMODEL \
					    --START_TIME=$START_TIME \
					    --END_TIME=$END_TIME \
					    --buffersize=$BUFFERSIZE \
					    --fctOutFile=$FCTFILE \
					    --torOutFile=$TORFILE \
					    --alphasFile=$ALPHAFILE \
					    --pfcOutFile=$PFCFILE	\
					    --qpWindow=$QP_WINDOW	\
					    --qpRandomize=$QP_RANDOMIZE	\
					    --enableMultiPath=$MULTI_PATH	\
					    --routing=$ROUTING	\
					    --transferSize=$TRANSFER_SIZE	\
					    --collective=$COLLECTIVE	\
					    --collectiveAlgorithm=$ALG 	\
					    --rdmaRto=$rto" > $DUMPFILE 2> $DUMPFILE)&
					sleep 5
				fi
				NUM=$(( $NUM+1  ))
			done
		done
	done
done

echo "Total $NUM experiments"
