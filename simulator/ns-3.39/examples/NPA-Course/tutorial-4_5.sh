# define RENO 0
# define CUBIC 1
# define VEGAS 2
# define BBR 3

source config.sh
DIR=$NS3/examples/NPA-Course
TUT=/home/vamsi/src/phd/teaching/NPA-2023/tutorials/tutorial-4_5

TCPS=(0 1 2 3)
TCPNAMES=("Reno" "Cubic" "Vegas" "BBR")
BUFFERS=(100000 75000 50000 25000 10000 5000)

cd $NS3

BUFFER=140000
for INDEX in 0 1 2 3;do
	./waf --run "tutorial-4_5 --TcpProt=${TCPS[$INDEX]} --BufferSize=$BUFFER"
	cp $DIR/tutorial-4_cwnd_oneflow_0.csv $TUT/cwnd-${TCPNAMES[$INDEX]}-$BUFFER
	echo "cwnd-${TCPNAMES[$INDEX]}-$BUFFER"
done

for BUFFER in ${BUFFERS[@]};do
	./waf --run "tutorial-4_5 --TcpProt=0 --BufferSize=$BUFFER"
	cp $DIR/tutorial-4_cwnd_oneflow_0.csv $TUT/cwnd-Reno-$BUFFER
	echo "cwnd-Reno-$BUFFER"
done