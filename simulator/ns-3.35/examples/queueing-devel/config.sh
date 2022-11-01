TEMPDIR=$(pwd)
export NS3=$(pwd)/../..
cd $NS3 # dirty hack to get absolute path instead of relative
export NS3=$(pwd) 
export N_CORES=37
cd $TEMPDIR 