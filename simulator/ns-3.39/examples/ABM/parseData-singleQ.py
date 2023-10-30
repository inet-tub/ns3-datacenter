import os
import sys
import numpy as np
import pandas as pd


flowFile=str(sys.argv[1])
torFile=str(sys.argv[2])
linerate=int(sys.argv[3])
rtt=float(sys.argv[4])
bdp=linerate*1e9*rtt*1e-6/8
load=float(sys.argv[5])
burst=float(sys.argv[6])
alg=str(sys.argv[7])
tcp=str(sys.argv[8])
scenario=str(sys.argv[9])
nprio=int(sys.argv[10])

# *fctOutput->GetStream () 
# "time " 
# "flowsize " 
# "fct "  
# "basefct " 
# "slowdown " 
# "basertt "  
# "flowstart "
# "priority "
# "incast "

# *torStats->GetStream ()
# "time "
# "tor " 
# "bufferSizeMB " 
# "occupiedBufferPct "
# "uplinkThroughput "  
# "priority0 "
# "priority1 "
# "priority2 "
# "priority3 "
# "priority4 "
# "priority5 "
# "priority6 "
# "priority7 "

flowDf=pd.read_csv(flowFile,delimiter=' ')

try:
	shortFlows=flowDf[(flowDf["flowsize"]<bdp)&(flowDf["incast"]==0)]
	shortFlows=list(shortFlows["slowdown"])
	shortFlows.sort()
	short999fct = shortFlows[int(len(shortFlows)*0.999)-1] # 1
	short99fct  = shortFlows[int(len(shortFlows)*0.99)-1] # 2
	short95fct  = shortFlows[int(len(shortFlows)*0.95)-1] # 3
	shortavgfct = np.mean(shortFlows) # 4
	shortmedfct = np.median(shortFlows) # 5
except:
	short999fct = 0 # 1
	short99fct  = 0 # 2
	short95fct  = 0 # 3
	shortavgfct = 0 # 4
	shortmedfct = 0 # 5

try:
	incastFlows=flowDf[flowDf["incast"]==1]
	incastFlows=list(incastFlows["slowdown"])
	incastFlows.sort()
	incast999fct = incastFlows[int(len(incastFlows)*0.999)-1] # 6
	incast99fct = incastFlows[int(len(incastFlows)*0.99)-1] # 7
	incast95fct = incastFlows[int(len(incastFlows)*0.95)-1] # 8
	incastavgfct = np.mean(incastFlows) # 9
	incastmedfct = np.median(incastFlows) # 10
except:
	incast999fct = 0 # 6
	incast99fct  = 0 # 7
	incast95fct  = 0 # 8
	incastavgfct = 0 # 9
	incastmedfct =  0 # 10

try:
	longFlows=flowDf[flowDf["flowsize"]>=30000000]
	longFlows=list(longFlows["slowdown"])
	longFlows.sort()
	long999fct = longFlows[int(len(longFlows)*0.999)-1] # 11
	long99fct  = longFlows[int(len(longFlows)*0.99)-1] # 12
	long95fct  = longFlows[int(len(longFlows)*0.95)-1] # 13
	longavgfct = np.mean(longFlows) # 14
	longmedfct = np.median(longFlows) # 15
except:
	long999fct = 0 # 11
	long99fct  = 0 # 12
	long95fct  = 0 # 13
	longavgfct = 0 # 14
	longmedfct = 0 # 15
# "time " 	1
# "tor " 	2
# "bufferSizeMB " 3 
# "occupiedBufferPct " 4
# "uplinkThroughput "  5
# "priority0 " 6
# "priority1 " 7
# "priority2 " 8
# "priority3 " 9
# "priority4 " 10
# "priority5 " 11
# "priority6 " 12
# "priority7 " 13
torDf=pd.read_csv(torFile,delimiter=' ',usecols=[1,2,3,4,5,6,7,8,9,10,11,12,13],names=["time","tor","bufferSizeMB","occupiedBufferPct","uplinkThroughput","priority0","priority1","priority2","priority3","priority4","priority5","priority6","priority7"],header=1)
torDf=torDf[(torDf["time"]>10)&(torDf["time"]<13)]

throughput = list(torDf["uplinkThroughput"])
throughput.sort()
avgTh = np.mean(throughput) # 16
medTh = np.median(throughput) # 17

buffer = list(torDf["occupiedBufferPct"])
buffer.sort()
bufmax = buffer[len(buffer)-1] # 18
buf999 = buffer[int(len(buffer)*0.999)-1] # 19
buf99 = buffer[int(len(buffer)*0.99)-1] # 20
buf95 = buffer[int(len(buffer)*0.95)-1] # 21
avgBuf = np.mean(buffer) # 22
medBuf = np.median(buffer) # 23

# print("short999fct","short99fct","short95fct","shortavgfct","shortmedfct","incast999fct","incast99fct","incast95fct","incastavgfct","incastmedfct","long999fct","long99fct","long95fct","longavgfct","longmedfct","avgTh","medTh","bufmax","buf999","buf99","buf95","avgBuf","medBuf")
print(short999fct,short99fct,short95fct,shortavgfct,shortmedfct,incast999fct,incast99fct,incast95fct,incastavgfct,incastmedfct,long999fct,long99fct,long95fct,longavgfct,longmedfct,avgTh,medTh,bufmax,buf999,buf99,buf95,avgBuf,medBuf,load,burst,alg,tcp,scenario,nprio)
