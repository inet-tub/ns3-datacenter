source config.sh
DIR=$(pwd)
DUMP_DIR=$DIR/dump_queueing
RESULTS_DIR=$DIR/results_queueing

if [ ! -d "$DUMP_DIR" ];then
	mkdir $DUMP_DIR
fi
if [ ! -d "$RESULTS_DIR" ];then
	mkdir $RESULTS_DIR
fi

cd $NS3

LOSSLESS=0
LOSSY=1

DT=101
FAB=102
ABM=110
REVERIE=111

DCQCNCC=1
INTCC=3
TIMELYCC=7
PINTCC=10
CUBIC=2
DCTCP=4


NUM=0

# BUFFER_ALGS=($DT $FAB $ABM "reverie")
BUFFER_ALGS=($DT)

BURST_SIZES=(0 500000 1000000 1500000 2000000 2500000)

LOADS=(0.2 0.4 0.6 0.8)

egresslossyFrac=0.8

gamma=0.999

START_TIME=1
END_TIME=6
FLOW_LAUNCH_END_TIME=5
BUFFER_PER_PORT_PER_GBPS=5.12 # in KiloBytes per port per Gbps
BUFFERSIZE=$(python3 -c "print(8*25*1000*$BUFFER_PER_PORT_PER_GBPS)") # in Bytes
ALPHAFILE=$DIR/alphas

EXP=$1

RDMAREQRATE=2
TCPREQRATE=2

############################################################################
######### Pure RDMA with a fixed burst size, across loads ########
rdmaburst=0
tcpload=0
tcpburst=0
RDMACC=$DCQCNCC
TCPCC=$CUBIC
alg=$DT
for rdmaload in ${LOADS[@]};do
	# tcpload=$(python3 -c "print('%.1f'%(0.8-$rdmaload))")
	for RDMACC in $DCQCNCC $INTCC $TIMELYCC ;do
		if [[ $RDMACC == $INTCC ]];then
			POWERTCP=true
		else
			POWERTCP=false
		fi

		if [[ $alg != $REVERIE ]];then
			BUFFERMODEL="sonic"
		else
			BUFFERMODEL="reverie"
		fi
		while [[ $(ps aux | grep queueing-optimized | wc -l) -gt $N_CORES ]];do
			sleep 30;
			echo "waiting for cores, $N_CORES running..."
		done
		DUMPFILE=$DUMP_DIR/queueing-$RDMACC-$rdmaload.out
		CHECK=$(cat $DUMPFILE | head -n1 | awk '{print $1}')
		if [[ $CHECK == "arrival" ]];then
			echo $DUMPFILE
			continue
		fi
		sed -i 1,17d $DUMPFILE
		NUM=$(( $NUM+1  ))
	done
done

echo "Total $NUM experiments"