source config.sh
DIR=$(pwd)
DUMP_DIR=$DIR/dump_sigcomm
RESULTS_DIR=$DIR/results_sigcomm

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
BUFFER_ALGS=($DT $ABM "reverie")

BURST_SIZES=(0 100000 500000 1000000 1500000 2000000)

LOADS=(0.2 0.4 0.6 0.8)

egresslossyFrac=0.8

gamma=0.999

START_TIME=1
END_TIME=5
FLOW_LAUNCH_END_TIME=4
BUFFER_PER_PORT_PER_GBPS=5.12 # in KiloBytes per port per Gbps
BUFFERSIZE=$(python3 -c "print(40*25*1000*$BUFFER_PER_PORT_PER_GBPS)") # in Bytes
ALPHAFILE=$DIR/alphas

EXP=$1

############################################################################
######### Pure RDMA with a fixed burst size, across loads ########
rdmaburst=1500000
tcpload=0
tcpburst=0
RDMACC=$DCQCNCC
TCPCC=$CUBIC
for rdmaload in ${LOADS[@]};do
	# tcpload=$(python3 -c "print('%.1f'%(0.8-$rdmaload))")
	for alg in ${BUFFER_ALGS[@]};do
		if [[ $alg != $REVERIE ]];then
			BUFFERMODEL="sonic"
		else
			BUFFERMODEL="reverie"
		fi
		while [[ $(ps aux | grep reverie-evaluation-sigcomm2023-optimized | wc -l) -gt $N_CORES ]];do
			sleep 30;
			echo "waiting for cores, $N_CORES running..."
		done
		FCTFILE=evaluation-$alg-$RDMACC-$TCPCC-$rdmaload-$tcpload-$rdmaburst-$tcpburst-$egresslossyFrac-$gamma.fct
		TORFILE=evaluation-$alg-$RDMACC-$TCPCC-$rdmaload-$tcpload-$rdmaburst-$tcpburst-$egresslossyFrac-$gamma.tor
		DUMPFILE=evaluation-$alg-$RDMACC-$TCPCC-$rdmaload-$tcpload-$rdmaburst-$tcpburst-$egresslossyFrac-$gamma.out
		echo $FCTFILE
		if [[ $EXP == 1 ]];then
			(time ./waf --run "reverie-evaluation-sigcomm2023 --bufferalgIngress=$alg --bufferalgEgress=$alg --rdmacc=$RDMACC --rdmaload=$rdmaload --rdmarequestSize=$rdmaburst --rdmaqueryRequestRate=1 --tcpload=$tcpload --tcpcc=$TCPCC --enableEcn=true --tcpqueryRequestRate=1 --tcprequestSize=$tcpburst --egressLossyShare=$egresslossyFrac --bufferModel=$BUFFERMODEL --gamma=$gamma --START_TIME=$START_TIME --END_TIME=$END_TIME --FLOW_LAUNCH_END_TIME=$FLOW_LAUNCH_END_TIME --buffersize=$BUFFERSIZE --fctOutFile=$FCTFILE --torOutFile=$TORFILE --alphasFile=$ALPHAFILE" > $DUMPFILE 2> $DUMPFILE)&
			sleep 5
		fi
		NUM=$(( $NUM+1  ))
	done
done

###### Pure RDMA low load, across bursts with default DCQCN
rdmaload=0.4
tcpload=0
tcpburst=0
RDMACC=$DCQCNCC
TCPCC=$CUBIC
for rdmaburst in ${BURST_SIZES[@]};do
	if [[ $rdmaburst == 1500000 ]];then
		continue;
	fi
	for alg in ${BUFFER_ALGS[@]};do
		if [[ $alg != $REVERIE ]];then
			BUFFERMODEL="sonic"
		else
			BUFFERMODEL="reverie"
		fi
		while [[ $(ps aux | grep reverie-evaluation-sigcomm2023-optimized | wc -l) -gt $N_CORES ]];do
			sleep 30;
			echo "waiting for cores, $N_CORES running..."
		done
		FCTFILE=evaluation-$alg-$RDMACC-$TCPCC-$rdmaload-$tcpload-$rdmaburst-$tcpburst-$egresslossyFrac-$gamma.fct
		TORFILE=evaluation-$alg-$RDMACC-$TCPCC-$rdmaload-$tcpload-$rdmaburst-$tcpburst-$egresslossyFrac-$gamma.tor
		echo $FCTFILE
		if [[ $EXP == 1 ]];then
			(time ./waf --run "reverie-evaluation-sigcomm2023 --bufferalgIngress=$alg --bufferalgEgress=$alg --rdmacc=$RDMACC --rdmaload=$rdmaload --rdmarequestSize=$rdmaburst --rdmaqueryRequestRate=1 --tcpload=$tcpload --tcpcc=$TCPCC --enableEcn=true --tcpqueryRequestRate=1 --tcprequestSize=$tcpburst --egressLossyShare=$egresslossyFrac --bufferModel=$BUFFERMODEL --gamma=$gamma --START_TIME=$START_TIME --END_TIME=$END_TIME --FLOW_LAUNCH_END_TIME=$FLOW_LAUNCH_END_TIME --buffersize=$BUFFERSIZE --fctOutFile=$FCTFILE --torOutFile=$TORFILE --alphasFile=$ALPHAFILE" > $DUMPFILE 2> $DUMPFILE)&
			sleep 5
		fi
		NUM=$(( $NUM+1  ))
	done
done


###### Pure RDMA low load, across bursts with PowerTCP
rdmaload=0.4
tcpload=0
tcpburst=0
RDMACC=$INTCC
TCPCC=$CUBIC
for rdmaburst in ${BURST_SIZES[@]};do
	if [[ $burst == 1500000 ]];then
		continue;
	fi
	for alg in ${BUFFER_ALGS[@]};do

		if [[ $alg != $REVERIE ]];then
			BUFFERMODEL="sonic"
		else
			BUFFERMODEL="reverie"
		fi
		while [[ $(ps aux | grep reverie-evaluation-sigcomm2023-optimized | wc -l) -gt $N_CORES ]];do
			sleep 30;
			echo "waiting for cores, $N_CORES running..."
		done
		FCTFILE=evaluation-$alg-$RDMACC-$TCPCC-$rdmaload-$tcpload-$rdmaburst-$tcpburst-$egresslossyFrac-$gamma.fct
		TORFILE=evaluation-$alg-$RDMACC-$TCPCC-$rdmaload-$tcpload-$rdmaburst-$tcpburst-$egresslossyFrac-$gamma.tor
		echo $FCTFILE
		if [[ $EXP == 1 ]];then
			(time ./waf --run "reverie-evaluation-sigcomm2023 --bufferalgIngress=$alg --bufferalgEgress=$alg --rdmacc=$RDMACC --powertcp=true --rdmaload=$rdmaload --rdmarequestSize=$rdmaburst --rdmaqueryRequestRate=1 --tcpload=$tcpload --tcpcc=$TCPCC --enableEcn=true --tcpqueryRequestRate=1 --tcprequestSize=$tcpburst --egressLossyShare=$egresslossyFrac --bufferModel=$BUFFERMODEL --gamma=$gamma --START_TIME=$START_TIME --END_TIME=$END_TIME --FLOW_LAUNCH_END_TIME=$FLOW_LAUNCH_END_TIME --buffersize=$BUFFERSIZE --fctOutFile=$FCTFILE --torOutFile=$TORFILE --alphasFile=$ALPHAFILE" > $DUMPFILE 2> $DUMPFILE)&
			sleep 5
		fi
		
		NUM=$(( $NUM+1  ))
	done
done

echo "PureRDMA $NUM experiments"
RDMANUM=$NUM
############################################################################
######## Pure TCP with a fixed burst size, across loads
tcpburst=1500000
rdmaload=0
RDMACC=$DCQCNCC
TCPCC=$CUBIC
tcpburst=0
 # --rdmarequestSize=$rdmaburst
for tcpload in ${LOADS[@]};do
	# tcpload=$(python3 -c "print('%.1f'%(0.8-$rdmaload))")
	for alg in ${BUFFER_ALGS[@]};do

		if [[ $alg != $REVERIE ]];then
			BUFFERMODEL="sonic"
		else
			BUFFERMODEL="reverie"
		fi
		while [[ $(ps aux | grep reverie-evaluation-sigcomm2023-optimized | wc -l) -gt $N_CORES ]];do
			sleep 30;
			echo "waiting for cores, $N_CORES running..."
		done
		FCTFILE=evaluation-$alg-$RDMACC-$TCPCC-$rdmaload-$tcpload-$rdmaburst-$tcpburst-$egresslossyFrac-$gamma.fct
		TORFILE=evaluation-$alg-$RDMACC-$TCPCC-$rdmaload-$tcpload-$rdmaburst-$tcpburst-$egresslossyFrac-$gamma.tor
		echo $FCTFILE
		if [[ $EXP == 1 ]];then
			(time ./waf --run "reverie-evaluation-sigcomm2023 --bufferalgIngress=$alg --bufferalgEgress=$alg --rdmacc=$RDMACC --rdmaload=$rdmaload --rdmarequestSize=$rdmaburst --rdmaqueryRequestRate=1 --tcpload=$tcpload --tcpcc=$TCPCC --enableEcn=true --tcpqueryRequestRate=1 --tcprequestSize=$tcpburst --egressLossyShare=$egresslossyFrac --bufferModel=$BUFFERMODEL --gamma=$gamma --START_TIME=$START_TIME --END_TIME=$END_TIME --FLOW_LAUNCH_END_TIME=$FLOW_LAUNCH_END_TIME --buffersize=$BUFFERSIZE --fctOutFile=$FCTFILE --torOutFile=$TORFILE --alphasFile=$ALPHAFILE" > $DUMPFILE 2> $DUMPFILE)&
			sleep 5
		fi

		NUM=$(( $NUM+1  ))
	done
done

echo "PureTCP $(( $NUM - $RDMANUM )) experiments"

echo "Total $NUM experiments"