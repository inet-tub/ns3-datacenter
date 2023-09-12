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
import sys

#%%
dump="dump_sigcomm/"
# plots="plots_sigcomm/"
plots="/home/vamsi/src/phd/writings/reverie/nsdi24/plots/evaluation/"


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

# bursts=["500000", "1000000"]
bursts=["500000", "1000000","1500000", "2000000", "2500000"]

buffer=2610000

plt.rcParams.update({'font.size': 18})

# print ("alg","rdmacc","tcpcc","rdmaload","tcpload","rdmaburst","tcpburst","egresslossyFrac","gamma", "numpfc","shortfctavg","shortfct95","shortfct99","shortfct999","tcpshortfct99","lossless","lossy","longfctav","medfctav")

#%%

dump="dump_sigcomm/"

loads=["0.2","0.4","0.6","0.8"]
loadsint=[0.2,0.4,0.6,0.8]

# bursts=["500000", "1000000"]
bursts=["500000", "1000000","1500000", "2000000", "2500000"]

rdmacc=str(DCQCNCC)
tcpcc=str(CUBIC)
rdmaburst="2000000"
tcpburst="0"
egresslossyFrac="0.8"
gamma="0.999"

rdmaload="0"

fig0,ax0 = plt.subplots(1,1,figsize=(4,5))
fig1,ax1 = plt.subplots(1,1,figsize=(4,5))
fig2,ax2 = plt.subplots(1,1,figsize=(4,5))
fig3,ax3 = plt.subplots(1,1,figsize=(4,5))
fig4,ax4 = plt.subplots(1,1,figsize=(4,5))
fig5,ax5 = plt.subplots(1,1,figsize=(4,5))
fig6,ax6 = plt.subplots(1,1,figsize=(4,5))
fig7,ax7 = plt.subplots(1,1,figsize=(4,5))
fig8,ax8 = plt.subplots(1,1,figsize=(4,5))
fig9,ax9 = plt.subplots(1,1,figsize=(4,5))


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
    longfctav=list()
    medfctav=list()
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
        fct99 = shortfct[int(len(shortfct)*0.99)-1]
        tcpshortfct99.append(fct99)
        
        shortfct = list(shortfctDF["slowdown"])
        shortfct.sort()
        fct95 = shortfct[int(len(shortfct)*0.95)-1]
        shortfct95.append(fct95)
        fct99 = shortfct[int(len(shortfct)*0.99)-1]
        shortfct99.append(fct99)
        fct999 = shortfct[int(len(shortfct)*0.999)-1]
        shortfct999.append(fct999)
        shortfctavg.append(np.mean(shortfct))
        
        pfcDF = pd.read_csv(pfcfile,delimiter=' ')
        numpfc.append(len(pfcDF))
        
        torDF = pd.read_csv(torfile,delimiter=' ')
        lossybuf = torDF["egressOccupancyLossy"]
        losslessbuf = torDF["egressOccupancyLossless"]
        totalbuf = torDF["totalused"]
        lossybuf = list(100*lossybuf/buffer)
        losslessbuf = list(100*losslessbuf/buffer)
        totalbuf = list(100*totalbuf/buffer)
        lossybuf.sort()
        losslessbuf.sort()
        totalbuf.sort()
        lossy.append(lossybuf[int(len(lossybuf)*0.99)])
        lossless.append(losslessbuf[int(len(lossybuf)*0.99)])
        total.append(totalbuf[int(len(lossybuf)*0.99)])
        
        longfctDF = fctDF[(fctDF["flowsize"]>1000000)&(fctDF["priority"])==1]
        longfct = list(longfctDF["slowdown"])
        longfct.sort()
        # longfctav.append(longfct[int(len(longfct)*0.99)])
        longfctav.append(np.mean(longfct))
        
        
        medfctDF = fctDF[(fctDF["flowsize"]<1000000)&(fctDF["flowsize"]>100000)&(fctDF["priority"])==1]
        medfct = list(longfctDF["slowdown"])
        medfct.sort()
        # medfctav.append(medfct[int(len(medfct)*0.99)])
        medfctav.append(np.mean(medfct))
        
        # print (alg,rdmacc,tcpcc,rdmaload,tcpload,rdmaburst,tcpburst,egresslossyFrac,gamma, numpfc[-1],shortfctavg[-1],shortfct95[-1],shortfct99[-1],shortfct999[-1],tcpshortfct99[-1],lossless[-1],lossy[-1],0,0)
        
    
    ax0.plot(loadsint,[i/1000 for i in numpfc],label=names[alg],lw=4,marker=markers[alg],c=colors[alg], markersize=20)
    ax1.plot(loadsint,shortfctavg,label=names[alg],lw=4,marker=markers[alg],c=colors[alg], markersize=20)
    ax2.plot(loadsint,shortfct95,label=names[alg],lw=4,marker=markers[alg],c=colors[alg], markersize=20)
    ax3.plot(loadsint,shortfct99,label=names[alg],lw=4,marker=markers[alg],c=colors[alg], markersize=20)
    ax4.plot(loadsint,shortfct999,label=names[alg],lw=4,marker=markers[alg],c=colors[alg], markersize=20)
    ax5.plot(loadsint,tcpshortfct99,label=names[alg],lw=4,marker=markers[alg],c=colors[alg], markersize=20)
    ax6.plot(loadsint,lossless,label=names[alg],lw=4,marker=markers[alg],c=colors[alg], markersize=20)
    ax7.plot(loadsint,lossy,label=names[alg],lw=4,marker=markers[alg],c=colors[alg], markersize=20)
    ax8.plot(loadsint,longfctav,label=names[alg],lw=4,marker=markers[alg],c=colors[alg], markersize=20)
    ax9.plot(loadsint,medfctav,label=names[alg],lw=4,marker=markers[alg],c=colors[alg], markersize=20)

# ax0.legend()
# ax1.legend()
# ax2.legend()
# ax3.legend()
# ax4.legend()
# ax5.legend()
# ax6.legend()
# ax7.legend()
# ax8.legend()

ax0.xaxis.grid(True,ls='--')
ax1.xaxis.grid(True,ls='--')
ax2.xaxis.grid(True,ls='--')
ax3.xaxis.grid(True,ls='--')
ax4.xaxis.grid(True,ls='--')
ax5.xaxis.grid(True,ls='--')
ax6.xaxis.grid(True,ls='--')
ax7.xaxis.grid(True,ls='--')
ax8.xaxis.grid(True,ls='--')

ax0.yaxis.grid(True,ls='--')
ax1.yaxis.grid(True,ls='--')
ax2.yaxis.grid(True,ls='--')
ax3.yaxis.grid(True,ls='--')
ax4.yaxis.grid(True,ls='--')
ax5.yaxis.grid(True,ls='--')
ax6.yaxis.grid(True,ls='--')
ax7.yaxis.grid(True,ls='--')
ax8.yaxis.grid(True,ls='--')

ax0.set_xlabel("TCP load (%)")
ax1.set_xlabel("TCP load (%)")
ax2.set_xlabel("TCP load (%)")
ax3.set_xlabel("TCP load (%)")
ax4.set_xlabel("TCP load (%)")
ax5.set_xlabel("TCP load (%)")
ax6.set_xlabel("TCP load (%)")
ax7.set_xlabel("TCP load (%)")
ax8.set_xlabel("TCP load (%)")

ax0.set_xticks([0.2,0.4,0.6,0.8])
ax1.set_xticks([0.2,0.4,0.6,0.8])
ax2.set_xticks([0.2,0.4,0.6,0.8])
ax3.set_xticks([0.2,0.4,0.6,0.8])
ax4.set_xticks([0.2,0.4,0.6,0.8])
ax5.set_xticks([0.2,0.4,0.6,0.8])
ax6.set_xticks([0.2,0.4,0.6,0.8])
ax7.set_xticks([0.2,0.4,0.6,0.8])
ax8.set_xticks([0.2,0.4,0.6,0.8])

ax0.set_xticklabels(["20","40","60","80"])
ax1.set_xticklabels(["20","40","60","80"])
ax2.set_xticklabels(["20","40","60","80"])
ax3.set_xticklabels(["20","40","60","80"])
ax4.set_xticklabels(["20","40","60","80"])
ax5.set_xticklabels(["20","40","60","80"])
ax6.set_xticklabels(["20","40","60","80"])
ax7.set_xticklabels(["20","40","60","80"])
ax8.set_xticklabels(["20","40","60","80"])

ax0.set_ylabel("# PFC pauses (x1000)")
ax1.set_ylabel("Avg. FCT")
ax2.set_ylabel("95-pct FCT")
ax3.set_ylabel("99-pct FCT")
ax4.set_ylabel("99.9-pct FCT")
ax5.set_ylabel("99-pct FCT")
ax6.set_ylabel("99-pct buffer (Lossless)")
ax7.set_ylabel("99-pct buffer (Lossy)")
ax8.set_ylabel("Avg. FCT")
ax8.set_yscale('log')

ax0.set_ylim(0,25)
ax1.set_ylim(10,25)
ax1.set_yticks([10,15,20,25])
ax5.set_ylim(5,50)
ax6.set_ylim(0,50)
ax7.set_ylim(0,50)

figs=[fig0, fig1, fig2,fig3, fig4,fig5, fig6, fig7, fig8, fig9]
filenames=["pfc","incastavgfct","incast95fct","incast99fct","incast999fct","tcp99fct","losslessbuf","lossybuf","tcplongfctav","tcpmedfctav"]

for i in range(len(figs)) :
    figs[i].tight_layout()
    # figs[i].savefig(plots+'tcploads-rdmabursts-'+filenames[i]+'.pdf')



#%%

dump="dump_sigcomm/"

loads=["0.2","0.4","0.6","0.8"]
loadsint=[0.2,0.4,0.6,0.8]

# bursts=["500000", "1000000"]
bursts=["500000", "1000000","1500000", "2000000"]

rdmacc=str(DCQCNCC)
tcpcc=str(CUBIC)
tcpburst="0"
tcpload="0.8"
egresslossyFrac="0.8"
gamma="0.999"

rdmaload="0"

fig0,ax0 = plt.subplots(1,1,figsize=(4,5))
fig1,ax1 = plt.subplots(1,1,figsize=(4,5))
fig2,ax2 = plt.subplots(1,1,figsize=(4,5))
fig3,ax3 = plt.subplots(1,1,figsize=(4,5))
fig4,ax4 = plt.subplots(1,1,figsize=(4,5))
fig5,ax5 = plt.subplots(1,1,figsize=(4,5))
fig6,ax6 = plt.subplots(1,1,figsize=(4,5))
fig7,ax7 = plt.subplots(1,1,figsize=(4,5))


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
        if len(shortfct)>0:
            fct95 = shortfct[int(len(shortfct)*0.95)-1]
            fct99 = shortfct[int(len(shortfct)*0.99)-1]
            fct999 = shortfct[int(len(shortfct)*0.999)-1]
        else:
            fct95=0
            fct99=0
            fct999=0
        shortfct99.append(fct99)
        shortfct95.append(fct95)
        shortfct999.append(fct999)
        shortfctavg.append(np.mean(shortfct))
        
        pfcDF = pd.read_csv(pfcfile,delimiter=' ')
        numpfc.append(len(pfcDF))
        
        torDF = pd.read_csv(torfile,delimiter=' ')
        lossybuf = torDF["egressOccupancyLossy"]
        losslessbuf = torDF["egressOccupancyLossless"]
        totalbuf = torDF["totalused"]
        lossybuf = list(100*lossybuf/buffer)
        losslessbuf = list(100*losslessbuf/buffer)
        totalbuf = list(100*totalbuf/buffer)
        lossybuf.sort()
        losslessbuf.sort()
        totalbuf.sort()
        lossy.append(lossybuf[int(len(lossybuf)*0.99)])
        lossless.append(losslessbuf[int(len(lossybuf)*0.99)])
        total.append(totalbuf[int(len(lossybuf)*0.99)])

        # print (alg,rdmacc,tcpcc,rdmaload,tcpload,rdmaburst,tcpburst,egresslossyFrac,gamma, numpfc[-1],shortfctavg[-1],shortfct95[-1],shortfct99[-1],shortfct999[-1],tcpshortfct99[-1],lossless[-1],lossy[-1],0,0)
        
    
    ax0.plot(np.arange(len(bursts)),[i/1000 for i in numpfc],label=names[alg],lw=4,marker=markers[alg],c=colors[alg], markersize=20)
    ax1.plot(np.arange(len(bursts)),shortfctavg,label=names[alg],lw=4,marker=markers[alg],c=colors[alg], markersize=20)
    ax2.plot(np.arange(len(bursts)),shortfct95,label=names[alg],lw=4,marker=markers[alg],c=colors[alg], markersize=20)
    ax3.plot(np.arange(len(bursts)),shortfct99,label=names[alg],lw=4,marker=markers[alg],c=colors[alg], markersize=20)
    ax4.plot(np.arange(len(bursts)),shortfct999,label=names[alg],lw=4,marker=markers[alg],c=colors[alg], markersize=20)
    ax5.plot(np.arange(len(bursts)),tcpshortfct99,label=names[alg],lw=4,marker=markers[alg],c=colors[alg], markersize=20)
    ax6.plot(np.arange(len(bursts)),lossless,label=names[alg],lw=4,marker=markers[alg],c=colors[alg], markersize=20)
    ax7.plot(np.arange(len(bursts)),lossy,label=names[alg],lw=4,marker=markers[alg],c=colors[alg], markersize=20)

# ax0.legend()
# ax1.legend()
# ax2.legend()
# ax3.legend()
# ax4.legend()
# ax5.legend()
# ax6.legend()
# ax7.legend()

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

ax0.set_xlabel("RDMA Burst Size (MB)")
ax1.set_xlabel("RDMA Burst Size (MB)")
ax2.set_xlabel("RDMA Burst Size (MB)")
ax3.set_xlabel("RDMA Burst Size (MB)")
ax4.set_xlabel("RDMA Burst Size (MB)")
ax5.set_xlabel("RDMA Burst Size (MB)")
ax6.set_xlabel("RDMA Burst Size (MB)")
ax7.set_xlabel("RDMA Burst Size (MB)")

ax0.set_xticks([0,1,2,3])
ax1.set_xticks([0,1,2,3])
ax2.set_xticks([0,1,2,3])
ax3.set_xticks([0,1,2,3])
ax4.set_xticks([0,1,2,3])
ax5.set_xticks([0,1,2,3])
ax6.set_xticks([0,1,2,3])
ax7.set_xticks([0,1,2,3])

ax0.set_xticklabels(["0.5","1","1.5","2"])
ax1.set_xticklabels(["0.5","1","1.5","2"])
ax2.set_xticklabels(["0.5","1","1.5","2"])
ax3.set_xticklabels(["0.5","1","1.5","2"])
ax4.set_xticklabels(["0.5","1","1.5","2"])
ax5.set_xticklabels(["0.5","1","1.5","2"])
ax6.set_xticklabels(["0.5","1","1.5","2"])
ax7.set_xticklabels(["0.5","1","1.5","2"])

ax0.set_ylabel("# PFC pauses (x1000)")
ax1.set_ylabel("Avg. FCT")
ax2.set_ylabel("95-pct FCT")
ax3.set_ylabel("99-pct FCT")
ax4.set_ylabel("99.9-pct FCT")
ax5.set_ylabel("99-pct FCT")
ax6.set_ylabel("99-pct buffer (Lossless)")
ax7.set_ylabel("99-pct buffer (Lossy)")

ax0.set_ylim(-1,25)
ax0.set_yticks([0,5,10,15,20,25])
ax1.set_ylim(5,21)
ax1.set_yticks([5,10,15,20])
ax5.set_ylim(5,60)
ax6.set_ylim(0,50)
ax7.set_ylim(0,50)

figs=[fig0, fig1, fig2,fig3, fig4,fig5, fig6, fig7]
filenames=["pfc","incastavgfct","incast95fct","incast99fct","incast999fct","tcp99fct","losslessbuf","lossybuf","tcplongfctav"]

for i in range(len(figs)) :
   figs[i].tight_layout()
   figs[i].savefig(plots+'tcploads-acrossrdmabursts-'+filenames[i]+'.pdf')

#%%
dump="loveland_dump/"

loads=["0.2","0.4","0.6","0.8"]
loadsint=[0.2,0.4,0.6,0.8]

# bursts=["500000", "1000000"]
bursts=["500000", "1000000","1500000", "2000000"]

rdmacc=str(INTCC)
tcpcc=str(CUBIC)
rdmaburst="0"
tcpload="0"
egresslossyFrac="0.8"
gamma="0.999"

tcpburst="1500000"

fig0,ax0 = plt.subplots(1,1,figsize=(4,5))
fig1,ax1 = plt.subplots(1,1,figsize=(4,5))
fig2,ax2 = plt.subplots(1,1,figsize=(4,5))
fig3,ax3 = plt.subplots(1,1,figsize=(4,5))
fig4,ax4 = plt.subplots(1,1,figsize=(4,5))
fig5,ax5 = plt.subplots(1,1,figsize=(4,5))
fig6,ax6 = plt.subplots(1,1,figsize=(4,5))
fig7,ax7 = plt.subplots(1,1,figsize=(4,5))
fig8,ax8 = plt.subplots(1,1,figsize=(4,5))

for alg in algs:
    shortfct95=list()
    shortfct99=list()
    shortfct999=list()
    shortfctavg=list()
    rdmashortfct99=list()
    rdmalongfctav=list()
    numpfc=list()
    lossy=list()
    lossless=list()
    total=list()
    for rdmaload in loads:
        fctfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.fct'
        torfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.tor'
        outfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.out'
        pfcfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.pfc'
        
        fctDF = pd.read_csv(fctfile,delimiter=' ')
        shortfctDF = fctDF[(fctDF["incastflow"]==1)&(fctDF["priority"]==1)] # fctDF[(fctDF["flowsize"]<100000)&(fctDF["priority"]==3)] #
        
        rdmashortfctDF = fctDF[(fctDF["flowsize"]<100000)&(fctDF["priority"]==3)]
        shortfct = list(rdmashortfctDF["slowdown"])
        shortfct.sort()
        fct99 = shortfct[int(len(shortfct)*0.99)]
        rdmashortfct99.append(fct99)
        
        rdmashortfctDF = fctDF[(fctDF["flowsize"]>1000000)&(fctDF["priority"]==3)]
        shortfct = list(rdmashortfctDF["slowdown"])
        shortfct.sort()
        fct = np.median(shortfct)
        rdmalongfctav.append(fct)
        
        shortfct = list(shortfctDF["slowdown"])
        shortfct.sort()
        if len(shortfct)>0:
            fct95 = shortfct[int(len(shortfct)*0.95)]
            fct99 = shortfct[int(len(shortfct)*0.99)]
            fct999 = shortfct[int(len(shortfct)*0.999)]
        else:
            fct95=0
            fct99=0
            fct999=0
        shortfct99.append(fct99)
        shortfct95.append(fct95)
        shortfct999.append(fct999)
        shortfctavg.append(np.mean(shortfct))
        
        pfcDF = pd.read_csv(pfcfile,delimiter=' ')
        numpfc.append(len(pfcDF))
        
        torDF = pd.read_csv(torfile,delimiter=' ')
        lossybuf = torDF["egressOccupancyLossy"]
        losslessbuf = torDF["egressOccupancyLossless"]
        totalbuf = torDF["totalused"]
        lossybuf = list(100*lossybuf/buffer)
        losslessbuf = list(100*losslessbuf/buffer)
        totalbuf = list(100*totalbuf/buffer)
        lossybuf.sort()
        losslessbuf.sort()
        totalbuf.sort()
        lossy.append(lossybuf[int(len(lossybuf)*0.99)])
        lossless.append(losslessbuf[int(len(lossybuf)*0.99)])
        total.append(totalbuf[int(len(lossybuf)*0.99)])

        # print (alg,rdmacc,tcpcc,rdmaload,tcpload,rdmaburst,tcpburst,egresslossyFrac,gamma, numpfc[-1],shortfctavg[-1],shortfct95[-1],shortfct99[-1],shortfct999[-1],rdmashortfct99[-1],lossless[-1],lossy[-1],0,0)
        
    
    ax0.plot(loadsint,[i/1000 for i in numpfc],label=names[alg],lw=4,marker=markers[alg],c=colors[alg], markersize=20)
    ax1.plot(loadsint,shortfctavg,label=names[alg],lw=4,marker=markers[alg],c=colors[alg], markersize=20)
    ax2.plot(loadsint,shortfct95,label=names[alg],lw=4,marker=markers[alg],c=colors[alg], markersize=20)
    ax3.plot(loadsint,shortfct99,label=names[alg],lw=4,marker=markers[alg],c=colors[alg], markersize=20)
    ax4.plot(loadsint,shortfct999,label=names[alg],lw=4,marker=markers[alg],c=colors[alg], markersize=20)
    ax5.plot(loadsint,rdmashortfct99,label=names[alg],lw=4,marker=markers[alg],c=colors[alg], markersize=20)
    ax6.plot(loadsint,lossless,label=names[alg],lw=4,marker=markers[alg],c=colors[alg], markersize=20)
    ax7.plot(loadsint,lossy,label=names[alg],lw=4,marker=markers[alg],c=colors[alg], markersize=20)
    ax8.plot(loadsint,rdmalongfctav,label=names[alg],lw=4,marker=markers[alg],c=colors[alg], markersize=20)

# ax0.legend()
# ax1.legend()
# ax2.legend()
# ax3.legend()
# ax4.legend()
# ax5.legend()
# ax6.legend()
# ax7.legend()
# ax8.legend()

ax0.xaxis.grid(True,ls='--')
ax1.xaxis.grid(True,ls='--')
ax2.xaxis.grid(True,ls='--')
ax3.xaxis.grid(True,ls='--')
ax4.xaxis.grid(True,ls='--')
ax5.xaxis.grid(True,ls='--')
ax6.xaxis.grid(True,ls='--')
ax7.xaxis.grid(True,ls='--')
ax8.xaxis.grid(True,ls='--')

ax0.yaxis.grid(True,ls='--')
ax1.yaxis.grid(True,ls='--')
ax2.yaxis.grid(True,ls='--')
ax3.yaxis.grid(True,ls='--')
ax4.yaxis.grid(True,ls='--')
ax5.yaxis.grid(True,ls='--')
ax6.yaxis.grid(True,ls='--')
ax7.yaxis.grid(True,ls='--')
ax8.yaxis.grid(True,ls='--')

ax0.set_xticks([0.2,0.4,0.6,0.8])
ax1.set_xticks([0.2,0.4,0.6,0.8])
ax2.set_xticks([0.2,0.4,0.6,0.8])
ax3.set_xticks([0.2,0.4,0.6,0.8])
ax4.set_xticks([0.2,0.4,0.6,0.8])
ax5.set_xticks([0.2,0.4,0.6,0.8])
ax6.set_xticks([0.2,0.4,0.6,0.8])
ax7.set_xticks([0.2,0.4,0.6,0.8])
ax8.set_xticks([0.2,0.4,0.6,0.8])

ax0.set_xticklabels(["20","40","60","80"])
ax1.set_xticklabels(["20","40","60","80"])
ax2.set_xticklabels(["20","40","60","80"])
ax3.set_xticklabels(["20","40","60","80"])
ax4.set_xticklabels(["20","40","60","80"])
ax5.set_xticklabels(["20","40","60","80"])
ax6.set_xticklabels(["20","40","60","80"])
ax7.set_xticklabels(["20","40","60","80"])
ax8.set_xticklabels(["20","40","60","80"])

ax0.set_xlabel("RDMA load (%)")
ax1.set_xlabel("RDMA load (%)")
ax2.set_xlabel("RDMA load (%)")
ax3.set_xlabel("RDMA load (%)")
ax4.set_xlabel("RDMA load (%)")
ax5.set_xlabel("RDMA load (%)")
ax6.set_xlabel("RDMA load (%)")
ax7.set_xlabel("RDMA load (%)")
ax8.set_xlabel("RDMA load (%)")

ax0.set_ylabel("# PFC pauses (x1000)")
ax1.set_ylabel("Avg. FCT")
ax2.set_ylabel("95-pct FCT")
ax3.set_ylabel("99-pct FCT")
ax4.set_ylabel("99.9-pct FCT")
ax5.set_ylabel("99-pct FCT")
ax6.set_ylabel("99-pct buffer (Lossless)")
ax7.set_ylabel("99-pct buffer (Lossy)")
ax8.set_ylabel("Avg. FCT")

ax0.set_ylim(-1,6)
ax0.set_yticks([0,1,2,3,4,5,6])
ax1.set_ylim(5,21)
ax1.set_yticks([5,10,15,20])
ax5.set_ylim(1,4)
ax5.set_yticks([1,2,3,4])
ax6.set_ylim(-1,30)
ax6.set_yticks([0,10,20,30])
ax7.set_ylim(-1,30)
ax7.set_yticks([0,10,20,30])

figs=[fig0, fig1, fig2,fig3, fig4,fig5, fig6, fig7,fig8]
filenames=["pfc","incastavgfct","incast95fct","incast99fct","incast999fct","tcp99fct","losslessbuf","lossybuf","tcplongfctav"]

for i in range(len(figs)) :
   figs[i].tight_layout()
   figs[i].savefig(plots+'rdmaloads-tcpbursts-'+filenames[i]+'.pdf')

#%%
dump="loveland_dump/"

loads=["0.2","0.4","0.6","0.8"]
loadsint=[0.2,0.4,0.6,0.8]

rdmacc=str(INTCC)
tcpcc=str(CUBIC)
tcpload="0"
egresslossyFrac="0.8"
gamma="0.999"

rdmaload="0.8"
rdmaburst="0"

fig0,ax0 = plt.subplots(1,1,figsize=(4,5))
fig1,ax1 = plt.subplots(1,1,figsize=(4,5))
fig2,ax2 = plt.subplots(1,1,figsize=(4,5))
fig3,ax3 = plt.subplots(1,1,figsize=(4,5))
fig4,ax4 = plt.subplots(1,1,figsize=(4,5))
fig5,ax5 = plt.subplots(1,1,figsize=(4,5))
fig6,ax6 = plt.subplots(1,1,figsize=(4,5))
fig7,ax7 = plt.subplots(1,1,figsize=(4,5))
fig8,ax8 = plt.subplots(1,1,figsize=(4,5))

burststemp=["12500","500000", "1000000","1500000"]

for alg in algs:
    shortfct95=list()
    shortfct99=list()
    shortfct999=list()
    shortfctavg=list()
    rdmashortfct99=list()
    rdmalongfctav=list()
    numpfc=list()
    lossy=list()
    lossless=list()
    total=list()
    for tcpburst in burststemp:
        fctfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.fct'
        torfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.tor'
        outfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.out'
        pfcfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.pfc'
        
        fctDF = pd.read_csv(fctfile,delimiter=' ')
        shortfctDF = fctDF[(fctDF["incastflow"]==1)&(fctDF["priority"]==1)] #fctDF[(fctDF["flowsize"]<100000)&(fctDF["priority"]==3)]
        
        rdmashortfctDF = fctDF[(fctDF["flowsize"]<100000)&(fctDF["priority"]==3)]
        shortfct = list(rdmashortfctDF["slowdown"])
        shortfct.sort()
        fct99 = shortfct[int(len(shortfct)*0.99)]
        rdmashortfct99.append(fct99)
        
        rdmashortfctDF = fctDF[(fctDF["flowsize"]>1000000)&(fctDF["priority"]==3)]
        shortfct = list(rdmashortfctDF["slowdown"])
        shortfct.sort()
        fct =  np.median(shortfct)
        rdmalongfctav.append(fct)
        
        shortfct = list(shortfctDF["slowdown"])
        shortfct.sort()
        if len(shortfct)>0:
            fct95 = shortfct[int(len(shortfct)*0.95)]
            fct99 = shortfct[int(len(shortfct)*0.99)]
            fct999 = shortfct[int(len(shortfct)*0.999)]
        else:
            fct95=0
            fct99=0
            fct999=0
        shortfct99.append(fct99)
        shortfct95.append(fct95)
        shortfct999.append(fct999)
        shortfctavg.append(np.mean(shortfct))
        
        pfcDF = pd.read_csv(pfcfile,delimiter=' ')
        numpfc.append(len(pfcDF))
        
        torDF = pd.read_csv(torfile,delimiter=' ')
        lossybuf = torDF["egressOccupancyLossy"]
        losslessbuf = torDF["egressOccupancyLossless"]
        totalbuf = torDF["totalused"]
        lossybuf = list(100*lossybuf/buffer)
        losslessbuf = list(100*losslessbuf/buffer)
        totalbuf = list(100*totalbuf/buffer)
        lossybuf.sort()
        losslessbuf.sort()
        totalbuf.sort()
        lossy.append(lossybuf[int(len(lossybuf)*0.99)])
        lossless.append(losslessbuf[int(len(lossybuf)*0.99)])
        total.append(totalbuf[int(len(lossybuf)*0.99)])

        # print (alg,rdmacc,tcpcc,rdmaload,tcpload,rdmaburst,tcpburst,egresslossyFrac,gamma, numpfc[-1],shortfctavg[-1],shortfct95[-1],shortfct99[-1],shortfct999[-1],rdmashortfct99[-1],lossless[-1],lossy[-1],0,0)
        
    
    ax0.plot(np.arange(len(burststemp)),[i/1000 for i in numpfc],label=names[alg],lw=4,marker=markers[alg],c=colors[alg], markersize=20)
    ax1.plot(np.arange(len(burststemp)),shortfctavg,label=names[alg],lw=4,marker=markers[alg],c=colors[alg], markersize=20)
    ax2.plot(np.arange(len(burststemp)),shortfct95,label=names[alg],lw=4,marker=markers[alg],c=colors[alg], markersize=20)
    ax3.plot(np.arange(len(burststemp)),shortfct99,label=names[alg],lw=4,marker=markers[alg],c=colors[alg], markersize=20)
    ax4.plot(np.arange(len(burststemp)),shortfct999,label=names[alg],lw=4,marker=markers[alg],c=colors[alg], markersize=20)
    ax5.plot(np.arange(len(burststemp)),rdmashortfct99,label=names[alg],lw=4,marker=markers[alg],c=colors[alg], markersize=20)
    ax6.plot(np.arange(len(burststemp)),lossless,label=names[alg],lw=4,marker=markers[alg],c=colors[alg], markersize=20)
    ax7.plot(np.arange(len(burststemp)),lossy,label=names[alg],lw=4,marker=markers[alg],c=colors[alg], markersize=20)
    ax8.plot(np.arange(len(burststemp)),rdmalongfctav,label=names[alg],lw=4,marker=markers[alg],c=colors[alg], markersize=20)

# ax0.legend()
# ax1.legend()
# ax2.legend()
# ax3.legend()
# ax4.legend()
# ax5.legend()
# ax6.legend()
# ax7.legend()
# ax8.legend()

ax0.xaxis.grid(True,ls='--')
ax1.xaxis.grid(True,ls='--')
ax2.xaxis.grid(True,ls='--')
ax3.xaxis.grid(True,ls='--')
ax4.xaxis.grid(True,ls='--')
ax5.xaxis.grid(True,ls='--')
ax6.xaxis.grid(True,ls='--')
ax7.xaxis.grid(True,ls='--')
ax8.xaxis.grid(True,ls='--')

ax0.yaxis.grid(True,ls='--')
ax1.yaxis.grid(True,ls='--')
ax2.yaxis.grid(True,ls='--')
ax3.yaxis.grid(True,ls='--')
ax4.yaxis.grid(True,ls='--')
ax5.yaxis.grid(True,ls='--')
ax6.yaxis.grid(True,ls='--')
ax7.yaxis.grid(True,ls='--')
ax8.yaxis.grid(True,ls='--')

ax0.set_xlabel("TCP Burst Size (MB)")
ax1.set_xlabel("TCP Burst Size (MB)")
ax2.set_xlabel("TCP Burst Size (MB)")
ax3.set_xlabel("TCP Burst Size (MB)")
ax4.set_xlabel("TCP Burst Size (MB)")
ax5.set_xlabel("TCP Burst Size (MB)")
ax6.set_xlabel("TCP Burst Size (MB)")
ax7.set_xlabel("TCP Burst Size (MB)")

ax0.set_xticks([0,1,2,3])
ax1.set_xticks([0,1,2,3])
ax2.set_xticks([0,1,2,3])
ax3.set_xticks([0,1,2,3])
ax4.set_xticks([0,1,2,3])
ax5.set_xticks([0,1,2,3])
ax6.set_xticks([0,1,2,3])
ax7.set_xticks([0,1,2,3])

ax0.set_xticklabels(["0.25","0.5","1","1.5"])
ax1.set_xticklabels(["0.25","0.5","1","1.5"])
ax2.set_xticklabels(["0.25","0.5","1","1.5"])
ax3.set_xticklabels(["0.25","0.5","1","1.5"])
ax4.set_xticklabels(["0.25","0.5","1","1.5"])
ax5.set_xticklabels(["0.25","0.5","1","1.5"])
ax6.set_xticklabels(["0.25","0.5","1","1.5"])
ax7.set_xticklabels(["0.25","0.5","1","1.5"])

# ax0.set_yticks([0,1,2,3])
# ax1.set_yticks([0,1,2,3])
# ax2.set_yticks([0,1,2,3])
# ax3.set_yticks([0,1,2,3])
# ax4.set_yticks([0,1,2,3])
# ax5.set_yticks([0,1,2,3])
# ax6.set_yticks([0,1,2,3])
# ax7.set_yticks([0,1,2,3])

# ax0.set_ylim(0,25)
# ax1.set_ylim(0,10)
# ax2.set_ylim(10,25)
# ax3.set_ylim(0,10)
# ax4.set_ylim(0,10)
# ax5.set_ylim(0,10)
# ax6.set_ylim(0,10)
# ax7.set_ylim(0,10)

ax0.set_ylabel("# PFC pauses (x1000)")
ax1.set_ylabel("Avg. FCT")
ax2.set_ylabel("95-pct FCT")
ax3.set_ylabel("99-pct FCT")
ax4.set_ylabel("99.9-pct FCT")
ax5.set_ylabel("99-pct FCT")
ax6.set_ylabel("99-pct buffer (Lossless)")
ax7.set_ylabel("99-pct buffer (Lossy)")
ax8.set_ylabel("Avg. FCT")

ax0.set_ylim(-1,6)
ax0.set_yticks([0,1,2,3,4,5,6])
ax1.set_ylim(0,16)
ax1.set_yticks([1,5,10,15])
ax5.set_ylim(1,4)
ax5.set_yticks([1,2,3,4])
ax6.set_ylim(-1,30)
ax6.set_yticks([0,10,20,30])
ax7.set_ylim(-1,30)
ax7.set_yticks([0,10,20,30])

figs=[fig0, fig1, fig2,fig3, fig4,fig5, fig6, fig7,fig8]
filenames=["pfc","incastavgfct","incast95fct","incast99fct","incast999fct","tcp99fct","losslessbuf","lossybuf","tcplongfctav"]

for i in range(len(figs)) :
   figs[i].tight_layout()
   figs[i].savefig(plots+'acrossrdmaloads-tcpbursts-'+filenames[i]+'.pdf')

#%%

plt.rcParams.update({'font.size': 14})
dump="dump_sigcomm/"

loads=["0.2","0.4","0.6","0.8"]
loadsint=[0.2,0.4,0.6,0.8]

# bursts=["500000", "1000000"]
bursts=["500000", "1000000","1500000", "2000000", "2500000"]

rdmacc=str(DCQCNCC)
tcpcc=str(CUBIC)
rdmaburst="2000000"
tcpburst="0"
egresslossyFrac="0.8"
gamma="0.999"

rdmaload="0.8"

fig0,ax0 = plt.subplots(1,1,figsize=(6,3))
fig1,ax1 = plt.subplots(1,1,figsize=(6,3))
fig2,ax2 = plt.subplots(1,1,figsize=(6,3))
fig3,ax3 = plt.subplots(1,1,figsize=(6,3))
fig4,ax4 = plt.subplots(1,1,figsize=(6,3))


alg=str(REVERIE)
numpfc=list()
shortfct95=list()
shortfct99=list()
shortfct999=list()
shortfctavg=list()

for gamma in ["0.8","0.9","0.99","0.999","0.999999"]:
    fctfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.fct'
    
    fctDF = pd.read_csv(fctfile,delimiter=' ')
    shortfctDF = fctDF[(fctDF["incastflow"]==1)&(fctDF["priority"]==3)] #fctDF[(fctDF["flowsize"]<100000)&(fctDF["priority"]==3)]   
    shortfct = list(shortfctDF["slowdown"])
    shortfct.sort()
    if len(shortfct)>0:
        fct95 = shortfct[int(len(shortfct)*0.95)]
        fct99 = shortfct[int(len(shortfct)*0.99)]
        fct999 = shortfct[int(len(shortfct)*0.999)]
    else:
        fct95=0
        fct99=0
        fct999=0
    shortfct99.append(fct99)
    shortfct95.append(fct95)
    shortfct999.append(fct999)
    shortfctavg.append(np.mean(shortfct))
    
    pfcfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.pfc'
    pfcDF = pd.read_csv(pfcfile,delimiter=' ')
    numpfc.append(len(pfcDF))

# alg=str(ABM)
# gamma="0.999"
# fctfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.fct'

# fctDF = pd.read_csv(fctfile,delimiter=' ')
# shortfctDF = fctDF[(fctDF["incastflow"]==1)&(fctDF["priority"]==3)] #fctDF[(fctDF["flowsize"]<100000)&(fctDF["priority"]==3)]   
# shortfct = list(shortfctDF["slowdown"])
# shortfct.sort()
# if len(shortfct)>0:
#     fct95 = shortfct[int(len(shortfct)*0.95)]
#     fct99 = shortfct[int(len(shortfct)*0.99)]
#     fct999 = shortfct[int(len(shortfct)*0.999)]
# else:
#     fct95=0
#     fct99=0
#     fct999=0
# shortfct99.append(fct99)
# shortfct95.append(fct95)
# shortfct999.append(fct999)
# shortfctavg.append(np.mean(shortfct))

# pfcfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.pfc'
# pfcDF = pd.read_csv(pfcfile,delimiter=' ')
# numpfc.append(len(pfcDF))
    
ax0.plot(np.arange(5),[i/1000 for i in numpfc],lw=4,marker='P', markersize=10,c='k')
ax1.plot(np.arange(5),shortfctavg,lw=4,marker='P', markersize=10,c='k')
ax2.plot(np.arange(5),shortfct95,lw=4,marker='P', markersize=10,c='k')
ax3.plot(np.arange(5),shortfct99,lw=4,marker='P',markersize=10,c='k')
ax4.plot(np.arange(5),shortfct999,lw=4,marker='P',markersize=10,c='k')

ax0.xaxis.grid(True,ls='--')
ax1.xaxis.grid(True,ls='--')
ax2.xaxis.grid(True,ls='--')
ax3.xaxis.grid(True,ls='--')
ax4.xaxis.grid(True,ls='--')

ax0.yaxis.grid(True,ls='--')
ax1.yaxis.grid(True,ls='--')
ax2.yaxis.grid(True,ls='--')
ax3.yaxis.grid(True,ls='--')
ax4.yaxis.grid(True,ls='--')

# ax0.set_xticklabels(["0.8",r'1-10^{-1}',"r'1-10^{-2}'",r'1-10^{-3}',r'1-10^{-6}'])
ax0.set_xticks(np.arange(5))
ax0.set_xticklabels(["0.8","0.9","0.99","0.999","0.999999"])
ax1.set_xticks(np.arange(5))
ax1.set_xticklabels(["0.8","0.9","0.99","0.999","0.999999"])

ax0.set_xlabel("Parameter "+r'$\gamma$'+ " value")
ax1.set_xlabel("Parameter "+r'$\gamma$'+ " value")
ax2.set_xlabel("Parameter "+r'$\gamma$'+ " value")
ax3.set_xlabel("Parameter "+r'$\gamma$'+ " value")
ax4.set_xlabel("Parameter "+r'$\gamma$'+ " value")

ax0.set_ylabel("# PFC pauses (x1000)")
ax1.set_ylabel("Avg. FCT")
ax2.set_ylabel("95-pct FCT")
ax3.set_ylabel("99-pct FCT")
ax4.set_ylabel("99.9-pct FCT")

fig0.tight_layout()
fig0.savefig(plots+'pfc.pdf')

fig1.tight_layout()
fig1.savefig(plots+'pfc-fctav.pdf')

fig2.tight_layout()
fig2.savefig(plots+'pfc-fct95.pdf')

fig3.tight_layout()
fig3.savefig(plots+'pfc-fct99.pdf')

fig4.tight_layout()
fig4.savefig(plots+'pfc-fct999.pdf')









#%%
# #%%
# rdmacc=str(DCQCNCC)
# tcpcc=str(CUBIC)
# # tcpburst="2000000"
# tcpload="0"
# rdmaload="0.8"

# rdmaburst="0"
# egresslossyFrac="0.8"
# gamma="0.999"

# fig,ax=plt.subplots(1,1)

# loads=["0","0.2","0.4","0.6","0.8"]
# loadsint=[0,0.2,0.4,0.6,0.8]

# for alg in algs:
#     shortfct99=list()
#     for tcpburst in bursts:
#         fctfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.fct'
#         torfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.tor'
#         outfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.out'
        
#         fctDF = pd.read_csv(fctfile,delimiter=' ')
#         shortfctDF = fctDF[(fctDF["incastflow"]==1)&(fctDF["priority"]==1)&(fctDF["timestamp"]<2)]
        
#         shortfct = list(shortfctDF["slowdown"])
#         shortfct.sort()
#         fct99 = shortfct[int(len(shortfct)*0.99)]
#         shortfct99.append(fct99)
        
#     ax.plot(np.arange(len(bursts)),shortfct99,label=alg)

# ax.legend()
# # ax.set_yscale('log')


#%%


dump="dump_sigcomm/"

loads=["0.2","0.4","0.6","0.8"]
loadsint=[0.2,0.4,0.6,0.8]

# bursts=["500000", "1000000"]
bursts=["500000", "1000000","1500000", "2000000", "2500000"]

rdmacc=str(DCQCNCC)
tcpcc=str(CUBIC)
rdmaburst="2000000"
rdmaload="0"
tcpburst="0"
tcpload="0.8"
lossyFracs=["0.2","0.4","0.6","0.8"]
lossyFracsInt=[0.2,0.4,0.6,0.8]

egresslossyFrac="0.8"
gamma="0.999"

rdmaload="0"

fig0,ax0 = plt.subplots(1,1,figsize=(4,5))
fig1,ax1 = plt.subplots(1,1,figsize=(4,5))
fig2,ax2 = plt.subplots(1,1,figsize=(4,5))
fig3,ax3 = plt.subplots(1,1,figsize=(4,5))
fig4,ax4 = plt.subplots(1,1,figsize=(4,5))
fig5,ax5 = plt.subplots(1,1,figsize=(4,5))
fig6,ax6 = plt.subplots(1,1,figsize=(4,5))
fig7,ax7 = plt.subplots(1,1,figsize=(4,5))
fig8,ax8 = plt.subplots(1,1,figsize=(4,5))
fig9,ax9 = plt.subplots(1,1,figsize=(4,5))


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
    longfctav=list()
    medfctav=list()
    for egresslossyFrac in lossyFracs:
        fctfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.fct'
        torfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.tor'
        outfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.out'
        pfcfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.pfc'
        
        fctDF = pd.read_csv(fctfile,delimiter=' ')
        shortfctDF = fctDF[(fctDF["incastflow"]==1)&(fctDF["priority"]==3)]
        
        tcpshortfctDF = fctDF[(fctDF["flowsize"]<100000)&(fctDF["priority"]==1)]
        shortfct = list(tcpshortfctDF["slowdown"])
        shortfct.sort()
        fct99 = shortfct[int(len(shortfct)*0.99)-1]
        tcpshortfct99.append(fct99)
        
        shortfct = list(shortfctDF["slowdown"])
        shortfct.sort()
        fct95 = shortfct[int(len(shortfct)*0.95)-1]
        shortfct95.append(fct95)
        fct99 = shortfct[int(len(shortfct)*0.99)-1]
        shortfct99.append(fct99)
        fct999 = shortfct[int(len(shortfct)*0.999)-1]
        shortfct999.append(fct999)
        shortfctavg.append(np.mean(shortfct))
        
        pfcDF = pd.read_csv(pfcfile,delimiter=' ')
        numpfc.append(len(pfcDF))
        
        torDF = pd.read_csv(torfile,delimiter=' ')
        lossybuf = torDF["egressOccupancyLossy"]
        losslessbuf = torDF["egressOccupancyLossless"]
        totalbuf = torDF["totalused"]
        lossybuf = list(100*lossybuf/buffer)
        losslessbuf = list(100*losslessbuf/buffer)
        totalbuf = list(100*totalbuf/buffer)
        lossybuf.sort()
        losslessbuf.sort()
        totalbuf.sort()
        lossy.append(lossybuf[int(len(lossybuf)*0.99)])
        lossless.append(losslessbuf[int(len(lossybuf)*0.99)])
        total.append(totalbuf[int(len(lossybuf)*0.99)])
        
        longfctDF = fctDF[(fctDF["flowsize"]>1000000)&(fctDF["priority"])==1]
        longfct = list(longfctDF["slowdown"])
        longfct.sort()
        # longfctav.append(longfct[int(len(longfct)*0.99)])
        longfctav.append(np.mean(longfct))
        
        
        medfctDF = fctDF[(fctDF["flowsize"]<1000000)&(fctDF["flowsize"]>100000)&(fctDF["priority"])==1]
        medfct = list(longfctDF["slowdown"])
        medfct.sort()
        # medfctav.append(medfct[int(len(medfct)*0.99)])
        medfctav.append(np.mean(medfct))
        
        # print (alg,rdmacc,tcpcc,rdmaload,tcpload,rdmaburst,tcpburst,egresslossyFrac,gamma, numpfc[-1],shortfctavg[-1],shortfct95[-1],shortfct99[-1],shortfct999[-1],tcpshortfct99[-1],lossless[-1],lossy[-1],0,0)
        
    
    ax0.plot(lossyFracsInt ,[i/1000 for i in numpfc],label=names[alg],lw=4,marker=markers[alg],c=colors[alg], markersize=20)
    ax1.plot(lossyFracsInt,shortfctavg,label=names[alg],lw=4,marker=markers[alg],c=colors[alg], markersize=20)
    ax2.plot(lossyFracsInt,shortfct95,label=names[alg],lw=4,marker=markers[alg],c=colors[alg], markersize=20)
    ax3.plot(lossyFracsInt,shortfct99,label=names[alg],lw=4,marker=markers[alg],c=colors[alg], markersize=20)
    ax4.plot(lossyFracsInt,shortfct999,label=names[alg],lw=4,marker=markers[alg],c=colors[alg], markersize=20)
    ax5.plot(lossyFracsInt,tcpshortfct99,label=names[alg],lw=4,marker=markers[alg],c=colors[alg], markersize=20)
    ax6.plot(lossyFracsInt,lossless,label=names[alg],lw=4,marker=markers[alg],c=colors[alg], markersize=20)
    ax7.plot(lossyFracsInt,lossy,label=names[alg],lw=4,marker=markers[alg],c=colors[alg], markersize=20)
    ax8.plot(lossyFracsInt,longfctav,label=names[alg],lw=4,marker=markers[alg],c=colors[alg], markersize=20)
    ax9.plot(lossyFracsInt,medfctav,label=names[alg],lw=4,marker=markers[alg],c=colors[alg], markersize=20)

# ax0.legend()
# ax1.legend()
# ax2.legend()
# ax3.legend()
# ax4.legend()
# ax5.legend()
# ax6.legend()
# ax7.legend()
# ax8.legend()

ax0.xaxis.grid(True,ls='--')
ax1.xaxis.grid(True,ls='--')
ax2.xaxis.grid(True,ls='--')
ax3.xaxis.grid(True,ls='--')
ax4.xaxis.grid(True,ls='--')
ax5.xaxis.grid(True,ls='--')
ax6.xaxis.grid(True,ls='--')
ax7.xaxis.grid(True,ls='--')
ax8.xaxis.grid(True,ls='--')
ax9.xaxis.grid(True,ls='--')

ax0.yaxis.grid(True,ls='--')
ax1.yaxis.grid(True,ls='--')
ax2.yaxis.grid(True,ls='--')
ax3.yaxis.grid(True,ls='--')
ax4.yaxis.grid(True,ls='--')
ax5.yaxis.grid(True,ls='--')
ax6.yaxis.grid(True,ls='--')
ax7.yaxis.grid(True,ls='--')
ax8.yaxis.grid(True,ls='--')
ax9.yaxis.grid(True,ls='--')

ax0.set_xlabel("Egress lossy pool (%)")
ax1.set_xlabel("Egress lossy pool (%)")
ax2.set_xlabel("Egress lossy pool (%)")
ax3.set_xlabel("Egress lossy pool (%)")
ax4.set_xlabel("Egress lossy pool (%)")
ax5.set_xlabel("Egress lossy pool (%)")
ax6.set_xlabel("Egress lossy pool (%)")
ax7.set_xlabel("Egress lossy pool (%)")
ax8.set_xlabel("Egress lossy pool (%)")
ax9.set_xlabel("Egress lossy pool (%)")

ax0.set_xticks([0.2,0.4,0.6,0.8])
ax1.set_xticks([0.2,0.4,0.6,0.8])
ax2.set_xticks([0.2,0.4,0.6,0.8])
ax3.set_xticks([0.2,0.4,0.6,0.8])
ax4.set_xticks([0.2,0.4,0.6,0.8])
ax5.set_xticks([0.2,0.4,0.6,0.8])
ax6.set_xticks([0.2,0.4,0.6,0.8])
ax7.set_xticks([0.2,0.4,0.6,0.8])
ax8.set_xticks([0.2,0.4,0.6,0.8])
ax9.set_xticks([0.2,0.4,0.6,0.8])

ax0.set_xticklabels(["20","40","60","80"])
ax1.set_xticklabels(["20","40","60","80"])
ax2.set_xticklabels(["20","40","60","80"])
ax3.set_xticklabels(["20","40","60","80"])
ax4.set_xticklabels(["20","40","60","80"])
ax5.set_xticklabels(["20","40","60","80"])
ax6.set_xticklabels(["20","40","60","80"])
ax7.set_xticklabels(["20","40","60","80"])
ax8.set_xticklabels(["20","40","60","80"])
ax9.set_xticklabels(["20","40","60","80"])

ax0.set_ylabel("# PFC pauses (x1000)")
ax1.set_ylabel("Avg. FCT")
ax2.set_ylabel("95-pct FCT")
ax3.set_ylabel("99-pct FCT")
ax4.set_ylabel("99.9-pct FCT")
ax5.set_ylabel("99-pct FCT")
ax6.set_ylabel("99-pct buffer (Lossless)")
ax7.set_ylabel("99-pct buffer (Lossy)")
ax8.set_ylabel("Avg. FCT")
ax9.set_ylabel("Median FCT")
# ax8.set_yscale('log')

ax0.set_ylim(0,25)
ax1.set_ylim(10,25)
ax1.set_yticks([10,15,20,25])
ax5.set_ylim(5,50)
ax6.set_ylim(0,50)
ax7.set_ylim(0,50)

figs=[fig0, fig1, fig2,fig3, fig4,fig5, fig6, fig7, fig8, fig9]
filenames=["pfc","incastavgfct","incast95fct","incast99fct","incast999fct","tcp99fct","losslessbuf","lossybuf","tcplongfctav","tcpmedfctav"]

for i in range(len(figs)) :
    figs[i].tight_layout()
    figs[i].savefig(plots+'lossyfrac-'+filenames[i]+'.pdf')

