source config.sh
# NS3=/home/vamsi/High-Precision-Congestion-Control/simulation
RES_DUMP=$NS3/examples/PowerTCP/dump_fairness
RES_RESULTS=$NS3/examples/PowerTCP/results_fairness

mkdir $RES_DUMP
mkdir $RES_RESULTS

algs=(0 1 2 3 4 5)

algNames=("dcqcn" "powerInt" "hpcc" "powerDelay" "timely" "dctcp")
CCMODE=(1 3 3 3 7 8)

# at the moment, power int and delay are called from hpcc ACK function separately and hence cc mode is still 3.

#--wien=true --delayWien=false

wien=false
delay=false

cd $NS3



######################################

# Topology and flows are specified in config file already, path to config file is also in .cc file. ToDo need to automate.

#####################################

N=1
for algorithm in ${algs[@]};do

	echo "evaluation-${algNames[$algorithm]}.out $N"
	N=$(( $N+1 ))
	RESULT_FILE="$RES_DUMP/evaluation-${algNames[$algorithm]}.out"
	# echo "time ./waf --run "evaluation-fairness --algorithm=${CCMODE[$algorithm]} --wien=$wien --delayWien=$delay --windowCheck=$window""
	cat $RESULT_FILE | grep 'Src 0 Total' > $RES_RESULTS/result-${algNames[$algorithm]}.1
	cat $RESULT_FILE | grep 'Src 1 Total' > $RES_RESULTS/result-${algNames[$algorithm]}.2
	cat $RESULT_FILE | grep 'Src 2 Total' > $RES_RESULTS/result-${algNames[$algorithm]}.3
	cat $RESULT_FILE | grep 'Src 3 Total' > $RES_RESULTS/result-${algNames[$algorithm]}.4
done



echo "##################################"
echo "#      FINISHED PARSING          #"
echo "##################################"
