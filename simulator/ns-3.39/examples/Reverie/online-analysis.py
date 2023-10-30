#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Tue Nov 30 13:14:40 2022

@author: vamsi
"""

import numpy as np
import matplotlib.pyplot as plt
import math
from mpl_toolkits.mplot3d import Axes3D
import  random
from matplotlib.colors import LogNorm, Normalize
import pandas as pd
#%%

T=100

time=np.arange(T)


N = 48

M = 1024*8

minAlpha = 1/(M-1)
maxAlpha = M-1

fig,ax = plt.subplots(1,1)
ax.set_xlabel("alpha")
df=pd.DataFrame()
comp=list()
nums=list()
alphas=np.arange(minAlpha,maxAlpha,0.01)
df["alphas"]=alphas
for alpha in alphas:
    num = np.min([np.floor(np.log(alpha*M)/np.log(1+alpha)), N])
    if num == N:
        num = int(alpha*M/((1+alpha)**N)) * N
    nums.append(num)
    c = (M /num)
    comp.append(c)

df["competitive"]=comp
df["num"]=nums
ax.plot(alphas,comp)
ax.set_xscale('log')

#%%
fig,ax = plt.subplots(1,1)
# ax.plot(np.arange(8,1025), [M/np.log2(M) for M in range(8,1025)])
it = np.arange(12,1025)
ax.plot(it, [np.log10(M)/(np.log10(np.log10(M))) for M in it])
ax.set_xscale('log',basex=2)

#%%

df = pd.read_csv('temp1.dat',delimiter=' ', header=None,names=["iat","flowsize","time","check"])

df.sort_values('iat')


#%%

fig,ax = plt.subplots(1,1)

time = np.arange(0,5001)

b = 10*10**9
r = 2*b

B = 9 * 10**6
alpha = 0.25

iburst = list()
dt = list()
av = list()
avq = list()
dtthresh = list()
avg = 0
avburst = 0
first = 1
for t in time:
    if (t <= 3000):
        burst = (r-b)*t/(10**6)/8
        if (burst < alpha*(B-burst)):
            dtburst = burst
        else:
            dtburst = alpha*B/(1+alpha)
        if (avg < alpha*(B-burst)):
            avburst = burst
        else:
            if (first==1):
                maxav = burst
                first = 0
            avburst = maxav
    else:
        burst = np.max([0,int(np.min([(r-b)*200/(10**6)/8,B]) - b*(t-200)/(10**6)/8)])
        dtburst = np.max([0,int(np.min([(r-b)*200/(10**6)/8,alpha*B/(1+alpha)]) - b*(t-200)/(10**6)/8)])
        avburst = np.max([0,int(np.min([(r-b)*200/(10**6)/8,maxav]) - b*(t-200)/(10**6)/8)])

    avg = 0.01*(avburst) + 0.99*avg
    iburst.append(burst)
    dt.append(dtburst)
    av.append(avburst)
    
ax.plot(time,iburst,label='burst')
ax.plot(time,dt,label='dt queue')
ax.plot(time,av,label='avg dt queue')
ax.legend()