NS3=/home/vamsi/src/phd/ns3-datacenter/simulator/ns-3.35/
RES_DUMP=$NS3/examples/PowerTCP/dump_workload
RES_RESULTS=$NS3/examples/PowerTCP/results_workload

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



START=0.1
END=10
FLOWEND=9

LOADS=(0.2 0.4 0.6 0.8 0.9 0.95)

REQ_RATE=(1 4 8 16)
REQ_SIZE=(1000000 2000000 4000000 6000000 8000000 1000000)

######################################

# Topology and flows are specified in config file already, path to config file is also in .cc file. ToDo need to automate.

#####################################

N=1
# echo "start"

# for req in ${REQ_RATE[@]};do
# 	for load in ${LOADS[@]};do
# 		for algorithm in ${algs[@]};do

# 			echo "evaluation-${algNames[$algorithm]}-$load-$req.out $N"
# 			N=$(( $N+1 ))
# 			RESULT_FILE="$RES_DUMP/evaluation-${algNames[$algorithm]}-$load-$req.out"
# 			# echo "time ./waf --run "evaluation-fairness --algorithm=${CCMODE[$algorithm]} --wien=$wien --delayWien=$delay --windowCheck=$window""
# 			# time ./waf --run "evaluation-workload --algorithm=${CCMODE[$algorithm]} --wien=$wien --delayWien=$delay --windowCheck=$window --queryRequestRate=$req --load=$load --START_TIME=$START --END_TIME=$END --FLOW_LAUNCH_END_TIME=$FLOWEND --incast=5" > $RESULT_FILE  2> $RESULT_FILE &
# 			cat $RESULT_FILE | grep 'FCT' | grep 'flowSize' > $RES_RESULTS/result-${algNames[$algorithm]}-$load-$req.fct
# 			cat $RESULT_FILE | grep 'switch 0' | grep 'time' > $RES_RESULTS/result-${algNames[$algorithm]}-$load-$req.buf
# 		done
# 	done
# done



# req=0
# query=0
# for load in ${LOADS[@]};do
# 	for algorithm in ${algs[@]};do

# 		echo "evaluation-${algNames[$algorithm]}-$load-$req-$query.out $N"
# 		N=$(( $N+1 ))
# 		RESULT_FILE="$RES_DUMP/evaluation-${algNames[$algorithm]}-$load-$req-$query.out"
		
# 		cat $RESULT_FILE | grep 'FCT' | grep 'size' > $RES_RESULTS/result-${algNames[$algorithm]}-$load-$req-$query.fct
# 		cat $RESULT_FILE | grep 'switch 0'| grep 'qlen' > $RES_RESULTS/result-${algNames[$algorithm]}-$load-$req-$query.buf
# 	done
# done

# echo "#######################################"
# echo "#      FINISHED LOAD EXPERIMENTS      #"
# echo "#######################################"

# query=$(( 2*1000*1000 ))
# load="0.8"
# for req in ${REQ_RATE[@]};do
# 	for algorithm in ${algs[@]};do

# 		echo "evaluation-${algNames[$algorithm]}-$load-$req-$query.out $N"
# 		N=$(( $N+1 ))
# 		RESULT_FILE="$RES_DUMP/evaluation-${algNames[$algorithm]}-$load-$req-$query.out"
		
# 		cat $RESULT_FILE | grep 'FCT' | grep 'size' > $RES_RESULTS/result-${algNames[$algorithm]}-$load-$req-$query.fct
# 		cat $RESULT_FILE | grep 'switch 0'| grep 'qlen' > $RES_RESULTS/result-${algNames[$algorithm]}-$load-$req-$query.buf
# 	done
# done

# echo "#######################################"
# echo "#      FINISHED RATE EXPERIMENTS      #"
# echo "#######################################"



req="4"
load="0.8"
for query in ${REQ_SIZE[@]};do
	for algorithm in ${algs[@]};do

		echo "evaluation-${algNames[$algorithm]}-$load-$req-$query.out $N"
		N=$(( $N+1 ))
		RESULT_FILE="$RES_DUMP/evaluation-${algNames[$algorithm]}-$load-$req-$query.out"
		
		cat $RESULT_FILE | grep 'FCT' | grep 'size' > $RES_RESULTS/result-${algNames[$algorithm]}-$load-$req-$query.fct
		cat $RESULT_FILE | grep 'switch 0'| grep 'qlen' > $RES_RESULTS/result-${algNames[$algorithm]}-$load-$req-$query.buf
	done
done

echo "##########################################"
echo "#      FINISHED BURST-4 EXPERIMENTS      #"
echo "##########################################"


req="1"
load="0.8"
for query in ${REQ_SIZE[@]};do
	for algorithm in ${algs[@]};do

		echo "evaluation-${algNames[$algorithm]}-$load-$req-$query.out $N"
		N=$(( $N+1 ))
		RESULT_FILE="$RES_DUMP/evaluation-${algNames[$algorithm]}-$load-$req-$query.out"
		
		cat $RESULT_FILE | grep 'FCT' | grep 'size' > $RES_RESULTS/result-${algNames[$algorithm]}-$load-$req-$query.fct
		cat $RESULT_FILE | grep 'switch 0'| grep 'qlen' > $RES_RESULTS/result-${algNames[$algorithm]}-$load-$req-$query.buf
	done
done

echo "##########################################"
echo "#      FINISHED BURST-1 EXPERIMENTS      #"
echo "##########################################"
