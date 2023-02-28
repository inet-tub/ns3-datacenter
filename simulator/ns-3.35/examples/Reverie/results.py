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
plots="plots_sigcomm/"
# plots="/home/vamsi/plots_sigcomm/"


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

# dump="dump_sigcomm/"

# loads=["0.2","0.4","0.6","0.8"]
# loadsint=[0.2,0.4,0.6,0.8]

# # bursts=["500000", "1000000"]
# bursts=["500000", "1000000","1500000", "2000000", "2500000"]

# rdmacc=str(DCQCNCC)
# tcpcc=str(CUBIC)
# rdmaburst="2000000"
# tcpburst="0"
# egresslossyFrac="0.8"
# gamma="0.999"

# rdmaload="0"


# print("tcpload","alg","shortfct95","numpfc","tcpshortfct99","lossless","lossy")
# for tcpload in loads:
#     shortfct95=list()
#     shortfct99=list()
#     shortfct999=list()
#     shortfctavg=list()
#     tcpshortfct99=list()
#     numpfc=list()
#     lossy=list()
#     lossless=list()
#     total=list()
#     longfctav=list()
#     medfctav=list()
#     for alg in algs:
#         fctfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.fct'
#         torfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.tor'
#         outfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.out'
#         pfcfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.pfc'
        
#         fctDF = pd.read_csv(fctfile,delimiter=' ')
#         shortfctDF = fctDF[(fctDF["incastflow"]==1)&(fctDF["priority"]==3)]
        
#         tcpshortfctDF = fctDF[(fctDF["flowsize"]<100000)&(fctDF["priority"]==1)]
#         shortfct = list(tcpshortfctDF["slowdown"])
#         shortfct.sort()
#         fct99 = shortfct[int(len(shortfct)*0.99)-1]
#         tcpshortfct99.append(fct99)
        
#         shortfct = list(shortfctDF["slowdown"])
#         shortfct.sort()
#         fct95 = shortfct[int(len(shortfct)*0.95)-1]
#         shortfct95.append(fct95)
#         fct99 = shortfct[int(len(shortfct)*0.99)-1]
#         shortfct99.append(fct99)
#         fct999 = shortfct[int(len(shortfct)*0.999)-1]
#         shortfct999.append(fct999)
#         shortfctavg.append(np.mean(shortfct))
        
#         pfcDF = pd.read_csv(pfcfile,delimiter=' ')
#         numpfc.append(len(pfcDF))
        
#         torDF = pd.read_csv(torfile,delimiter=' ')
#         lossybuf = torDF["egressOccupancyLossy"]
#         losslessbuf = torDF["egressOccupancyLossless"]
#         totalbuf = torDF["totalused"]
#         lossybuf = list(100*lossybuf/buffer)
#         losslessbuf = list(100*losslessbuf/buffer)
#         totalbuf = list(100*totalbuf/buffer)
#         lossybuf.sort()
#         losslessbuf.sort()
#         totalbuf.sort()
#         lossy.append(lossybuf[int(len(lossybuf)*0.99)])
#         lossless.append(losslessbuf[int(len(lossybuf)*0.99)])
#         total.append(totalbuf[int(len(lossybuf)*0.99)])
        
#         longfctDF = fctDF[(fctDF["flowsize"]>1000000)&(fctDF["priority"])==1]
#         longfct = list(longfctDF["slowdown"])
#         longfct.sort()
#         # longfctav.append(longfct[int(len(longfct)*0.99)])
#         longfctav.append(np.mean(longfct))
        
        
#         medfctDF = fctDF[(fctDF["flowsize"]<1000000)&(fctDF["flowsize"]>100000)&(fctDF["priority"])==1]
#         medfct = list(longfctDF["slowdown"])
#         medfct.sort()
#         # medfctav.append(medfct[int(len(medfct)*0.99)])
#         medfctav.append(np.mean(medfct))
        
#         # print (alg,rdmacc,tcpcc,rdmaload,tcpload,rdmaburst,tcpburst,egresslossyFrac,gamma, numpfc[-1],shortfctavg[-1],shortfct95[-1],shortfct99[-1],shortfct999[-1],tcpshortfct99[-1],lossless[-1],lossy[-1],0,0)
#         print(tcpload,alg,shortfctavg[-1],numpfc[-1],tcpshortfct99[-1],lossless[-1],lossy[-1])

# exit(1)
#%%

# dump="dump_sigcomm/"

# loads=["0.2","0.4","0.6","0.8"]
# loadsint=[0.2,0.4,0.6,0.8]

# # bursts=["500000", "1000000"]
# bursts=["500000", "1000000","1500000", "2000000"]

# rdmacc=str(DCQCNCC)
# tcpcc=str(CUBIC)
# tcpburst="0"
# tcpload="0.8"
# egresslossyFrac="0.8"
# gamma="0.999"

# rdmaload="0"

# print("rdmaburst","alg","shortfct95","numpfc","tcpshortfct99","lossless","lossy")
# for rdmaburst in bursts:
#     shortfct95=list()
#     shortfct99=list()
#     shortfct999=list()
#     shortfctavg=list()
#     tcpshortfct99=list()
#     numpfc=list()
#     lossy=list()
#     lossless=list()
#     total=list()
#     for alg in algs:
#         fctfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.fct'
#         torfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.tor'
#         outfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.out'
#         pfcfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.pfc'
        
#         fctDF = pd.read_csv(fctfile,delimiter=' ')
#         shortfctDF = fctDF[(fctDF["incastflow"]==1)&(fctDF["priority"]==3)]
        
#         tcpshortfctDF = fctDF[(fctDF["flowsize"]<100000)&(fctDF["priority"]==1)]
#         shortfct = list(tcpshortfctDF["slowdown"])
#         shortfct.sort()
#         fct99 = shortfct[int(len(shortfct)*0.99)]
#         tcpshortfct99.append(fct99)
        
#         shortfct = list(shortfctDF["slowdown"])
#         shortfct.sort()
#         if len(shortfct)>0:
#             fct95 = shortfct[int(len(shortfct)*0.95)-1]
#             fct99 = shortfct[int(len(shortfct)*0.99)-1]
#             fct999 = shortfct[int(len(shortfct)*0.999)-1]
#         else:
#             fct95=0
#             fct99=0
#             fct999=0
#         shortfct99.append(fct99)
#         shortfct95.append(fct95)
#         shortfct999.append(fct999)
#         shortfctavg.append(np.mean(shortfct))
        
#         pfcDF = pd.read_csv(pfcfile,delimiter=' ')
#         numpfc.append(len(pfcDF))
        
#         torDF = pd.read_csv(torfile,delimiter=' ')
#         lossybuf = torDF["egressOccupancyLossy"]
#         losslessbuf = torDF["egressOccupancyLossless"]
#         totalbuf = torDF["totalused"]
#         lossybuf = list(100*lossybuf/buffer)
#         losslessbuf = list(100*losslessbuf/buffer)
#         totalbuf = list(100*totalbuf/buffer)
#         lossybuf.sort()
#         losslessbuf.sort()
#         totalbuf.sort()
#         lossy.append(lossybuf[int(len(lossybuf)*0.99)])
#         lossless.append(losslessbuf[int(len(lossybuf)*0.99)])
#         total.append(totalbuf[int(len(lossybuf)*0.99)])

#         # print (alg,rdmacc,tcpcc,rdmaload,tcpload,rdmaburst,tcpburst,egresslossyFrac,gamma, numpfc[-1],shortfctavg[-1],shortfct95[-1],shortfct99[-1],shortfct999[-1],tcpshortfct99[-1],lossless[-1],lossy[-1],0,0)
#         print(rdmaburst,alg,shortfctavg[-1],numpfc[-1],tcpshortfct99[-1],lossless[-1],lossy[-1])
    
# exit(1)
#%%

# dump="loveland_dump/"

# loads=["0.2","0.4","0.6","0.8"]
# loadsint=[0.2,0.4,0.6,0.8]

# # bursts=["500000", "1000000"]
# bursts=["500000", "1000000","1500000", "2000000"]

# rdmacc=str(INTCC)
# tcpcc=str(CUBIC)
# rdmaburst="0"
# tcpload="0"
# egresslossyFrac="0.8"
# gamma="0.999"

# tcpburst="1500000"

# print("rdmaload","alg","shortfct95","numpfc","tcpshortfct99","lossless","lossy")
# for rdmaload in loads:
#     shortfct95=list()
#     shortfct99=list()
#     shortfct999=list()
#     shortfctavg=list()
#     rdmashortfct99=list()
#     rdmalongfctav=list()
#     numpfc=list()
#     lossy=list()
#     lossless=list()
#     total=list()
#     for alg in algs:
#         fctfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.fct'
#         torfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.tor'
#         outfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.out'
#         pfcfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.pfc'
        
#         fctDF = pd.read_csv(fctfile,delimiter=' ')
#         shortfctDF = fctDF[(fctDF["incastflow"]==1)&(fctDF["priority"]==1)] # fctDF[(fctDF["flowsize"]<100000)&(fctDF["priority"]==3)] #
        
#         rdmashortfctDF = fctDF[(fctDF["flowsize"]<100000)&(fctDF["priority"]==3)]
#         shortfct = list(rdmashortfctDF["slowdown"])
#         shortfct.sort()
#         fct99 = shortfct[int(len(shortfct)*0.99)]
#         rdmashortfct99.append(fct99)
        
#         rdmashortfctDF = fctDF[(fctDF["flowsize"]>1000000)&(fctDF["priority"]==3)]
#         shortfct = list(rdmashortfctDF["slowdown"])
#         shortfct.sort()
#         fct = np.median(shortfct)
#         rdmalongfctav.append(fct)
        
#         shortfct = list(shortfctDF["slowdown"])
#         shortfct.sort()
#         if len(shortfct)>0:
#             fct95 = shortfct[int(len(shortfct)*0.95)]
#             fct99 = shortfct[int(len(shortfct)*0.99)]
#             fct999 = shortfct[int(len(shortfct)*0.999)]
#         else:
#             fct95=0
#             fct99=0
#             fct999=0
#         shortfct99.append(fct99)
#         shortfct95.append(fct95)
#         shortfct999.append(fct999)
#         shortfctavg.append(np.mean(shortfct))
        
#         pfcDF = pd.read_csv(pfcfile,delimiter=' ')
#         numpfc.append(len(pfcDF))
        
#         torDF = pd.read_csv(torfile,delimiter=' ')
#         lossybuf = torDF["egressOccupancyLossy"]
#         losslessbuf = torDF["egressOccupancyLossless"]
#         totalbuf = torDF["totalused"]
#         lossybuf = list(100*lossybuf/buffer)
#         losslessbuf = list(100*losslessbuf/buffer)
#         totalbuf = list(100*totalbuf/buffer)
#         lossybuf.sort()
#         losslessbuf.sort()
#         totalbuf.sort()
#         lossy.append(lossybuf[int(len(lossybuf)*0.99)])
#         lossless.append(losslessbuf[int(len(lossybuf)*0.99)])
#         total.append(totalbuf[int(len(lossybuf)*0.99)])

#         # print (alg,rdmacc,tcpcc,rdmaload,tcpload,rdmaburst,tcpburst,egresslossyFrac,gamma, numpfc[-1],shortfctavg[-1],shortfct95[-1],shortfct99[-1],shortfct999[-1],rdmashortfct99[-1],lossless[-1],lossy[-1],0,0)
        
#         print(rdmaload,alg,shortfctavg[-1],numpfc[-1],rdmashortfct99[-1],lossless[-1],lossy[-1])

# exit(1)
#%%
# dump="loveland_dump/"

# loads=["0.2","0.4","0.6","0.8"]
# loadsint=[0.2,0.4,0.6,0.8]

# rdmacc=str(INTCC)
# tcpcc=str(CUBIC)
# tcpload="0"
# egresslossyFrac="0.8"
# gamma="0.999"

# rdmaload="0.8"
# rdmaburst="0"


# burststemp=["12500","500000", "1000000","1500000"]
# print("tcpburst","alg","shortfct95","numpfc","tcpshortfct99","lossless","lossy")
# for tcpburst in burststemp:
#     shortfct95=list()
#     shortfct99=list()
#     shortfct999=list()
#     shortfctavg=list()
#     rdmashortfct99=list()
#     rdmalongfctav=list()
#     numpfc=list()
#     lossy=list()
#     lossless=list()
#     total=list()
#     for alg in algs:
#         fctfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.fct'
#         torfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.tor'
#         outfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.out'
#         pfcfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.pfc'
        
#         fctDF = pd.read_csv(fctfile,delimiter=' ')
#         shortfctDF = fctDF[(fctDF["incastflow"]==1)&(fctDF["priority"]==1)] #fctDF[(fctDF["flowsize"]<100000)&(fctDF["priority"]==3)]
        
#         rdmashortfctDF = fctDF[(fctDF["flowsize"]<100000)&(fctDF["priority"]==3)]
#         shortfct = list(rdmashortfctDF["slowdown"])
#         shortfct.sort()
#         fct99 = shortfct[int(len(shortfct)*0.99)]
#         rdmashortfct99.append(fct99)
        
#         rdmashortfctDF = fctDF[(fctDF["flowsize"]>1000000)&(fctDF["priority"]==3)]
#         shortfct = list(rdmashortfctDF["slowdown"])
#         shortfct.sort()
#         fct =  np.median(shortfct)
#         rdmalongfctav.append(fct)
        
#         shortfct = list(shortfctDF["slowdown"])
#         shortfct.sort()
#         if len(shortfct)>0:
#             fct95 = shortfct[int(len(shortfct)*0.95)]
#             fct99 = shortfct[int(len(shortfct)*0.99)]
#             fct999 = shortfct[int(len(shortfct)*0.999)]
#         else:
#             fct95=0
#             fct99=0
#             fct999=0
#         shortfct99.append(fct99)
#         shortfct95.append(fct95)
#         shortfct999.append(fct999)
#         shortfctavg.append(np.mean(shortfct))
        
#         pfcDF = pd.read_csv(pfcfile,delimiter=' ')
#         numpfc.append(len(pfcDF))
        
#         torDF = pd.read_csv(torfile,delimiter=' ')
#         lossybuf = torDF["egressOccupancyLossy"]
#         losslessbuf = torDF["egressOccupancyLossless"]
#         totalbuf = torDF["totalused"]
#         lossybuf = list(100*lossybuf/buffer)
#         losslessbuf = list(100*losslessbuf/buffer)
#         totalbuf = list(100*totalbuf/buffer)
#         lossybuf.sort()
#         losslessbuf.sort()
#         totalbuf.sort()
#         lossy.append(lossybuf[int(len(lossybuf)*0.99)])
#         lossless.append(losslessbuf[int(len(lossybuf)*0.99)])
#         total.append(totalbuf[int(len(lossybuf)*0.99)])

#         # print (alg,rdmacc,tcpcc,rdmaload,tcpload,rdmaburst,tcpburst,egresslossyFrac,gamma, numpfc[-1],shortfctavg[-1],shortfct95[-1],shortfct99[-1],shortfct999[-1],rdmashortfct99[-1],lossless[-1],lossy[-1],0,0)
        
#         print(tcpburst,alg,shortfctavg[-1],numpfc[-1],rdmashortfct99[-1],lossless[-1],lossy[-1])

# exit(1)

#%%

# plt.rcParams.update({'font.size': 14})
# dump="dump_sigcomm/"

# loads=["0.2","0.4","0.6","0.8"]
# loadsint=[0.2,0.4,0.6,0.8]

# # bursts=["500000", "1000000"]
# bursts=["500000", "1000000","1500000", "2000000", "2500000"]

# rdmacc=str(DCQCNCC)
# tcpcc=str(CUBIC)
# rdmaburst="2000000"
# tcpburst="0"
# egresslossyFrac="0.8"
# gamma="0.999"
# tcpload="0"
# rdmaload="0.8"

# fig0,ax0 = plt.subplots(1,1,figsize=(6,3))
# fig1,ax1 = plt.subplots(1,1,figsize=(6,3))
# fig2,ax2 = plt.subplots(1,1,figsize=(6,3))
# fig3,ax3 = plt.subplots(1,1,figsize=(6,3))
# fig4,ax4 = plt.subplots(1,1,figsize=(6,3))


# alg=str(REVERIE)
# numpfc=list()
# shortfct95=list()
# shortfct99=list()
# shortfct999=list()
# shortfctavg=list()
# print("gamma","shortfctavg","numpfc")
# for gamma in ["0.8","0.9","0.99","0.999","0.999999"]:
#     fctfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.fct'
    
#     fctDF = pd.read_csv(fctfile,delimiter=' ')
#     shortfctDF = fctDF[(fctDF["incastflow"]==1)&(fctDF["priority"]==3)] #fctDF[(fctDF["flowsize"]<100000)&(fctDF["priority"]==3)]   
#     shortfct = list(shortfctDF["slowdown"])
#     shortfct.sort()
#     if len(shortfct)>0:
#         fct95 = shortfct[int(len(shortfct)*0.95)]
#         fct99 = shortfct[int(len(shortfct)*0.99)]
#         fct999 = shortfct[int(len(shortfct)*0.999)]
#     else:
#         fct95=0
#         fct99=0
#         fct999=0
#     shortfct99.append(fct99)
#     shortfct95.append(fct95)
#     shortfct999.append(fct999)
#     shortfctavg.append(np.mean(shortfct))
    
#     pfcfile = dump+"evaluation-"+alg+'-'+rdmacc+'-'+tcpcc+'-'+rdmaload+'-'+tcpload+'-'+rdmaburst+'-'+tcpburst+'-'+egresslossyFrac+'-'+gamma+'.pfc'
#     pfcDF = pd.read_csv(pfcfile,delimiter=' ')
#     numpfc.append(len(pfcDF))

#     print(gamma,shortfctavg[-1],numpfc[-1])

# exit(1)