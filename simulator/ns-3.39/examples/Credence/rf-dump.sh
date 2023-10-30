source config.sh
DIR="$NS3/examples/Credence"
LQD_DIR="$DIR/lqd-logs"
# LQD_DIR="/home/vamsi/src/phd/codebase/ns3-datacenter-Old/simulator/ns-3.35/examples/Credence/lqd-logs"
DUMP_DIR="$DIR/rf_models"

if [ ! -d "$DUMP_DIR" ];then
	mkdir $DUMP_DIR
fi
if [ ! -d "$LQD_DIR" ];then
	mkdir $LQD_DIR
fi

PYTHON="/usr/bin/python3.11"

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

CDFNAME="WS"

N=0

BURSTS=(0.75 0.5 0.875 1.0 1.25)
BURST_FREQ=2

LOAD=0.8
TCP=$DCTCP
ALG=$LQD

MAX_DEPTH=4
N_ESTIMATORS=8

BURST_SIZES=0.75
for SWITCH in 0 1 2 3 4 5;do
	DUMPSCOREFILE="$DUMP_DIR/model-$TCP-$LOAD-$BURST_SIZES-$BURST_FREQ-$CDFNAME-$SWITCH.score"
	echo "accuracy precision recall f1score trees depth errorinv" > $DUMPSCOREFILE
	LQDOUTFILE="$LQD_DIR/lqdtrace-$TCP-$LOAD-$BURST_SIZES-$BURST_FREQ-$CDFNAME-$SWITCH.csv"
	DUMPMODELFILE="$DUMP_DIR/model-$TCP-$LOAD-$BURST_SIZES-$BURST_FREQ-$CDFNAME"
	N=$(( $N+1 ))
	($PYTHON trainLqd.py $LQDOUTFILE $DUMPMODELFILE $MAX_DEPTH $SWITCH >> $DUMPSCOREFILE 2> /dev/null)&
	echo "$BURST_SIZES $SWITCH"
	cat $DUMPSCOREFILE
	sleep 2
done