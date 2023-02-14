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
dump="dump14/"
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


colors={}
colors[str(DT)]='red'
colors[str(ABM)]='blue'
colors[str(REVERIE)]='green'

markers={}
markers[str(DT)]='x'
markers[str(ABM)]='^'
markers[str(REVERIE)]='P'

names={}
names[str(DT)]="DT"
names[str(ABM)]="ABM"
names[str(REVERIE)]="Reverie"

algs=[str(DT),str(ABM),str(REVERIE)]
loads=["0.2","0.4","0.6","0.8"]
loadsint=[0.2,0.4,0.6,0.8]

bursts=["500000", "1000000"]
# bursts=["500000", "1000000","1500000", "2000000", "2500000"]

buffer=2610000

#%%
rdmacc=str(DCQCNCC)
tcpcc=str(CUBIC)
rdmaburst="2000000"
tcpburst="0"
egresslossyFrac="0.8"
gamma="0.999"

rdmaload="0"

fig0,ax0 = plt.subplots(1,1)
fig1,ax1 = plt.subplots(1,1)
fig2,ax2 = plt.subplots(1,1)
fig3,ax3 = plt.subplots(1,1)
fig4,ax4 = plt.subplots(1,1)
fig5,ax5 = plt.subplots(1,1)
fig6,ax6 = plt.subplots(1,1)
fig7,ax7 = plt.subplots(1,1)


for alg in algs:
    shortfct95=list()
    shortfct99=list()
    shortfct999=list()
    shortfctavg=list()
    tcpshortfct99=list()
    numpfc=list()
    lossy=list()
    lossless=list()
    total=list()
    for tcpload in loads:
        fctfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.fct'
        torfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.tor'
        outfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.out'
        pfcfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.pfc'
        
        fctDF = pd.read_csv(fctfile,delimiter=' ')
        shortfctDF = fctDF[(fctDF["incastflow"]==1)&(fctDF["priority"]==3)]
        
        tcpshortfctDF = fctDF[(fctDF["flowsize"]<100000)&(fctDF["priority"]==1)]
        shortfct = list(tcpshortfctDF["slowdown"])
        shortfct.sort()
        fct99 = shortfct[int(len(shortfct)*0.99)]
        tcpshortfct99.append(fct99)
        
        shortfct = list(shortfctDF["slowdown"])
        shortfct.sort()
        fct95 = shortfct[int(len(shortfct)*0.95)]
        shortfct95.append(fct95)
        fct99 = shortfct[int(len(shortfct)*0.99)]
        shortfct99.append(fct99)
        fct999 = shortfct[int(len(shortfct)*0.999)]
        shortfct999.append(fct999)
        shortfctavg.append(np.mean(shortfct))
        
        pfcDF = pd.read_csv(pfcfile,delimiter=' ')
        numpfc.append(len(pfcDF))
        
        torDF = pd.read_csv(torfile,delimiter=' ')
        lossybuf = torDF["egressOccupancyLossy"]
        losslessbuf = torDF["egressOccupancyLossless"]
        totalbuf = torDF["totalused"]
        lossy.append(np.max(100*lossybuf/buffer))
        lossless.append(np.max(100*losslessbuf/buffer))
        total.append(np.max(100*totalbuf/buffer))
        
    
    ax0.plot(loadsint,[i/1000 for i in numpfc],label=names[alg],lw=2,marker=markers[alg],c=colors[alg], markersize=10)
    ax1.plot(loadsint,shortfctavg,label=names[alg],lw=2,marker=markers[alg],c=colors[alg], markersize=10)
    ax2.plot(loadsint,shortfct95,label=names[alg],lw=2,marker=markers[alg],c=colors[alg], markersize=10)
    ax3.plot(loadsint,shortfct99,label=names[alg],lw=2,marker=markers[alg],c=colors[alg], markersize=10)
    ax4.plot(loadsint,shortfct999,label=names[alg],lw=2,marker=markers[alg],c=colors[alg], markersize=10)
    ax5.plot(loadsint,tcpshortfct99,label=names[alg],lw=2,marker=markers[alg],c=colors[alg], markersize=10)
    ax6.plot(loadsint,lossless,label=names[alg],lw=2,marker=markers[alg],c=colors[alg], markersize=10)
    ax7.plot(loadsint,lossy,label=names[alg],lw=2,marker=markers[alg],c=colors[alg], markersize=10)

ax0.legend()
ax1.legend()
ax2.legend()
ax3.legend()
ax4.legend()
ax5.legend()
ax6.legend()
ax7.legend()

ax0.xaxis.grid(True,ls='--')
ax1.xaxis.grid(True,ls='--')
ax2.xaxis.grid(True,ls='--')
ax3.xaxis.grid(True,ls='--')
ax4.xaxis.grid(True,ls='--')
ax5.xaxis.grid(True,ls='--')
ax6.xaxis.grid(True,ls='--')
ax7.xaxis.grid(True,ls='--')

ax0.yaxis.grid(True,ls='--')
ax1.yaxis.grid(True,ls='--')
ax2.yaxis.grid(True,ls='--')
ax3.yaxis.grid(True,ls='--')
ax4.yaxis.grid(True,ls='--')
ax5.yaxis.grid(True,ls='--')
ax6.yaxis.grid(True,ls='--')
ax7.yaxis.grid(True,ls='--')

ax0.set_xlabel("TCP load (%)")
ax1.set_xlabel("TCP load (%)")
ax2.set_xlabel("TCP load (%)")
ax3.set_xlabel("TCP load (%)")
ax4.set_xlabel("TCP load (%)")
ax5.set_xlabel("TCP load (%)")
ax6.set_xlabel("TCP load (%)")
ax7.set_xlabel("TCP load (%)")

ax0.set_ylabel("# PFC pauses (x1000)")
ax1.set_ylabel("Avg. FCT")
ax2.set_ylabel("95-pct FCT")
ax3.set_ylabel("99-pct FCT")
ax4.set_ylabel("99.9-pct FCT")
ax5.set_ylabel("99-pct FCT")
ax6.set_ylabel("99-pct buffer (Lossless)")
ax7.set_ylabel("99-pct buffer (Lossy)")


#%%

rdmacc=str(DCQCNCC)
tcpcc=str(CUBIC)
tcpburst="0"
tcpload="0.8"
egresslossyFrac="0.8"
gamma="0.999"

rdmaload="0"

fig0,ax0 = plt.subplots(1,1)
fig1,ax1 = plt.subplots(1,1)
fig2,ax2 = plt.subplots(1,1)
fig3,ax3 = plt.subplots(1,1)
fig4,ax4 = plt.subplots(1,1)
fig5,ax5 = plt.subplots(1,1)
fig6,ax6 = plt.subplots(1,1)
fig7,ax7 = plt.subplots(1,1)


for alg in algs:
    shortfct95=list()
    shortfct99=list()
    shortfct999=list()
    shortfctavg=list()
    tcpshortfct99=list()
    numpfc=list()
    lossy=list()
    lossless=list()
    total=list()
    for rdmaburst in bursts:
        fctfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.fct'
        torfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.tor'
        outfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.out'
        pfcfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.pfc'
        
        fctDF = pd.read_csv(fctfile,delimiter=' ')
        shortfctDF = fctDF[(fctDF["incastflow"]==1)&(fctDF["priority"]==3)]
        
        tcpshortfctDF = fctDF[(fctDF["flowsize"]<100000)&(fctDF["priority"]==1)]
        shortfct = list(tcpshortfctDF["slowdown"])
        shortfct.sort()
        fct99 = shortfct[int(len(shortfct)*0.99)]
        tcpshortfct99.append(fct99)
        
        shortfct = list(shortfctDF["slowdown"])
        shortfct.sort()
        fct95 = shortfct[int(len(shortfct)*0.95)]
        shortfct95.append(fct95)
        fct99 = shortfct[int(len(shortfct)*0.99)]
        shortfct99.append(fct99)
        fct999 = shortfct[int(len(shortfct)*0.999)]
        shortfct999.append(fct999)
        shortfctavg.append(np.mean(shortfct))
        
        pfcDF = pd.read_csv(pfcfile,delimiter=' ')
        numpfc.append(len(pfcDF))
        
        torDF = pd.read_csv(torfile,delimiter=' ')
        lossybuf = torDF["egressOccupancyLossy"]
        losslessbuf = torDF["egressOccupancyLossless"]
        totalbuf = torDF["totalused"]
        lossy.append(np.max(100*lossybuf/buffer))
        lossless.append(np.max(100*losslessbuf/buffer))
        total.append(np.max(100*totalbuf/buffer))
        
    
    ax0.plot(np.arange(len(bursts)),[i/1000 for i in numpfc],label=names[alg],lw=2,marker=markers[alg],c=colors[alg], markersize=10)
    ax1.plot(np.arange(len(bursts)),shortfctavg,label=names[alg],lw=2,marker=markers[alg],c=colors[alg], markersize=10)
    ax2.plot(np.arange(len(bursts)),shortfct95,label=names[alg],lw=2,marker=markers[alg],c=colors[alg], markersize=10)
    ax3.plot(np.arange(len(bursts)),shortfct99,label=names[alg],lw=2,marker=markers[alg],c=colors[alg], markersize=10)
    ax4.plot(np.arange(len(bursts)),shortfct999,label=names[alg],lw=2,marker=markers[alg],c=colors[alg], markersize=10)
    ax5.plot(np.arange(len(bursts)),tcpshortfct99,label=names[alg],lw=2,marker=markers[alg],c=colors[alg], markersize=10)
    ax6.plot(np.arange(len(bursts)),lossless,label=names[alg],lw=2,marker=markers[alg],c=colors[alg], markersize=10)
    ax7.plot(np.arange(len(bursts)),lossy,label=names[alg],lw=2,marker=markers[alg],c=colors[alg], markersize=10)

ax0.legend()
ax1.legend()
ax2.legend()
ax3.legend()
ax4.legend()
ax5.legend()
ax6.legend()
ax7.legend()

ax0.xaxis.grid(True,ls='--')
ax1.xaxis.grid(True,ls='--')
ax2.xaxis.grid(True,ls='--')
ax3.xaxis.grid(True,ls='--')
ax4.xaxis.grid(True,ls='--')
ax5.xaxis.grid(True,ls='--')
ax6.xaxis.grid(True,ls='--')
ax7.xaxis.grid(True,ls='--')

ax0.yaxis.grid(True,ls='--')
ax1.yaxis.grid(True,ls='--')
ax2.yaxis.grid(True,ls='--')
ax3.yaxis.grid(True,ls='--')
ax4.yaxis.grid(True,ls='--')
ax5.yaxis.grid(True,ls='--')
ax6.yaxis.grid(True,ls='--')
ax7.yaxis.grid(True,ls='--')

ax0.set_xlabel("TCP load (%)")
ax1.set_xlabel("TCP load (%)")
ax2.set_xlabel("TCP load (%)")
ax3.set_xlabel("TCP load (%)")
ax4.set_xlabel("TCP load (%)")
ax5.set_xlabel("TCP load (%)")
ax6.set_xlabel("TCP load (%)")
ax7.set_xlabel("TCP load (%)")

ax0.set_ylabel("# PFC pauses (x1000)")
ax1.set_ylabel("Avg. FCT")
ax2.set_ylabel("95-pct FCT")
ax3.set_ylabel("99-pct FCT")
ax4.set_ylabel("99.9-pct FCT")
ax5.set_ylabel("99-pct FCT")
ax6.set_ylabel("99-pct buffer (Lossless)")
ax7.set_ylabel("99-pct buffer (Lossy)")

#%%
rdmacc=str(DCQCNCC)
tcpcc=str(CUBIC)
# tcpburst="2000000"
tcpload="0"
rdmaload="0.8"

rdmaburst="0"
egresslossyFrac="0.8"
gamma="0.999"

fig,ax=plt.subplots(1,1)

loads=["0","0.2","0.4","0.6","0.8"]
loadsint=[0,0.2,0.4,0.6,0.8]

for alg in algs:
    shortfct99=list()
    for tcpburst in bursts:
        fctfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.fct'
        torfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.tor'
        outfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.out'
        
        fctDF = pd.read_csv(fctfile,delimiter=' ')
        shortfctDF = fctDF[(fctDF["incastflow"]==1)&(fctDF["priority"]==1)&(fctDF["timestamp"]<2)]
        
        shortfct = list(shortfctDF["slowdown"])
        shortfct.sort()
        fct99 = shortfct[int(len(shortfct)*0.99)]
        shortfct99.append(fct99)
        
    ax.plot(np.arange(len(bursts)),shortfct99,label=alg)

ax.legend()
# ax.set_yscale('log')