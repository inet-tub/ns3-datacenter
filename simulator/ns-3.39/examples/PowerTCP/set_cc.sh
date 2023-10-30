source config.sh
DIR=$NS3/examples/PowerTCP/
VAR=$1
VAL=$2
sed -i -e "s/$VAR .*/$VAR $VAL/g" $DIR/config.txt
echo "$(cat $DIR/config.txt | grep $VAR)"
