#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Tue Oct 25 14:45:21 2022

@author: vamsi
"""

import os
import sys
import numpy as np
import pandas as pd
# import matplotlib
# matplotlib.use('Agg')}
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from scipy.stats import cumfreq
from matplotlib.patches import Patch
from matplotlib.lines import Line2D
from matplotlib.ticker import FormatStrFormatter

# plt.style.use('grayscale')
# plt.cm.gray

NS3="/home/vamsi/Dropbox/src/phd/codebase/ns3-datacenter/simulator/ns-3.35/"

#%%

nodeId = 1
portId = 4
nodeType = "ToRServer"
qIndex = 1 # exclude control packets

df = pd.read_csv(NS3+"arrival.txt",delimiter=' ')
df = df[(df["time"]>10020000000)&(df["qIndex"]==1)&(df["nodeId"]==nodeId)&(df["portId"]==portId)&(df["type"]==nodeType)&(df["qIndex"]==qIndex)]
#%%

Bdp = df[(df["unsched"]==1)]
nonBdp = df[(df["unsched"]==0)]

#%%

interarrival = list([ list(df["time"])[i+1]-list(df["time"])[i] for i in range(0,len(df["time"])-1)  ])

#%%
interarrivalBdp = list([ list(Bdp["time"])[i+1]-list(Bdp["time"])[i] for i in range(0,len(Bdp["time"])-1) ])

#%%

interarrivalnonBdp = list([ list(nonBdp["time"])[i+1]-list(nonBdp["time"])[i] for i in range(0,len(nonBdp["time"])-1)  ])

#%%

x, y = sorted(interarrivalBdp), np.arange(len(interarrivalBdp)) / len(interarrivalBdp)

fig,ax= plt.subplots(1,1)

ax.plot(x,y)
lam = 0.45*1e-3
ax.plot(x,[1-np.exp(-lam*i) for i in x])
ax.set_xscale('log')

#%%
x, y = sorted(interarrivalnonBdp), np.arange(len(interarrivalnonBdp)) / len(interarrivalnonBdp)

fig,ax= plt.subplots(1,1)

ax.plot(x,y)
lam = 0.55*1e-3
ax.plot(x,[1-np.exp(-lam*i) for i in x])
ax.set_xscale('log')

#%%
x, y = sorted(interarrival), np.arange(len(interarrival)) / len(interarrival)

fig,ax= plt.subplots(1,1)

ax.plot(x,y)
lam = 0.45*1e-3
ax.plot(x,[1-np.exp(-lam*i) for i in x])
ax.set_xscale('log')