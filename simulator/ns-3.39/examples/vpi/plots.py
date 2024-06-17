#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Tue May 28 03:32:37 2024

@author: vamsi
"""

import numpy as np
import matplotlib.pyplot as plt
import math
from mpl_toolkits.mplot3d import Axes3D
import  random
from matplotlib.colors import LogNorm, Normalize
import matplotlib.cm as cm
import matplotlib.colors as mcolors
import pandas as pd
import numpy as np
import sys

#%%
dump="dump_vpi/"
results="results_vpi/"
plots="/home/vamsi/src/phd/writings/ml-transport/plots/"

DCQCNCC=1
INTCC=3
TIMELYCC=7
PINTCC=10
CUBIC=2
DCTCP=4
DCTCPCC=8

FLOW_ECMP=0
RANDOM_ECMP=1
SOURCE_ROUTING=2

ALL_TO_ALL=666
ALL_REDUCE=667

RING=777
TREE=778


RDMACC=DCTCPCC


colors={}

colors[FLOW_ECMP]={}
colors[FLOW_ECMP][True]='r'
colors[FLOW_ECMP][False]='y'

colors[RANDOM_ECMP]={}
colors[RANDOM_ECMP][True]='g'
colors[RANDOM_ECMP][False]='b'

colors[SOURCE_ROUTING]={}
colors[SOURCE_ROUTING][True]='k'

labels={}

labels[FLOW_ECMP]={}
labels[FLOW_ECMP][True]='Ecmp-permute'
labels[FLOW_ECMP][False]='Ecmp'

labels[RANDOM_ECMP]={}
labels[RANDOM_ECMP][True]='Spray-permute'
labels[RANDOM_ECMP][False]='Spray'

labels[SOURCE_ROUTING]={}
labels[SOURCE_ROUTING][True]='SR-permute'

markers={}

markers[FLOW_ECMP]={}
markers[FLOW_ECMP][True]='o'
markers[FLOW_ECMP][False]='o'

markers[RANDOM_ECMP]={}
markers[RANDOM_ECMP][True]='s'
markers[RANDOM_ECMP][False]='s'

markers[SOURCE_ROUTING]={}
markers[SOURCE_ROUTING][True]='D'

TRANSFER_SIZES=[8000,16000,32000,64000,128000,256000]
QP_WINDOWS=[8,16,32,64,128,256]
#%%

df = pd.read_csv(results+"opeth.dat",delimiter=" ")

qpWindow=256

fig,ax = plt.subplots(1,1)
ax.xaxis.grid('True',ls='--')
ax.yaxis.grid('True',ls='--')
ax.set_xlabel("Transfer size (KB)")
ax.set_ylabel("Completion time (ms)")
for randomize in [True, False]:
    for routing in [FLOW_ECMP, RANDOM_ECMP, SOURCE_ROUTING]:
        if (routing == SOURCE_ROUTING and randomize == False):
            continue
        dfX = df[(df["randomize"]==randomize) & (df["routing"]==routing) & (df["window"]==qpWindow)]
        if randomize == True:
            ax.plot(dfX["transferSize"]/1000,dfX["completionTime"]/10**6,c = colors[routing][randomize],label=labels[routing][randomize],marker=markers[routing][randomize], markersize = 10)
        else:
            ax.plot(dfX["transferSize"]/1000,dfX["completionTime"]/10**6,c = colors[routing][randomize],label=labels[routing][randomize],marker=markers[routing][randomize], markersize = 10, mfc='none')
        # print(dfX, randomize, routing)
ax.legend()
fig.tight_layout()
fig.savefig(plots+'cct-'+str(qpWindow)+'.pdf')
#%%
# df = pd.read_csv(results+"opeth.dat",delimiter=" ")

qpWindow=256

for transferSize in TRANSFER_SIZES:
    fig, ax = plt.subplots(1,1)
    ax.xaxis.grid('True',ls='--')
    ax.yaxis.grid('True',ls='--')
    ax.set_xlabel("Completion time (ms)")
    ax.set_ylabel("CDF")
    for randomize in [True, False]:
        for routing in [FLOW_ECMP, RANDOM_ECMP, SOURCE_ROUTING]:
            if (routing == SOURCE_ROUTING and randomize == False):
                continue
            if (routing == RANDOM_ECMP):
                multiPath = "true"
            else:
                multiPath = "false"
            if randomize == True:
                rand = "true"
            else:
                rand = "false"
            df = pd.read_csv(dump+"evaluation-"+str(RDMACC)+'-'+str(multiPath)+'-'+str(routing)+'-'+str(rand)+'-'+str(qpWindow)+'-'+str(transferSize)+".fct", delimiter = " ", usecols=[2])
            # dfX = df[(df["randomize"]==randomize) & (df["routing"]==routing) & (df["window"]==qpWindow)]
            fcts = df["fctus"]
            x=np.sort(fcts/10**6)
            y=np.arange(len(fcts))/float(len(fcts))
            ax.plot(x,y,c = colors[routing][randomize])
            ax.set_title('Transfer size = '+str(transferSize))
            if randomize == True:
                ax.plot(x[-1],y[-1],c = colors[routing][randomize],marker=markers[routing][randomize], markersize = 10,label=labels[routing][randomize])
            else:
                ax.plot(x[-1],y[-1],c = colors[routing][randomize],marker=markers[routing][randomize], markersize = 10,mfc='none',label=labels[routing][randomize])
            # print(dfX)
    ax.legend()
    fig.tight_layout()
    fig.savefig(plots+'cct-cdf-'+str(qpWindow)+'-'+str(transferSize)+'.pdf')

#%%

qpWindow=256

for transferSize in TRANSFER_SIZES:
    fig, ax = plt.subplots(1,1)
    ax.xaxis.grid('True',ls='--')
    ax.yaxis.grid('True',ls='--')
    ax.set_xlabel("Shared buffer occupancy (KB)")
    ax.set_ylabel("CDF")
    for randomize in [True, False]:
        for routing in [FLOW_ECMP, RANDOM_ECMP, SOURCE_ROUTING]:
            if (routing == SOURCE_ROUTING and randomize == False):
                continue
            if (routing == RANDOM_ECMP):
                multiPath = "true"
            else:
                multiPath = "false"
            if randomize == True:
                rand = "true"
            else:
                rand = "false"
            df = pd.read_csv(dump+"evaluation-"+str(RDMACC)+'-'+str(multiPath)+'-'+str(routing)+'-'+str(rand)+'-'+str(qpWindow)+'-'+str(transferSize)+".tor", delimiter = " ", usecols=[0,1])
            df = df[df["switch"]==0]
            # dfX = df[(df["randomize"]==randomize) & (df["routing"]==routing) & (df["window"]==qpWindow)]
            buffer = df["totalused"]
            x=np.sort(buffer/10**3)
            y=np.arange(len(buffer))/float(len(buffer))
            ax.plot(x,y,c = colors[routing][randomize])
            ax.set_title('Transfer size = '+str(transferSize))
            if randomize == True:
                ax.plot(x[-1],y[-1],c = colors[routing][randomize],marker=markers[routing][randomize], markersize = 10,label=labels[routing][randomize])
            else:
                ax.plot(x[-1],y[-1],c = colors[routing][randomize],marker=markers[routing][randomize], markersize = 10,mfc='none',label=labels[routing][randomize])
            # print(dfX)
    ax.legend()
    fig.tight_layout()
    fig.savefig(plots+'buffer-cdf-'+str(qpWindow)+'-'+str(transferSize)+'.pdf')


    
#%%

colormap = cm.jet
normalize = mcolors.Normalize(vmin=np.min(QP_WINDOWS), vmax=np.max(QP_WINDOWS))
colorsmap = colormap(normalize(QP_WINDOWS))

df = pd.read_csv(results+"opeth.dat",delimiter=" ")
routing=FLOW_ECMP

fig,ax = plt.subplots(1,1)
ax.xaxis.grid('True',ls='--')
ax.yaxis.grid('True',ls='--')
ax.set_xlabel("Transfer size (KB)")
ax.set_ylabel("Completion time (ms)")
for randomize in [True, False]:
    color = 0
    for qpWindow in QP_WINDOWS:
        dfX = df[(df["randomize"]==randomize) & (df["routing"]==routing) & (df["window"]==qpWindow)]
        if randomize == True:
            ax.plot(dfX["transferSize"],dfX["completionTime"]/10**6,c=colorsmap[color],label="w="+str(qpWindow),marker=markers[routing][randomize], markersize = 10)
        else:
            ax.plot(dfX["transferSize"],dfX["completionTime"]/10**6,c=colorsmap[color],label="w="+str(qpWindow),marker=markers[routing][randomize], markersize = 10, mfc='none')
        # print(dfX, randomize, routing)
        color = color + 1
ax.legend(ncols=2,framealpha=0)
#%%
# df = pd.read_csv(results+"opeth.dat",delimiter=" ")

routing=FLOW_ECMP
multiPath = "false"

colormap = cm.jet
normalize = mcolors.Normalize(vmin=np.min(QP_WINDOWS), vmax=np.max(QP_WINDOWS))
colorsmap = colormap(normalize(QP_WINDOWS))


for transferSize in TRANSFER_SIZES:
    color = 0
    fig, ax = plt.subplots(1,1)
    ax.xaxis.grid('True',ls='--')
    ax.yaxis.grid('True',ls='--')
    ax.set_xlabel("Completion time (ms)")
    ax.set_ylabel("CDF")
    for qpWindow in QP_WINDOWS:
        for randomize in [True, False]:
            if randomize == True:
                rand = "true"
            else:
                rand = "false"
            df = pd.read_csv(dump+"evaluation-"+str(RDMACC)+'-'+str(multiPath)+'-'+str(routing)+'-'+str(rand)+'-'+str(qpWindow)+'-'+str(transferSize)+".fct", delimiter = " ", usecols=[2])
            # dfX = df[(df["randomize"]==randomize) & (df["routing"]==routing) & (df["window"]==qpWindow)]
            fcts = df["fctus"]
            x=np.sort(fcts/10**6)
            y=np.arange(len(fcts))/float(len(fcts))
            ax.plot(x,y,c = colorsmap[color])
            ax.set_title('Transfer size = '+str(transferSize)+ " (ECMP)")
            if randomize == True:
                ax.plot(x[-1],y[-1],c = colorsmap[color],marker=markers[routing][randomize], markersize = 10,label="w="+str(qpWindow))
            else:
                ax.plot(x[-1],y[-1],c = colorsmap[color],marker=markers[routing][randomize], markersize = 10,mfc='none',label="w="+str(qpWindow))
            # print(dfX)
        color = color + 1
    ax.legend()
    
    
    
#%%