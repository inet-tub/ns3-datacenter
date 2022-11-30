#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Tue Mar 15 23:19:40 2022

@author: vamsi
"""

import numpy as np
import matplotlib.pyplot as plt
import math
from mpl_toolkits.mplot3d import Axes3D
import  random
from matplotlib.colors import LogNorm, Normalize

#%%

plt.rcParams.update({'font.size': 14})


buffer = [4, 9, 12, 16, 22]

bufferperPortperGbps = [85, 19.2, 9.6, 5.12, 3.44]

bufferperPort = [85*48, 19.2*48, 9.6*32, 5.12*32, 3.44*64]

cap = [48*1, 48*10, 32*40, 32*100, 64*100]

portCap = [1, 10, 40, 100, 100]


labels=["Br. 56538", "Trident+", "Trident II", "Tomahawk", "Tofino"]
#%%
xoff = list()
speed_light=3*10**8*0.65
p_delay = 100/speed_light

for i in range(len(portCap)):
    x = (2250*2 + 3850 + portCap[i]*(10**9)*p_delay*2)/portCap[i]
    print(x)
    xoff.append(x/1000)

fig,ax = plt.subplots(1,1)
fig1,ax1 = plt.subplots(1,1)

ax.plot(np.arange(len(cap)), cap,marker='x',markersize=20,lw=4,c='g')
# ax.set_yscale('log')
ax.xaxis.grid(True,ls='--')
ax.yaxis.grid(True,ls='--')
ax.set_ylabel('Gbps')
ax.set_xticks(np.arange(len(cap)))
ax.set_xticklabels(labels)


ax1.plot(np.arange(len(cap)), xoff, marker='^',markersize=20,lw=4,c='b')
ax1.plot(np.arange(len(cap)), bufferperPort,marker='s',markersize=20,lw=4,c='g')
ax1.plot(np.arange(len(cap)), bufferperPortperGbps,marker='x',markersize=20,lw=4,c='r')
ax1.set_yscale('log')
ax1.xaxis.grid(True,ls='--')
ax1.yaxis.grid(True,ls='--')
ax1.set_ylabel('KB per Port per GBPS')
ax1.set_xticks(np.arange(len(cap)))
ax1.set_xticklabels(labels)
# fig1.savefig('buffer.svg')

#%%
plt.rcParams.update({'font.size': 14})

egresslossyOcc = np.arange(1,14)

ingressBuffer = 18

alpha=1

thresh=[alpha*(ingressBuffer - egresslossy)/(1+alpha) for egresslossy in egresslossyOcc]

fig,ax = plt.subplots(1,1)
ax.set_xlabel("Egress lossy occupied buffer (MB)")
ax.set_ylabel("Ingres pfc threshold (MB)")
ax.xaxis.grid(True,ls='--')
ax.yaxis.grid(True,ls='--')

ax.plot(egresslossyOcc, thresh)

N=10

thresh = [alpha*(4)/(1+n*alpha) for n in np.arange(N)]
fig,ax = plt.subplots(1,1)
# ax.set_xlabel("Egress lossy occupied buffer")
ax.set_ylabel("Ingres pfc threshold (MB)")
ax.xaxis.grid(True,ls='--')
ax.yaxis.grid(True,ls='--')

ax.plot(np.arange(N), thresh)


#%%

plt.rcParams.update({'font.size': 14})
plotsdir="/home/vamsi/src/phd/writings/rdma-buffer/hotnets22/plots/"


def pfcThreshold(ning, negg):
    thresh = alphaing*(Bing + negg*alphaegg*(Bing-Begg))/((1+ning*alphaing)*(1+negg*alphaegg))
    return thresh

def egressLossyThreshold(ning, negg):
    thresh = alphaegg*Begg/(1+negg*alphaegg)
    return thresh


def lossyOccupancy(ning, negg):
    total =  Begg *( (negg*alphaegg)/(1+negg*alphaegg) )
    return total


def losslessOccupancy(ning, negg):
    total = Bing * ( (ning*alphaing)/(1+ning*alphaing) ) - Begg *( ((ning*alphaing)/(1+ning*alphaing))*( (negg*alphaegg)/(1+negg*alphaegg) )  )
    return total

def lossylosslessratio(ning,negg):
    return (lossyOccupancy(ning,negg)/losslessOccupancy(ning,negg))


fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')


alphaing = 1
alphaegg = 1
Bing = 5.9
Begg = 4.7
x = y = np.arange(0,10)
X, Y = np.meshgrid(x, y)
zs = np.array(pfcThreshold(np.ravel(X), np.ravel(Y)))
Z = zs.reshape(X.shape)
ax.plot_surface(X, Y, Z)
# ax.set_zlim(0,3)


alphaing = 1
alphaegg = 1
Bing = 5.9
Begg = 3
x = y = np.arange(0,10)
X, Y = np.meshgrid(x, y)
zs = np.array(pfcThreshold(np.ravel(X), np.ravel(Y)))
Z = zs.reshape(X.shape)
ax.plot_surface(X, Y, Z,alpha=0.3)
# ax.set_zlim(0,2)

# ax.set_zticks([0,1,2])
ax.set_zticks([2,4,6])
ax.set_xlabel('\n# ingress \nlossless queues')
ax.set_ylabel('\n# egrees \nlossy queues')
ax.set_zlabel('\npfc threshold \n(MB)')
# ax.set_zlim(0,9)
fig.tight_layout()
fig.savefig(plotsdir+'pfcthresh.pdf')
#%%

fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')

alphaing = 1
alphaegg = 1
Bing = 5.9
Begg = 4.7
x = y = np.arange(0,10)
X, Y = np.meshgrid(x, y)
zs = np.array(egressLossyThreshold(np.ravel(X), np.ravel(Y)))
Z = zs.reshape(X.shape)
ax.plot_surface(X, Y, Z)

alphaing = 1
alphaegg = 1
Bing = 5.9
Begg = 3
x = y = np.arange(0,10)
X, Y = np.meshgrid(x, y)
zs = np.array(egressLossyThreshold(np.ravel(X), np.ravel(Y)))
Z = zs.reshape(X.shape)
ax.plot_surface(X, Y, Z,alpha=0.3)

ax.set_zticks([2,4,6])
ax.set_xlabel('\n# ingress \nlossless queues')
ax.set_ylabel('\n# egrees \nlossy queues')
ax.set_zlabel('\ndrop threshold \n(MB)')
# ax.set_zlim(0,9)
fig.tight_layout()
fig.savefig(plotsdir+'dropthresh.pdf')


#%%


import seaborn as sns; sns.set_theme()

plt.rcParams.update({'font.size': 18})

plotsdir="/home/vamsi/src/phd/writings/rdma-buffer/hotnets22/plots/"


N=20

alphaing = 1
alphaegg = 1
Bing = 5.9
Begg = 4.7
ratiogrid=np.zeros((N,N))

for i in range(1,N+1):
    for j in range (1,N+1):
        ratiogrid[i-1][j-1]=lossylosslessratio(i,j)

maxValue = ratiogrid[0][N-1]

sns.set(font_scale=1.2)
ax = sns.heatmap(ratiogrid,cmap='RdYlGn_r',vmin=0,vmax=4)
ax.set_title("Lossy/Lossless Buffer allocation ratio")
ax.set_xlabel("# Egress lossy queues")
ax.set_ylabel("# Ingress lossless queues")
ax.set_xticks([1,4,8,12,16,20])
ax.set_xticklabels([1,4,8,12,16,20],rotation=0)
ax.set_yticks([1,4,8,12,16,20])
ax.set_yticklabels([1,4,8,12,16,20],rotation=0)
ax.figure.tight_layout()
ax.figure.savefig(plotsdir+'heatmap-ratio-main.pdf')

plt.clf()
alphaing = 1
alphaegg = 1
Bing = 5.9
Begg = 5.5
ratiogrid=np.zeros((N,N))

for i in range(1,N+1):
    for j in range (1,N+1):
        ratiogrid[i-1][j-1]=lossylosslessratio(i,j)

sns.set(font_scale=1.2)
ax = sns.heatmap(ratiogrid,cmap='RdYlGn_r',annot=False,vmin=0,vmax=4,cbar=False,yticklabels=False,xticklabels=False)
# ax.set_title("Lossy/Lossless Buffer allocation ratio")
ax.figure.tight_layout()
ax.figure.savefig(plotsdir+'heatmap-ratio-1.pdf')


plt.clf()
alphaing = 1
alphaegg = 1
Bing = 5.9
Begg = 3
ratiogrid=np.zeros((N,N))

for i in range(1,N+1):
    for j in range (1,N+1):
        ratiogrid[i-1][j-1]=lossylosslessratio(i,j)

sns.set(font_scale=1.2)
ax = sns.heatmap(ratiogrid,cmap='RdYlGn_r',annot=False,vmin=0,vmax=4,cbar=False,yticklabels=False,xticklabels=False)
# ax.set_title("Lossy/Lossless Buffer allocation ratio")
ax.figure.tight_layout()
ax.figure.savefig(plotsdir+'heatmap-ratio-2.pdf')

plt.clf()
alphaing = 1
alphaegg = 1
Bing = 5.9
Begg = 2
ratiogrid=np.zeros((N,N))

for i in range(1,N+1):
    for j in range (1,N+1):
        ratiogrid[i-1][j-1]=lossylosslessratio(i,j)

sns.set(font_scale=1.2)
ax = sns.heatmap(ratiogrid,cmap='RdYlGn_r',annot=False,vmin=0,vmax=4,cbar=False,yticklabels=False,xticklabels=False)
# ax.set_title("Lossy/Lossless Buffer allocation ratio")
ax.figure.tight_layout()
ax.figure.savefig(plotsdir+'heatmap-ratio-3.pdf')



#%%

import pandas as pd


plotsdir="/home/vamsi/src/phd/writings/rdma-buffer/hotnets22/plots/"
plt.rcParams.update({'font.size': 16})

DIR="/home/vamsi/Dropbox/src/phd/codebase/ns3-datacenter/simulator/ns-3.35/examples/buffer-devel/"

algs=["DT","ABM","DT-newbuffer","ABM-newbuffer"]
alg="DT-newbuffer"
alg="DT"
alg="ABM-newbuffer"
alg="ABM"
df = pd.read_csv(DIR+'losslesssecond'+str(alg)+'.txt',delimiter=' ',usecols=[1,3,5,7,9,11,13],names=["switch","buffer","eglossless","eglossy","ing","head","time"])
df0 = df[df["switch"]==0]
df1 = df[df["switch"]==1]

fig,ax = plt.subplots(1,1)
ax.plot(df0["time"]*1000,df0["eglossy"],label="Lossy (TCP)",lw=2,c='red')
ax.plot(df0["time"]*1000,df0["eglossless"],label="Lossless (RDMA)",lw=2,c='green')
ax.plot(df0["time"]*1000,df0["ing"],label="Shared Buffer",lw=2,c='blue',alpha=0.4,ls='--')
# ax.plot(df0["time"]*1000,df0["head"],label="headroom",lw=2)
ax.legend()
# ax.set_yscale('log')
ax.set_ylim([10000,10**7])
ax.set_xlim([0,10])
ax.set_yticks([0,2*10**6,4*10**6,6*10**6,8*10**6,10*10**6])
ax.set_yticklabels(["0","2","4","6","8","10"])
ax.set_ylabel("Buffer Occupancy (MB)")
ax.set_xlabel("Time (ms)")
ax.xaxis.grid(True,ls='--')
ax.yaxis.grid(True,ls='--')
ax.set_title(alg)
fig.tight_layout()
# fig.savefig(plotsdir+'losslesssecond.pdf')
#%%
import pandas as pd


plotsdir="/home/vamsi/src/phd/writings/rdma-buffer/hotnets22/plots/"
plt.rcParams.update({'font.size': 16})

DIR="/home/vamsi/lakewood/src/phd/codebase/ns3-datacenter/simulator/ns-3.35/examples/buffer-devel/dump_test/"

algs=["101","110"]

algNames={}
algNames["101"]="DT"
algNames["110"]="ABM"

models=["sonic","new"]

# alphas=["0.25","0.5","1","2","3","4"]
alphas=["0.25","0.5","1","2"]

for model in models:
    for alg in algs:
        fig,ax = plt.subplots(1,1)
        alphaFig=0.4
        for alpha in alphas:

            df = pd.read_csv(DIR+'tor-'+alg+'-'+alpha+'-'+model+'-losslessfirst'+'.tor',delimiter=' ',usecols=[1,3,5,7,9,11,13],names=["switch","buffer","eglossless","eglossy","ing","head","time"])
            df0 = df[df["switch"]==0]
            df1 = df[df["switch"]==1]
            if (alpha=="2"):
                ax.plot(df0["time"]*1000,df0["eglossy"],label="TCP",lw=2,c='red',alpha=alphaFig)
                ax.plot(df0["time"]*1000,df0["eglossless"]-df0["head"],label="RDMA Shared",lw=2,c='green',alpha=alphaFig)
                # ax.plot(df0["time"]*1000,df0["head"],label="RDMA Headroom",lw=2,c='blue',alpha=alphaFig)
                # ax.plot(df0["time"]*1000,df0["ing"],label="Shared Buffer",lw=2,c='blue',ls='--',alpha=alphaFig)
            else:
                ax.plot(df0["time"]*1000,df0["eglossy"],lw=2,c='red',alpha=alphaFig)
                ax.plot(df0["time"]*1000,df0["eglossless"]-df0["head"],lw=2,c='green',alpha=alphaFig)
                # ax.plot(df0["time"]*1000,df0["head"],lw=2,c='blue',alpha=alphaFig)
                # ax.plot(df0["time"]*1000,df0["ing"],lw=2,c='blue',ls='--',alpha=alphaFig)
            # ax.plot(df0["time"]*1000,df0["head"],label="headroom",lw=2)
            alphaFig+=0.2
        ax.legend()
        # ax.set_yscale('log')
        ax.set_ylim([10000,10**7])
        ax.set_xlim([0,30])
        ax.set_yticks([0,2*10**6,4*10**6,6*10**6,8*10**6,10*10**6])
        ax.set_yticklabels(["0","2","4","6","8","10"])
        ax.set_ylabel("Buffer Occupancy (MB)")
        ax.set_xlabel("Time (ms)")
        ax.xaxis.grid(True,ls='--')
        ax.yaxis.grid(True,ls='--')
        ax.set_title(alg+'-'+model)
        fig.tight_layout()
        # fig.savefig(plotsdir+'losslesssecond.pdf')
