source config.sh
BUFFER_DIR="$NS3/examples/buffer-devel"
echo "BUFFER_DIR=$NS3/examples/buffer-devel NS3=$NS3"
DUMP_DIR="$BUFFER_DIR/dump_test"
mkdir $DUMP_DIR
DT=101
FAB=102
CS=103
IB=104
ABM=110

SONIC="sonic"
NEW="new"

END_TIME=0.030

SETUP="losslessfirst"

cd $NS3

for ALG in $DT $ABM;do
	for BUFFERMODEL in $SONIC $NEW; do
		for ALPHA in 0.25 0.5 1 2 3 4;do
			ALPHAFILE="$BUFFER_DIR/alphas$ALPHA"
			FCTFILE="$DUMP_DIR/fct-$ALG-$ALPHA-$BUFFERMODEL-$SETUP.fct"
			TORFILE="$DUMP_DIR/tor-$ALG-$ALPHA-$BUFFERMODEL-$SETUP.tor"
			PFCFILE="$DUMP_DIR/pfc-$ALG-$ALPHA-$BUFFERMODEL-$SETUP.pfc"
			OUTFILE="$DUMP_DIR/dump-$ALG-$ALPHA-$BUFFERMODEL-$SETUP.out"
			(time ./waf --run "test-mixed --bufferModel=$BUFFERMODEL --END_TIME=$END_TIME --bufferalgIngress=$ALG --bufferalgEgress=$ALG --alphasFile=$ALPHAFILE --fctOutFile=$FCTFILE --torOutFile=$TORFILE --pfcOutFile=$PFCFILE" > $OUTFILE ; echo "$FLOWFILE")&
			sleep 5
		done
	done
done