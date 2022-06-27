#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Sat Jan 29 16:01:28 2022

@author: vamsi
"""
import os
import sys
import numpy as np
import pandas as pd
# import matplotlib
# matplotlib.use('Agg')
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import pylab
import matplotlib

plt.rcParams.update({'font.size': 22})

#%%
DT=101
FAB=102
CS=103
IB=104
ABM=110
ALGS=[DT,FAB,CS,IB,ABM]

ALGNAMES={}
ALGNAMES[DT]="DT"
ALGNAMES[FAB]="FAB"
ALGNAMES[CS]="CS"
ALGNAMES[IB]="IB (AFD + Elephant trap)"
ALGNAMES[ABM]="ABM"

RENO=0
CUBIC=1
DCTCP=2
HPCC=3
POWERTCP=4
HOMA=5
TIMELY=6
THETAPOWERTCP=7


markers={}
markers[DT]='X'
markers[FAB]='^'
markers[CS]='d'
markers[IB]='o'
markers[ABM]='s'

colors={}
colors[DT]='red'
colors[FAB]='orange'
colors[CS]='black'
colors[IB]='blue'
colors[ABM]='green'

#%%

LINERATE=10*1e9
RTT=8*10*1e-6
BDP = (LINERATE*RTT/8)

TCP=CUBIC
LOADS=[0.2,0.4,0.6,0.8]
BURST_SIZES=0.3
BURST_FREQ=2


df = pd.read_csv('results-all.dat',delimiter=' ')
#%%
ALGS=[DT,FAB,CS,IB,ABM]
fig,ax = plt.subplots(1,1)
fig1,ax1 = plt.subplots(1,1)
fig2,ax2 = plt.subplots(1,1)
fig3,ax3 = plt.subplots(1,1)
fig4,ax4 = plt.subplots(1,1)

handles=list()
for alg in ALGS:
    dfalg = df[(df["burst"]==0.3)&(df["alg"]==alg)&(df["scenario"]=="single")&(df["tcp"]==TCP)]
    ax.plot(dfalg["load"]*100,dfalg["short99fct"],marker=markers[alg],color=colors[alg],lw=2,markersize=10,label=ALGNAMES[alg])
    ax1.plot(dfalg["load"]*100,dfalg["incast99fct"],marker=markers[alg],color=colors[alg],lw=2,markersize=10,label=ALGNAMES[alg])
    ax2.plot(dfalg["load"]*100,dfalg["longavgfct"],marker=markers[alg],color=colors[alg],lw=2,markersize=10,label=ALGNAMES[alg])
    ax3.plot(dfalg["load"]*100,dfalg["buf99"],marker=markers[alg],color=colors[alg],lw=2,markersize=10,label=ALGNAMES[alg])
    handle=ax4.plot(dfalg["load"]*100,dfalg["avgTh"],marker=markers[alg],color=colors[alg],lw=2,markersize=10,label=ALGNAMES[alg])    
    handles.append(handle)

figlegend = pylab.figure(figsize=(8.3,0.5))
figlegend.legend(*ax.get_legend_handles_labels(),ncol=len(ALGS),framealpha=0,fontsize=12,loc='upper center')
figlegend.savefig('legend.pdf')

ax.set_ylim(5,250)
ax.set_yscale('log')
ax.set_yticks([10,20,50,100,200])

ax1.set_yscale('log')
ax1.set_ylim(5,250) 
ax1.set_yticks([10,20,50,100,200])

ax3.set_ylim(0,105)
ax3.set_yticks([0,20,40,60,80,100])

ax4.set_ylim(0,105)
ax4.set_yticks([0,20,40,60,80,100])

ax.set_xlabel('Load (%)')
ax1.set_xlabel('Load (%)')
ax2.set_xlabel('Load (%)')
ax3.set_xlabel('Load (%)')
ax4.set_xlabel('Load (%)')

ax.set_ylabel('99-pct FCT slowdown')
ax1.set_ylabel('99-pct FCT slowdown')
ax2.set_ylabel('Median FCT slowdown')
ax3.set_ylabel('99-pct Buffer (%)')
ax4.set_ylabel('Avg. Throughput (%)')

ax.xaxis.grid(True,ls='--')
ax1.xaxis.grid(True,ls='--')
ax2.xaxis.grid(True,ls='--')
ax3.xaxis.grid(True,ls='--')
ax4.xaxis.grid(True,ls='--')

ax.yaxis.grid(True,ls='--')
ax1.yaxis.grid(True,ls='--')
ax2.yaxis.grid(True,ls='--')
ax3.yaxis.grid(True,ls='--')
ax4.yaxis.grid(True,ls='--')

fig.tight_layout()
fig.savefig("load-"+ALGNAMES[alg]+'-shortfct.pdf')
fig1.tight_layout()
fig1.savefig("load-"+ALGNAMES[alg]+'-incastfct.pdf')
fig2.tight_layout()
fig2.savefig("load-"+ALGNAMES[alg]+'-longfct.pdf')
fig3.tight_layout()
fig3.savefig("load-"+ALGNAMES[alg]+'-buf.pdf')
fig4.tight_layout()
fig4.savefig("load-"+ALGNAMES[alg]+'-th.pdf')


#%%

LINERATE=10*1e9
RTT=8*10*1e-6
BDP = (LINERATE*RTT/8)

TCP=CUBIC
LOADS=[0.2,0.4,0.6,0.8]
LOAD=0.4
# BURST_SIZES=0.3
BURST_FREQ=2
ALGS=[DT,FAB,CS,IB,ABM]

df = pd.read_csv('results-all.dat',delimiter=' ')

fig,ax = plt.subplots(1,1)
fig1,ax1 = plt.subplots(1,1)
fig2,ax2 = plt.subplots(1,1)
fig3,ax3 = plt.subplots(1,1)
fig4,ax4 = plt.subplots(1,1)


for alg in ALGS:
    dfalg = df[(df["load"]==0.4)&(df["alg"]==alg)&(df["burst"]<=0.75)&(df["scenario"]=="single")&(df["tcp"]==TCP)]
    dfalg=dfalg.sort_values(["burst"])
    ax.plot(dfalg["burst"]*100,dfalg["short99fct"],marker=markers[alg],color=colors[alg],lw=2,markersize=10,label=ALGNAMES[alg])
    ax1.plot(dfalg["burst"]*100,dfalg["incast99fct"],marker=markers[alg],color=colors[alg],lw=2,markersize=10,label=ALGNAMES[alg])
    ax2.plot(dfalg["burst"]*100,dfalg["longmedfct"],marker=markers[alg],color=colors[alg],lw=2,markersize=10,label=ALGNAMES[alg])
    ax3.plot(dfalg["burst"]*100,dfalg["buf99"],marker=markers[alg],color=colors[alg],lw=2,markersize=10,label=ALGNAMES[alg])
    ax4.plot(dfalg["burst"]*100,dfalg["avgTh"],marker=markers[alg],color=colors[alg],lw=2,markersize=10,label=ALGNAMES[alg])


ax.set_ylim(2,150)
ax.set_yscale('log')
ax.set_yticks([10,20,50,100,150])


ax1.set_ylim(2,180)
ax1.set_yscale('log')
ax1.set_yticks([10,20,50,100,150])


ax3.set_ylim(0,100)
ax3.set_yticks([0,20,40,60,80,100])

ax4.set_ylim(0,100)
ax4.set_yticks([0,20,40,60,80,100])

ax.set_xlabel('Request Size (% of buffer size)')
ax1.set_xlabel('Request Size (% of buffer size)')
ax2.set_xlabel('Request Size (% of buffer size)')
ax3.set_xlabel('Request Size (% of buffer size)')
ax4.set_xlabel('Request Size (% of buffer size)')

ax.set_ylabel('99-pct FCT slowdown')
ax1.set_ylabel('99-pct FCT slowdown')
ax2.set_ylabel('Median FCT slowdown')
ax3.set_ylabel('99-pct Buffer (%)')
ax4.set_ylabel('Avg. Throughput (%)')

ax.xaxis.grid(True,ls='--')
ax1.xaxis.grid(True,ls='--')
ax2.xaxis.grid(True,ls='--')
ax3.xaxis.grid(True,ls='--')
ax4.xaxis.grid(True,ls='--')
ax.yaxis.grid(True,ls='--')
ax1.yaxis.grid(True,ls='--')
ax2.yaxis.grid(True,ls='--')
ax3.yaxis.grid(True,ls='--')
ax4.yaxis.grid(True,ls='--')
fig.tight_layout()
fig.savefig("burst-"+ALGNAMES[alg]+'-shortfct.pdf')
fig1.tight_layout()
fig1.savefig("burst-"+ALGNAMES[alg]+'-incastfct.pdf')
fig2.tight_layout()
fig2.savefig("burst-"+ALGNAMES[alg]+'-longfct.pdf')
fig3.tight_layout()
fig3.savefig("burst-"+ALGNAMES[alg]+'-buf.pdf')
fig4.tight_layout()
fig4.savefig("burst-"+ALGNAMES[alg]+'-th.pdf')

#%%


LINERATE=10*1e9
RTT=8*10*1e-6
BDP = (LINERATE*RTT/8)

LOAD=0.4
# BURST_SIZES=0.3
BURST_FREQ=2

TCPS=[CUBIC,DCTCP,TIMELY,POWERTCP]
tcpnames={}
tcpnames[DCTCP]="dctcp"
tcpnames[TIMELY]="timely"
tcpnames[POWERTCP]="powertcp"
tcpnames[CUBIC]="cubic"

df = pd.read_csv('results-all.dat',delimiter=' ')
ALGS=[DT,ABM]

for tcp in TCPS:
    fig,ax = plt.subplots(1,1)
    fig1,ax1 = plt.subplots(1,1)
    fig2,ax2 = plt.subplots(1,1)
    fig3,ax3 = plt.subplots(1,1)
    fig4,ax4 = plt.subplots(1,1)
    
    
    for alg in ALGS:
        dfalg = df[(df["tcp"]==tcp)&(df["load"]==0.4)&(df["alg"]==alg)&(df["burst"]<=0.75)&(df["scenario"]=="single")&(df["burst"]!=0.3)]
        dfalg=dfalg.sort_values(["burst"])
        ax.plot(dfalg["burst"]*100,dfalg["short99fct"],marker=markers[alg],color=colors[alg],lw=2,markersize=10,label=ALGNAMES[alg])
        ax1.plot(dfalg["burst"]*100,dfalg["incast99fct"],marker=markers[alg],color=colors[alg],lw=2,markersize=10,label=ALGNAMES[alg])
        ax2.plot(dfalg["burst"]*100,dfalg["longmedfct"],marker=markers[alg],color=colors[alg],lw=2,markersize=10,label=ALGNAMES[alg])
        ax3.plot(dfalg["burst"]*100,dfalg["buf99"],marker=markers[alg],color=colors[alg],lw=2,markersize=10,label=ALGNAMES[alg])
        ax4.plot(dfalg["burst"]*100,dfalg["avgTh"],marker=markers[alg],color=colors[alg],lw=2,markersize=10,label=ALGNAMES[alg])
    
    ax.set_ylim(2,220)
    ax.set_yscale('log')
    ax.set_yticks([10,20,50,100,200])
    
    
    ax1.set_ylim(2,220)
    ax1.set_yscale('log')
    ax1.set_yticks([10,20,50,100,200])
    
    
    ax3.set_ylim(0,100)
    ax3.set_yticks([0,20,40,60,80,100])
    
    
    ax4.set_ylim(0,100)
    ax4.set_yticks([0,20,40,60,80,100])
    ax.set_xticks([10,25,50,75])
    ax1.set_xticks([10,25,50,75])
    ax3.set_xticks([10,25,50,75])
    ax4.set_xticks([10,25,50,75])
    
    ax.set_xlabel('Request Size (% of buffer size)')
    ax1.set_xlabel('Request Size (% of buffer size)')
    ax2.set_xlabel('Request Size (% of buffer size)')
    ax3.set_xlabel('Request Size (% of buffer size)')
    ax4.set_xlabel('Request Size (% of buffer size)')
    
    ax.set_ylabel('99-pct FCT slowdown')
    ax1.set_ylabel('99-pct FCT slowdown')
    ax2.set_ylabel('99-pct FCT slowdown')
    ax3.set_ylabel('99-pct Buffer (%)')
    ax4.set_ylabel('Avg. Throughput (%)')

    ax.xaxis.grid(True,ls='--')
    ax1.xaxis.grid(True,ls='--')
    ax2.xaxis.grid(True,ls='--')
    ax3.xaxis.grid(True,ls='--')
    ax4.xaxis.grid(True,ls='--')
    ax.yaxis.grid(True,ls='--')
    ax1.yaxis.grid(True,ls='--')
    ax2.yaxis.grid(True,ls='--')
    ax3.yaxis.grid(True,ls='--')
    ax4.yaxis.grid(True,ls='--')
    # ax.legend()
    # ax1.legend()
    fig.tight_layout()
    fig.savefig("tcp-"+tcpnames[tcp]+'-shortfct.pdf')
    fig1.tight_layout()
    fig1.savefig("tcp-"+tcpnames[tcp]+'-incastfct.pdf')
    fig2.tight_layout()
    fig2.savefig("tcp-"+tcpnames[tcp]+'-longfct.pdf')
    fig3.tight_layout()
    fig3.savefig("tcp-"+tcpnames[tcp]+'-buf.pdf')
    fig4.tight_layout()
    fig4.savefig("tcp-"+tcpnames[tcp]+'-th.pdf')
    
#%%

LINERATE=10*1e9
RTT=8*10*1e-6
BDP = (LINERATE*RTT/8)

LOAD=0.4
BURST_SIZES=0.3
BURST_FREQ=2

TCPS=[CUBIC,DCTCP,THETAPOWERTCP]
tcpnames={}
tcpnames[CUBIC]="cubic"
tcpnames[DCTCP]="dctcp"
tcpnames[THETAPOWERTCP]="thetapowertcp"

df = pd.read_csv('results-all.dat',delimiter=' ')

for tcp in TCPS:
    fig,ax = plt.subplots(1,1)
    fig1,ax1=plt.subplots(1,1)
    for alg in ALGS:
        dfalg = df[(df["tcp"]==tcp)&(df["alg"]==alg)&(df["burst"]==BURST_SIZES)&(df["scenario"]=="multi")]
        if tcp==THETAPOWERTCP:
            # ax.plot(dfalg["load"],dfalg["short999fct"],marker=markers[alg],color=colors[alg],lw=2,markersize=10,label=ALGNAMES[alg])
            ax.plot(dfalg["load"]*100,dfalg["incast99fct"],marker=markers[alg],color=colors[alg],lw=2,markersize=10,label=ALGNAMES[alg])
        else:
            ax.plot(dfalg["load"]*100,dfalg["short99fct"],marker=markers[alg],color=colors[alg],lw=2,markersize=10,label=ALGNAMES[alg])
        ax1.plot(dfalg["load"]*100,dfalg["buf99"],marker=markers[alg],color=colors[alg],lw=2,markersize=10,label=ALGNAMES[alg])
    ax.set_ylim(5,180)
    ax.set_yscale('log')
    ax.set_yticks([10,20,50,100,150])
    ax.xaxis.grid(True,ls='--')
    ax1.xaxis.grid(True,ls='--')
    ax.yaxis.grid(True,ls='--')
    ax1.yaxis.grid(True,ls='--')
    
    ax.set_xlabel('Cubic Load (%)')
    ax1.set_xlabel('Cubic Load (%)')
    
    ax.set_ylabel('99-pct FCT slowdown')
    fig.tight_layout()
    fig.savefig('multi-'+tcpnames[tcp]+'.pdf')

    ax1.set_ylim(0,105)
    ax1.set_yticks([0,20,40,60,80,100])
    ax1.set_ylabel("99-pct buffer (%)")
    fig1.tight_layout()
    fig1.savefig('multi-buf99.pdf')
    
#%%
LOAD=0.4
tcp=DCTCP
BURST_FREQ=2
BURST_SIZES=0.5
alg=ABM
df = pd.read_csv('results-all.dat',delimiter=' ')

intervals=["1", "10", "100", "1K","DT"]

dfalg = df[(df["scenario"]=="1")|(df["scenario"]=="10")|(df["scenario"]=="100")|(df["scenario"]=="1000")|(df["scenario"]=="dt")] #|(df["scenario"]=="dt")

fig,ax=plt.subplots(1,1)
ax.bar(np.arange(len(intervals)),list(dfalg["short999fct"]),color=['black', 'gray', 'darkgray', 'lightgray',"red"])
ax.set_xticks(np.arange(len(intervals)))
ax.set_xticklabels(intervals,rotation=40)
ax.set_ylim(12,27)
ax.set_ylabel("99.9-pct FCT\n slowdown")
ax.set_xlabel("Update interval (RTTs)")
# ax.set_yscale('log')
# ax.set_yticks([10,30,50,100,200])
# ax.xaxis.grid(True,ls='--')
ax.yaxis.grid(True,ls='--')
fig.tight_layout()
fig.savefig('intervals-shortfct1.pdf')

fig,ax=plt.subplots(1,1)
ax.bar(np.arange(len(intervals)),list(dfalg["incast999fct"]),color=['black', 'gray', 'darkgray', 'lightgray',"red"])
ax.set_xticks(np.arange(len(intervals)))
ax.set_xticklabels(intervals,rotation=40)
ax.set_ylim(7,250)
ax.set_ylabel("99.9-pct FCT\n slowdown")
ax.set_xlabel("Update interval (RTTs)")
# ax.xaxis.grid(True,ls='--')
# ax.set_yscale('log')
# ax.set_yticks([10,30,50,100,200])
ax.yaxis.grid(True,ls='--')
fig.tight_layout()
fig.savefig('intervals-incastfct1.pdf')

fig,ax=plt.subplots(1,1)
ax.bar(np.arange(len(intervals)),list(dfalg["longmedfct"]),color=['black', 'gray', 'darkgray', 'lightgray',"red"])
ax.set_xticks(np.arange(len(intervals)))
ax.set_xticklabels(intervals,rotation=40)
ax.set_ylim(1,10)
ax.set_ylabel("Median FCT slowdown")
ax.set_yticks([2,4,6,8,10])
ax.set_xlabel("Update interval (RTTs)")
# ax.xaxis.grid(True,ls='--')
ax.yaxis.grid(True,ls='--')
fig.tight_layout()
fig.savefig('intervals-longfct1.pdf')

fig,ax=plt.subplots(1,1)
ax.bar(np.arange(len(intervals)),list(dfalg["buf99"]),color='k')
ax.set_xticks(np.arange(len(intervals)))
ax.set_xticklabels(intervals,rotation=40)
# ax.xaxis.grid(True,ls='--')
ax.yaxis.grid(True,ls='--')
ax.set_ylim(1,100)


fig,ax=plt.subplots(1,1)
ax.bar(np.arange(len(intervals)),list(dfalg["avgTh"]),color='k')
ax.set_xticks(np.arange(len(intervals)))
ax.set_xticklabels(intervals,rotation=40)
# ax.xaxis.grid(True,ls='--')
ax.yaxis.grid(True,ls='--')
ax.set_ylim(1,100)


#%%


LOAD=0.4
tcp=DCTCP
BURST_FREQ=2
BURST_SIZES=0.5
alg=DT
df = pd.read_csv('results-all.dat',delimiter=' ')

intervals=["Trident2", "8KB", "7KB", "6KB", "Tomahawk", "Tofino"]


plt.rcParams.update({'font.size': 21})

colors={}
colors[DT]='darkgray'
colors[ABM]='k'
colors[IB]='lightgray'

algnames={}
algnames[DT]="dt"
algnames[ABM]="abm"
algnames[IB]="ib"

tcpnames={}
tcpnames[DCTCP]="dctcp"
tcpnames[POWERTCP]="powertcp"

hatches={}
hatches[DT]='/'
hatches[ABM]='.'
hatches[IB]='-'

for tcp in [DCTCP,POWERTCP]:
    wid=0.2
    shift=0
    fig,ax=plt.subplots(1,1)
    fig1,ax1=plt.subplots(1,1)
    for alg in [ABM,DT,IB]:
        dfalg = df[(df["tcp"]==tcp)&(df["alg"]==alg)&((df["scenario"]=="9.6")|(df["scenario"]=="8")|(df["scenario"]=="7")|(df["scenario"]=="6")|(df["scenario"]=="5.12")|(df["scenario"]=="3.44"))]
        ax.bar(np.arange(len(intervals))+shift,list(dfalg["short99fct"]),color=colors[alg],width=wid,label=ALGNAMES[alg],hatch=hatches[alg],alpha=0.99)
        ax1.bar(np.arange(len(intervals))+shift,list(dfalg["incast99fct"]),color=colors[alg],width=wid,label=ALGNAMES[alg],hatch=hatches[alg],alpha=0.99)
        shift+=wid
    
    ax.set_xticks(np.arange(len(intervals)))
    ax.set_xticklabels(intervals,rotation=40)
    ax.set_ylim(1,120)
    ax.set_ylabel("99-pct FCT \nslowdown")
    ax.set_xlabel("Buffer per port per Gbps")
    ax.set_yscale('log')
    ax.set_yticks([10,50,100,150])
    # ax.xaxis.grid(True,ls='--')
    ax.yaxis.grid(True,ls='--')
    fig.tight_layout()
    # fig.savefig('buffer-shortfct-'+tcpnames[tcp]+'-.pdf')
    
    
    ax1.set_xticks(np.arange(len(intervals)))
    ax1.set_xticklabels(intervals,rotation=40)
    ax1.set_ylim(1,120)
    ax1.set_ylabel("99-pct FCT \nslowdown")
    ax1.set_title("(Buffer per port per Gbps)",fontsize=20)
    # ax.xaxis.grid(True,ls='--')
    ax1.set_yscale('log')
    ax1.set_yticks([10,50,100,150])
    ax1.yaxis.grid(True,ls='--')
    fig1.tight_layout()
    fig1.savefig('buffer-incastfct-'+tcpnames[tcp]+'-.pdf')
figlegend = pylab.figure(figsize=(6.3,0.5))
figlegend.legend(*ax.get_legend_handles_labels(),ncol=3,framealpha=0,fontsize=12,loc='upper center')
# figlegend.savefig('legend-buffers.pdf')

# fig,ax=plt.subplots(1,1)
# ax.bar(np.arange(len(intervals)),list(dfalg["longmedfct"]),color=['black', 'gray', 'darkgray', 'lightgray', 'silver',"red"])
# ax.set_xticks(np.arange(len(intervals)))
# ax.set_xticklabels(intervals,rotation=40)
# ax.set_ylim(1,10)
# ax.set_ylabel("99.9-pct FCT slowdown")
# ax.set_yticks([2,4,6,8,10])
# # ax.xaxis.grid(True,ls='--')
# ax.yaxis.grid(True,ls='--')
# fig.tight_layout()
# # fig.savefig('intervals-longfct.pdf')

# fig,ax=plt.subplots(1,1)
# ax.bar(np.arange(len(intervals)),list(dfalg["buf99"]),color='k')
# ax.set_xticks(np.arange(len(intervals)))
# ax.set_xticklabels(intervals,rotation=40)
# # ax.xaxis.grid(True,ls='--')
# ax.yaxis.grid(True,ls='--')
# ax.set_ylim(1,100)


# fig,ax=plt.subplots(1,1)
# ax.bar(np.arange(len(intervals)),list(dfalg["avgTh"]),color='k')
# ax.set_xticks(np.arange(len(intervals)))
# ax.set_xticklabels(intervals,rotation=40)
# # ax.xaxis.grid(True,ls='--')
# ax.yaxis.grid(True,ls='--')
# ax.set_ylim(1,100)

#%%


LOAD=0.4
tcp=DCTCP
BURST_FREQ=2
BURST_SIZES=0.5
alg=ABM
df = pd.read_csv('results-all.dat',delimiter=' ')

lines={}
lines[CUBIC]='--'
lines[DCTCP]='-'

fig,ax = plt.subplots(1,1)
fig1,ax1 = plt.subplots(1,1)
fig2,ax2 = plt.subplots(1,1)
fig3,ax3 = plt.subplots(1,1)
fig4,ax4 = plt.subplots(1,1)

labels={}
labels[CUBIC]={}
labels[DCTCP]={}
labels[CUBIC][DT]="Cubic with DT"
labels[DCTCP][DT]="DCTCP with DT"
labels[CUBIC][ABM]="Cubic with ABM"
labels[DCTCP][ABM]="DCTCP with ABM"



for tcp in [CUBIC, DCTCP]:
    
    for alg in [DT, ABM]:
        dfalg = df[(df["scenario"]=="multiqueue")&(df["alg"]==alg)&(df["tcp"]==tcp)]
        ax.plot(dfalg["nprio"],dfalg["short99fct"],marker=markers[alg],color=colors[alg],lw=2,markersize=10,label=labels[tcp][alg],ls=lines[tcp])
        ax1.plot(dfalg["nprio"],dfalg["incast99fct"],marker=markers[alg],color=colors[alg],lw=2,markersize=10,label=labels[tcp][alg],ls=lines[tcp])
        ax2.plot(dfalg["nprio"],dfalg["longmedfct"],marker=markers[alg],color=colors[alg],lw=2,markersize=10,label=labels[tcp][alg],ls=lines[tcp])
        ax3.plot(dfalg["nprio"],dfalg["buf99"],marker=markers[alg],color=colors[alg],lw=2,markersize=10,label=labels[tcp][alg],ls=lines[tcp])
        ax4.plot(dfalg["nprio"],dfalg["avgTh"],marker=markers[alg],color=colors[alg],lw=2,markersize=10,label=labels[tcp][alg],ls=lines[tcp])
        
# ax.set_yscale('log')
ax.set_ylim(1,200)
ax.xaxis.grid(True,ls='--')
ax.yaxis.grid(True,ls='--')
# ax.set_yticks([1,10,100])
# ax.set_yticklabels([1,10,100])
ax.set_xticks([2,3,4,5,6,7,8])
# ax1.set_yscale('log')
ax1.set_ylim(1,300)
ax1.xaxis.grid(True,ls='--')
ax1.yaxis.grid(True,ls='--')
# ax1.set_yticks([1,10,100])
# ax1.set_yticklabels([1,10,100])
ax1.set_xticks([2,3,4,5,6,7,8])
ax1.set_ylabel("99-pct FCT slowdown")
ax1.set_xlabel("Number of queues per port")
# fig1.legend(loc='upper center',ncol=2,framealpha=0)
fig1.tight_layout()
fig1.savefig('fcts-nprior.pdf')

ax3.set_ylim(0,100)
ax3.xaxis.grid(True,ls='--')
ax3.yaxis.grid(True,ls='--')
ax3.set_xticks([2,3,4,5,6,7,8])
ax3.set_ylabel("99-pct buffer (%)")
ax3.set_xlabel("Number of queues per port")
# fig3.legend(loc='upper center',ncol=2,framealpha=0)
fig3.tight_layout()
fig3.savefig('buffer-nprior.pdf')
