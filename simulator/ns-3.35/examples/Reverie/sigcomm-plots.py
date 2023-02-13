#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Sun Feb 12 19:56:37 2023

@author: vamsi
"""

import numpy as np
import matplotlib.pyplot as plt
import math
from mpl_toolkits.mplot3d import Axes3D
import  random
from matplotlib.colors import LogNorm, Normalize
import pandas as pd
import numpy as np

#%%
dump="dump_sigcomm/"
plots="plots_sigcomm/"


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

# DUMP_DIR/evaluation-$alg-$RDMACC-$TCPCC-$rdmaload-$tcpload-$rdmaburst-$tcpburst-$egresslossyFrac-$gamma.fct

algs=[str(DT),str(ABM),str(REVERIE)]
loads=["0.2","0.4","0.6","0.8"]
loadsint=[0.2,0.4,0.6,0.8]

#%%
rdmacc=str(DCQCNCC)
tcpcc=str(CUBIC)
tcpload="0"
rdmaburst="1500000"
tcpburst="0"
egresslossyFrac="0.8"
gamma="0.999"

fig,ax=plt.subplots(1,1)

for alg in algs:
    shortfct99=list()
    shortfctavg=list()
    for rdmaload in loads:
        fctfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.fct'
        torfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.tor'
        outfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.out'
        
        fctDF = pd.read_csv(fctfile,delimiter=' ')
        shortfctDF = fctDF[(fctDF["incastflow"]==1)&(fctDF["priority"]==3)]
        
        shortfct = list(shortfctDF["slowdown"])
        fct99 = shortfct[int(len(shortfct)*0.99)]
        shortfct99.append(fct99)
        shortfctavg.append(np.mean(shortfct))
        
    # ax.plot(loadsint,shortfct99,label=alg)
    ax.plot(loadsint,shortfctavg,label=alg)

ax.legend()
ax.set_yscale('log')

#%%

rdmacc=str(DCQCNCC)
tcpcc=str(CUBIC)
tcpload="0"
rdmaload="0.4"
tcpburst="0"
egresslossyFrac="0.8"
gamma="0.999"

bursts=["100000", "500000", "1000000", "1500000", "2000000"]

fig,ax=plt.subplots(1,1)

for alg in algs:
    shortfct99=list()
    for rdmaburst in bursts:
        fctfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.fct'
        torfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.tor'
        outfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.out'
        
        fctDF = pd.read_csv(fctfile,delimiter=' ')
        shortfctDF = fctDF[(fctDF["incastflow"]==1)&(fctDF["priority"]==3)]
        
        shortfct = list(shortfctDF["slowdown"])
        fct99 = shortfct[int(len(shortfct)*0.99)]
        shortfct99.append(fct99)
        
    ax.plot(np.arange(len(bursts)),shortfct99,label=alg)

ax.legend()
# ax.set_yscale('log')

#%%
rdmacc=str(DCQCNCC)
tcpcc=str(CUBIC)
rdmaload="0"
tcpburst="0"
rdmaburst="2000000"
egresslossyFrac="0.8"
gamma="0.999"

fig,ax=plt.subplots(1,1)

for alg in algs:
    shortfct99=list()
    for tcpload in loads:
        fctfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.fct'
        torfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.tor'
        outfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.out'
        
        fctDF = pd.read_csv(fctfile,delimiter=' ')
        shortfctDF = fctDF[(fctDF["incastflow"]==1)&(fctDF["priority"]==3)]
        
        shortfct = list(shortfctDF["slowdown"])
        fct99 = shortfct[int(len(shortfct)*0.99)]
        shortfct99.append(fct99)
        
    ax.plot(loadsint,shortfct99,label=alg)

ax.legend()
# ax.set_yscale('log')