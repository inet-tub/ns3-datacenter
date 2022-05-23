#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Mon Mar  1 06:46:30 2021

@author: vamsi
"""
import os
import requests
import pandas as pd
import numpy as np
import math
import matplotlib.pyplot as plt
import pylab
from matplotlib.lines import Line2D

NS3="/home/vamsi/lakewood/src/phd/ns3-datacenter/simulator/ns-3.35/"
plots_dir="/home/vamsi/Powertcp-NSDI/"

# plots_dir="/home/vamsi/Powertcp-NSDI/"
plt.rcParams.update({'font.size': 18})



algs=list(["dcqcn", "powerInt", "hpcc", "powerDelay", "timely", "dctcp"])
algnames={}
algnames["dcqcn"]="DCQCN"
algnames["powerInt"]="PowerTCP"
algnames["hpcc"]="HPCC"
algnames["powerDelay"]=r'$\theta-PowerTCP$'
algnames["timely"]="TIMELY"
algnames["DCTCP"]="DCTCP"


#%%

######### BURST ###############

results=NS3+"examples/PowerTCP/results_burst/"
plt.rcParams.update({'font.size': 22})


figlegend = pylab.figure(figsize=(11.5,1.5))
lenged_elements=list()

# red green blue brownm grey
colorsBurst=list(["#1979a9","red", "#478fb5","tab:brown","tab:gray"])
labels=list(['Throughput','Qlen'])

for i in range(1,3):
    lenged_elements.append(Line2D([0],[0], color=colorsBurst[i-1],lw=6, label=labels[i-1]))

for alg in algs:
    
    df = pd.read_csv(results+'result-'+alg+'.burst',delimiter=' ',usecols=[5,9,11,13],names=["th","qlen","time","power"])


    fig,ax = plt.subplots(1,1)
    # fig.suptitle(alg)
    ax.xaxis.grid(True,ls='--')
    ax.yaxis.grid(True,ls='--')
    ax1=ax.twinx()
    ax.set_yticks([10e9,25e9,40e9,80e9,100e9])
    ax.set_yticklabels(["10","25","40","80","100"])
    ax.set_ylabel("Throughput (Gbps)")
    
    start=0.15
    xtics=[i*0.001+start for i in range(0,6)]
    ax.set_xticks(xtics)
    xticklabels=[str(i) for i in range(0,6)]
    ax.set_xticklabels(xticklabels)
    
    ax.set_xlabel("Time (ms)")
    # ax.set_xlim(0.299,0.302)
    ax.set_xlim(0.1495,0.154)
    # ax.set_xlim(0.0998,0.159)
    ax.plot(df["time"],df["th"],label="Throughput",c='#1979a9',lw=2)
    ax1.set_ylim(0,600)
    ax1.set_ylabel("Queue length (KB)")
    ax1.plot(df["time"],df["qlen"]/(1000),c='r',label="Qlen",lw=2)
    # ax1.set_ylim(0,2)
    # ax1.plot(df["time"],df["power"],c='g',label="NormPower",lw=2)
    # ax1.set_title(alg)
    
    # ax1.axhline(25*10**9*5*1e-6*10/(1000*8),c='k',ls='--')
    
    # ax.legend(loc=1)
    # ax1.legend(loc=3)
    # fig.legend(loc=2,ncol=2,framealpha=0,borderpad=-0.1)
    fig.tight_layout()    
    # fig.savefig(plots_dir+'burst/'+alg+'.pdf')
    
    # ax1.axvspan(0.3,0.300180,color='grey',alpha=0.5)
    # ax1.axvspan(0.3,0.301180,color='grey',alpha=0.2)
    # fig.savefig("/home/vamsi/hpccPERACK.pdf")
    # fig.savefig("/home/vamsi/wienupPERACK.pdf")
figlegend.tight_layout()
figlegend.legend(handles=lenged_elements,loc=9,ncol=2, framealpha=0,fontsize=48)
# figlegend.savefig(plots_dir+'/burst/burst-legend.pdf') 
#%%


######## FAIRNESS #############

algs=list(["dcqcn", "powerInt", "hpcc", "powerDelay", "timely", "dctcp"])
results=NS3+"examples/PowerTCP/results_fairness/"

plt.rcParams.update({'font.size': 24})

figlegend = pylab.figure(figsize=(20.5,1.5))
lenged_elements=list()

colorsFair=list(["#d65151","#7ab547", "#478fb5","tab:brown","tab:gray"])
labels=list(['flow-1','flow-2','flow-3','flow-4','flow-5'])

for i in range(1,5):
    lenged_elements.append(Line2D([0],[0], color=colorsFair[i-1],lw=6, label=labels[i-1]))


for alg in algs:
    
    # if (alg=="powerDelay"):
        # continue
    
    fig,ax = plt.subplots(1,1)
    ax.xaxis.grid(True,ls='--')
    ax.yaxis.grid(True,ls='--')
    
    ax.set_ylabel("Throughput (Gbps)")
    ax.set_xlabel("Time (s)")
    # fig.suptitle(alg)
    
    df1 = pd.read_csv(results+'result-'+alg+'.1',delimiter=' ',usecols=[5,7],names=["th","time"])
    df2 = pd.read_csv(results+'result-'+alg+'.2',delimiter=' ',usecols=[5,7],names=["th","time"])
    df3 = pd.read_csv(results+'result-'+alg+'.3',delimiter=' ',usecols=[5,7],names=["th","time"])
    df4 = pd.read_csv(results+'result-'+alg+'.4',delimiter=' ',usecols=[5,7],names=["th","time"])
    
    ax.set_xlim(0,0.7)
    
    # ax.plot(df1["time"][::100],df1["th"][::100]/1e9)
    # ax.plot(df2["time"][::100],df2["th"][::100]/1e9)
    # ax.plot(df3["time"][::100],df3["th"][::100]/1e9)
    # ax.plot(df4["time"][::100],df4["th"][::100]/1e9)
    
    ax.plot(df1["time"],df1["th"]/1e9,c=colorsFair[0])
    ax.plot(df2["time"],df2["th"]/1e9,c=colorsFair[1])
    ax.plot(df3["time"],df3["th"]/1e9,c=colorsFair[2])
    ax.plot(df4["time"],df4["th"]/1e9,c=colorsFair[3])
    
    fig.tight_layout()    
    # fig.savefig(plots_dir+'fairness/'+alg+'.pdf')

figlegend.tight_layout()
figlegend.legend(handles=lenged_elements,loc=9,ncol=5, framealpha=0,fontsize=48)
# figlegend.savefig(plots_dir+'/fairness/fair-legend.pdf')


#%%

########## WORKLOADS ##################

results=NS3+"examples/PowerTCP/results_workload/"

K=1000
M=K*K
# algs=list(["dcqcn", "powerInt", "hpcc", "powerDelay", "timely", "dctcp"])
algs=list(["dcqcn", "powerInt", "hpcc", "powerDelay", "timely"])
# algs=list(["powerInt", "hpcc", "powerDelay"])

algnames={}
algnames["dcqcn"]="DCQCN"
algnames["powerInt"]="PowerTCP-INT"
algnames["hpcc"]="HPCC"
algnames["powerDelay"]="PowerTCP-Delay"
algnames["timely"]="TIMELY"
algnames["DCTCP"]="DCTCP"


markers={}
markers["dcqcn"]="x"
markers["powerInt"]="s"
markers["hpcc"]="^"
markers["powerDelay"]="o"
markers["timely"]="*"
markers["DCTCP"]="."

colors={}
colors["dcqcn"]='brown'
colors["powerInt"]='red'
colors["hpcc"]='blue'
colors["powerDelay"]='green'
colors["timely"]='cyan'
colors["DCTCP"]='black'



flowStep = [ 0,5*K, 10*K, 20*K, 30*K, 50*K, 75*K, 100*K, 200*K, 400*K,600*K,800*K, 1*M, 5*M, 10*M,30*M  ]
flowSteps= [ 5*K, 10*K, 20*K, 30*K, 50*K, 75*K, 100*K, 200*K, 400*K,600*K,800*K, 1*M, 5*M, 10*M,30*M ]
fS=np.arange(len(flowSteps))
flowSteps= [ "5K", "", "20K","", "50K", "", "100K","", "400K","","800K","", "5M", "","30M" ]
loads=list(["0.2","0.4","0.6","0.8","0.9","0.95"])
REQ_RATE=list(["1","4", "8", "16"])
REQ_SIZE=list(["1000000", "2000000", "4000000", "6000000", "8000000"])


#%%

#######################

# FLOWSIZE VS FCT

#######################

req="0"
query="0"
algs=list(["dcqcn", "powerInt", "hpcc", "powerDelay", "timely"])
load="0.8"
plt.rcParams.update({'font.size': 18})

figlegend = pylab.figure(figsize=(32.5,1.5))
lenged_elements=list()

for alg in algs:
    lenged_elements.append(Line2D([0],[0], color=colors[alg],marker=markers[alg],lw=10,markersize=30, label=algnames[alg]))

fctsShort={}
for alg in algs:
    fctsShort[alg]={}

for load in ["0.2","0.6"]:
    fig,ax = plt.subplots(1,1)
    ax.set_label("Flow size")
    ax.set_ylabel("99.9-pct FCT slowdown")
    ax.set_xlabel("Flow size (bytes)")
    # fig.suptitle("load="+load+" query="+query)
    # ax.set_xscale('log')
    ax.set_yscale('log')
    ax.set_ylim(1,20)
    ax.set_xticks(fS)
    ax.set_xticklabels(flowSteps,rotation=30)
    ax.set_yticks([1,10])
    ax.set_yticklabels([1,10])
    
    # fig1,ax1 = plt.subplots(1,1)
    # ax1.set_label("Flow size")
    # ax1.set_ylabel("FCT 95-pct")
    # fig1.suptitle("load="+load)
    # ax1.set_xscale('log')
    # # ax1.set_yscale('log')
    
    # fig2,ax2 = plt.subplots(1,1)
    # ax2.set_label("Flow size")
    # ax2.set_ylabel("FCT median")
    # fig2.suptitle("load="+load)
    # ax2.set_xscale('log')
    # # ax2.set_yscale('log')
    
    for alg in algs:
        df = pd.read_csv(results+'result-'+alg+'-'+load+'-'+req+'-'+query+'.fct',delimiter=' ',usecols=[1,3,5],names=["fct","size","base"])
        lfct99=list()
        lfct95=list()
        lfct50=list()
        for i in range(1,len(flowStep)):
            df1=df.loc[ (df["size"]<flowStep[i]) & (df["size"] >= flowStep[i-1]) ]
            fcts=df1["fct"].to_list()
            basefcts=df1["base"].to_list()
            sd=list([fcts[i]/basefcts[i] for i in range(len(fcts))])
            sd.sort()
            try:
                fct99 = sd[int(len(sd)*0.999)]
            except:
                fct99 = 0
            try:
                fct95 = sd[int(len(sd)*0.95)]
            except:
                fct95 = 0
            try:
                fct50 = sd[int(len(sd)*0.5)]
            except:
                fct50 = 0
            # print(fct99,fct95,fct50,flowStep[i],load,alg)
            lfct99.append(fct99)
            lfct95.append(fct95)
            lfct50.append(fct50)
        ax.plot(fS,lfct99,label=algnames[alg],marker=markers[alg],lw=2,markersize=10,c=colors[alg])
        print(lfct99[0],alg,load)
        fctsShort[alg][load]=lfct99[9]


    fig.tight_layout()
    fig.savefig(plots_dir+'workload/fct-'+load+'.pdf')


figlegend.tight_layout()
figlegend.legend(handles=lenged_elements,loc=9,ncol=5, framealpha=0,fontsize=52)
figlegend.savefig(plots_dir+'/workload/fct-legend.pdf')

#%%
for load in ["0.2","0.6"]:
    for alg in algs:
        print ( 100*(fctsShort[alg][load]-fctsShort["powerInt"][load])/fctsShort[alg][load], alg, load)
print("\n")

for load in ["0.2","0.6"]:
    for alg in algs:
        print ( 100*(fctsShort[alg][load]-fctsShort["powerDelay"][load])/fctsShort[alg][load], alg, load)




#%%

figlegend = pylab.figure(figsize=(32.5,1.5))
lenged_elements=list()

for alg in algs:
    lenged_elements.append(Line2D([0],[0], color=colors[alg],marker=markers[alg],lw=10,markersize=30, label=algnames[alg]))

figlegend.tight_layout()
figlegend.legend(handles=lenged_elements,loc=9,ncol=5, framealpha=0,fontsize=52)
figlegend.savefig(plots_dir+'/workload/all-legend.pdf')

#%%


#######################

# LOAD VS FCT - SHORT FLOWS

#######################

plt.rcParams.update({'font.size': 20})

algs=list([ "powerInt", "hpcc", "powerDelay"])

req="0"
query="0"
loads=list(["0.2","0.4","0.6","0.8","0.9","0.95"])

loadInt=list([20,40,60,80,90,95])
fig,ax = plt.subplots(1,1)
ax.xaxis.grid(True,ls='--')
ax.yaxis.grid(True,ls='--')

maxS=5*1*1000
minS=5*1*1000

fctsall={}

for alg in algs:
    fctsall[alg]=list()
    fcts99=list()
    for load in loads:
        df = pd.read_csv(results+'result-'+alg+'-'+load+'-'+req+'-'+query+'.fct',delimiter=' ',usecols=[1,3,5],names=["fct","size","base"])
        # print(len(df["fct"]))
        df1 = df.loc[(df["size"]<maxS) & (df["size"]> maxS-minS)]
        df1.reset_index(drop=True, inplace=True)
        # df1=df
        short=list()
        for i in range(len(df1["fct"])):
            short.append(df1["fct"][i]/df1["base"][i])
        short.sort()
        fcts99.append(short[int(len(short)*0.999)])
        
    ax.plot(loadInt,fcts99,label=algnames[alg],marker=markers[alg],lw=2,markersize=10,c=colors[alg])
    # ax.plot(loadInt,[i*1e6 for i in fcts99],label=alg)
    # ax.set_yscale('log')
    # ax.set_ylim(1,1.8)
    fctsall[alg]=fcts99

ax.set_xlabel('load (%)')
ax.set_ylabel('99.9-pct FCT slowdown')
# fig.savefig(plots_dir+'fct'+'.pdf')
# ax.legend(framealpha=0)
fig.tight_layout()
fig.savefig(plots_dir+'workload/fct-shortLoad'+'.pdf')

#%%

for alg in algs:
    for load in range(len(loads)):
        print( 100*(fctsall[alg][load]-fctsall["powerInt"][load])/fctsall[alg][load], alg, loads[load] )

#%%


#######################

# LOAD VS FCT - LONG FLOWS

#######################

algs=list([ "powerInt", "hpcc", "powerDelay"])

req="0"
query="0"
loads=list(["0.2","0.4","0.6","0.8","0.9","0.95"])

loadInt=list([20,40,60,80,90,95])
fig,ax = plt.subplots(1,1)
ax.xaxis.grid(True,ls='--')
ax.yaxis.grid(True,ls='--')

maxS=10*M
minS=1*M

fctsall={}

for alg in algs:
    fcts99=list()
    fctsall[alg]=list()
    for load in loads:
        df = pd.read_csv(results+'result-'+alg+'-'+load+'-'+req+'-'+query+'.fct',delimiter=' ',usecols=[1,3,5],names=["fct","size","base"])
        # print(len(df["fct"]))
        df1 = df.loc[(df["size"]<maxS) & (df["size"]> maxS-minS)]
        df1.reset_index(drop=True, inplace=True)
        # df1=df
        short=list()
        for i in range(len(df1["fct"])):
            short.append(df1["fct"][i]/df1["base"][i])
        short.sort()
        fcts99.append(short[int(len(short)*0.999)])
        
    ax.plot(loadInt,fcts99,label=algnames[alg],marker=markers[alg],lw=2,markersize=10,c=colors[alg])
    # ax.plot(loadInt,[i*1e6 for i in fcts99],label=alg)
    # ax.set_yscale('log')
    # ax.set_ylim(1,1.8)
    fctsall[alg]=fcts99

ax.set_xlabel('load (%)')
ax.set_ylabel('99.9-pct FCT slowdown')
# fig.savefig(plots_dir+'fct'+'.pdf')
# ax.legend(framealpha=0)
fig.tight_layout()
fig.savefig(plots_dir+'workload/fct-longLoad'+'.pdf')


#%%

for alg in algs:
    for load in range(len(loads)):
        print( 100*(fctsall[alg][load]-fctsall["powerInt"][load])/fctsall[alg][load], alg, loads[load] )


#%%


#######################

# BURSTRATE VS FCT - SHORT FLOWS

#######################

algs=list([ "powerInt", "hpcc", "powerDelay"])

req="0"
query="2000000"
loads=list(["0.2","0.4","0.6","0.8","0.9","0.95"])

loadInt=list([20,40,60,80,90,95])
fig,ax = plt.subplots(1,1)
ax.xaxis.grid(True,ls='--')
ax.yaxis.grid(True,ls='--')

# ax.set_yscale('log')

maxS=5*1*1000
minS=5*1*1000

fctsall={}

load="0.8"

for alg in algs:
    fcts99=list()
    fctsall[alg]=list()
    for req in REQ_RATE:
        df = pd.read_csv(results+'result-'+alg+'-'+load+'-'+req+'-'+query+'.fct',delimiter=' ',usecols=[1,3,5],names=["fct","size","base"])
        # print(len(df["fct"]))
        df1 = df.loc[(df["size"]<maxS) & (df["size"]> maxS-minS)]
        df1.reset_index(drop=True, inplace=True)
        # df1=df
        short=list()
        for i in range(len(df1["fct"])):
            short.append(df1["fct"][i]/df1["base"][i])
        short.sort()
        fcts99.append(short[int(len(short)*0.999)])
        
    ax.plot(REQ_RATE,fcts99,label=algnames[alg],marker=markers[alg],lw=2,markersize=10,c=colors[alg])
    # ax.plot(loadInt,[i*1e6 for i in fcts99],label=alg)
    # ax.set_yscale('log')
    # ax.set_ylim(1,1.8)
    fctsall[alg]=fcts99

ax.set_xlabel('Request rate')
ax.set_ylabel('99.9-pct FCT slowdown')
# fig.savefig(plots_dir+'fct'+'.pdf')
# ax.legend(framealpha=0)
fig.tight_layout()
fig.savefig(plots_dir+'workload/fct-shortRate'+'.pdf')
#%%

for alg in algs:
    for req in range(len(REQ_RATE)):
        print( 100*(fctsall[alg][req]-fctsall["powerInt"][req])/fctsall[alg][req], alg, REQ_RATE[req] )

#%%

#######################

# BURSTRATE VS FCT - LONG FLOWS

#######################

algs=list([ "powerInt", "hpcc", "powerDelay"])

req="0"
query="2000000"
loads=list(["0.2","0.4","0.6","0.8","0.9","0.95"])

loadInt=list([20,40,60,80,90,95])
fig,ax = plt.subplots(1,1)
ax.xaxis.grid(True,ls='--')
ax.yaxis.grid(True,ls='--')

maxS=10*M
minS=1*M

fctsall={}

load="0.8"

for alg in algs:
    fcts99=list()
    fctsall[alg]=list()
    for req in REQ_RATE:
        df = pd.read_csv(results+'result-'+alg+'-'+load+'-'+req+'-'+query+'.fct',delimiter=' ',usecols=[1,3,5],names=["fct","size","base"])
        # print(len(df["fct"]))
        df1 = df.loc[(df["size"]<maxS) & (df["size"]> maxS-minS)]
        df1.reset_index(drop=True, inplace=True)
        # df1=df
        short=list()
        for i in range(len(df1["fct"])):
            short.append(df1["fct"][i]/df1["base"][i])
        short.sort()
        fcts99.append(short[int(len(short)*0.999)])
        
    ax.plot(REQ_RATE,fcts99,label=algnames[alg],marker=markers[alg],lw=2,markersize=10,c=colors[alg])
    # ax.plot(loadInt,[i*1e6 for i in fcts99],label=alg)
    # ax.set_yscale('log')
    # ax.set_ylim(1,1.8)
    fctsall[alg]=fcts99

ax.set_xlabel('Request rate')
ax.set_ylabel('99.9-pct FCT slowdown')
# fig.savefig(plots_dir+'fct'+'.pdf')
# ax.legend(framealpha=0)
fig.tight_layout()
fig.savefig(plots_dir+'workload/fct-longRate'+'.pdf')
#%%

for alg in algs:
    for req in range(len(REQ_RATE)):
        print( 100*(fctsall[alg][req]-fctsall["powerInt"][req])/fctsall[alg][req], alg, REQ_RATE[req] )

#%%

#######################

# BURSTSIZE VS FCT - SHORT FLOWS

#######################

algs=list([ "powerInt", "hpcc", "powerDelay"])

req="4"
query="0"
loads=list(["0.2","0.4","0.6","0.8","0.9","0.95"])

loadInt=list([20,40,60,80,90,95])
fig,ax = plt.subplots(1,1)
ax.xaxis.grid(True,ls='--')
ax.yaxis.grid(True,ls='--')

ax.set_xticks(np.arange(len(REQ_SIZE)))
ax.set_xticklabels([1,2,4,6,8])

# ax.set_yscale('log')

maxS=5*1*1000
minS=5*1*1000

fctsall={}

load="0.8"

for alg in algs:
    fcts99=list()
    fctsall[alg]=list()
    for query in REQ_SIZE:
        df = pd.read_csv(results+'result-'+alg+'-'+load+'-'+req+'-'+query+'.fct',delimiter=' ',usecols=[1,3,5],names=["fct","size","base"])
        # print(len(df["fct"]))
        df1 = df.loc[(df["size"]<maxS) & (df["size"]> maxS-minS)]
        df1.reset_index(drop=True, inplace=True)
        # df1=df
        short=list()
        for i in range(len(df1["fct"])):
            short.append(df1["fct"][i]/df1["base"][i])
        short.sort()
        fcts99.append(short[int(len(short)*0.999)])
        
    ax.plot(np.arange(len(REQ_SIZE)),fcts99,label=algnames[alg],marker=markers[alg],lw=2,markersize=10,c=colors[alg])
    # ax.plot(loadInt,[i*1e6 for i in fcts99],label=alg)
    # ax.set_yscale('log')
    # ax.set_ylim(1,1.8)
    fctsall[alg]=fcts99

ax.set_xlabel('Request size (MB)')
ax.set_ylabel('99.9-pct FCT slowdown')
# fig.savefig(plots_dir+'fct'+'.pdf')
# ax.legend(framealpha=0)
fig.tight_layout()
fig.savefig(plots_dir+'workload/fct-shortBurst4'+'.pdf')
#%%

for alg in algs:
    for req in range(len(REQ_SIZE)):
        print( 100*(fctsall[alg][req]-fctsall["powerDelay"][req])/fctsall[alg][req], alg, REQ_SIZE[req] )

#%%

#######################

# BURSTSIZE VS FCT - LONG FLOWS

#######################

algs=list([ "powerInt", "hpcc", "powerDelay"])

req="1"
query="0"
loads=list(["0.2","0.4","0.6","0.8","0.9","0.95"])

loadInt=list([20,40,60,80,90,95])
fig,ax = plt.subplots(1,1)
ax.xaxis.grid(True,ls='--')
ax.yaxis.grid(True,ls='--')
ax.set_xticks(np.arange(len(REQ_SIZE)))
ax.set_xticklabels([1,2,4,6,8])

maxS=10*M
minS=1*M

fctsall={}

load="0.8"

for alg in algs:
    fcts99=list()
    fctsall[alg]=list()
    for query in REQ_SIZE:
        df = pd.read_csv(results+'result-'+alg+'-'+load+'-'+req+'-'+query+'.fct',delimiter=' ',usecols=[1,3,5],names=["fct","size","base"])
        # print(len(df["fct"]))
        df1 = df.loc[(df["size"]<maxS) & (df["size"]> maxS-minS)]
        df1.reset_index(drop=True, inplace=True)
        # df1=df
        short=list()
        for i in range(len(df1["fct"])):
            short.append(df1["fct"][i]/df1["base"][i])
        short.sort()
        fcts99.append(short[int(len(short)*0.999)])
        
    ax.plot(np.arange(len(REQ_SIZE)),fcts99,label=algnames[alg],marker=markers[alg],lw=2,markersize=10,c=colors[alg])
    # ax.plot(loadInt,[i*1e6 for i in fcts99],label=alg)
    # ax.set_yscale('log')
    # ax.set_ylim(1,1.8)
    fctsall[alg]=fcts99

ax.set_xlabel('Request size (MB)')
ax.set_ylabel('99.9-pct FCT slowdown')
# fig.savefig(plots_dir+'fct'+'.pdf')
# ax.legend(framealpha=0)
fig.tight_layout()
fig.savefig(plots_dir+'workload/fct-longBurst4'+'.pdf')
#%%

for alg in algs:
    for req in range(len(REQ_SIZE)):
        print( 100*(fctsall[alg][req]-fctsall["powerDelay"][req])/fctsall[alg][req], alg, REQ_SIZE[req] )


#%%

#######################

# CDF BUFFER 0.8 LOAD

#######################


# algs=list([ "powerInt", "hpcc", "dcqcn","timely"])
# algs=list(["dcqcn", "powerInt", "hpcc", "powerDelay", "timely"])
algs=list(["powerInt", "hpcc", "powerDelay"])
alg="powerInt"
load="0.9"
query="2000000"
req="8"

query="0"
req="0"
plt.rcParams.update({'font.size': 20})

fig,ax=plt.subplots(1,1)
ax.set_xticks([0,50*K,100*K,150*K,200*K])
ax.set_xticklabels(["0","50","100","150","200"])
ax.set_ylabel("CDF")
ax.set_xlabel("Buffer occupancy (KB)")

for alg in algs:
    df = pd.read_csv(results+'result-'+alg+'-'+load+'-'+req+'-'+query+'.buf',delimiter=' ',usecols=[3],names=["qlen"])
    
    qlen=df["qlen"].tolist()
    sortQlen= np.sort(qlen)
    p = 1. * np.arange(len(sortQlen))/(len(sortQlen) - 1)
    
    
    ax.plot(sortQlen,p,c=colors[alg],lw=2)
    ax.plot(sortQlen[len(sortQlen)-1],1,marker=markers[alg],markersize=10,c=colors[alg],label=algnames[alg])

# ax.set_xscale('log')
# ax.legend(framealpha=0)
fig.tight_layout()
fig.savefig(plots_dir+'workload/buf-load'+'.pdf')
#%%


#######################

# CDF BUFFER 2MB BURST, 16 Rate

#######################

# algs=list([ "powerInt", "hpcc", "dcqcn","timely"])
# algs=list(["dcqcn", "powerInt", "hpcc", "powerDelay", "timely"])
algs=list(["powerInt", "hpcc", "powerDelay"])
alg="powerInt"
load="0.8"
query="2000000"
req="16"

plt.rcParams.update({'font.size': 20})

fig,ax=plt.subplots(1,1)
ax.set_xticks([0,0.5*M,1*M,1.5*M,2*M,2.5*M])
ax.set_xticklabels(["0","0.5","1","1.5","2","2.5"])
ax.set_ylabel("CDF")
ax.set_xlim(-50*K,1.5*M)
ax.set_xlabel("Buffer occupancy (MB)")

cdfs=pd.DataFrame()

for alg in algs:
    df = pd.read_csv(results+'result-'+alg+'-'+load+'-'+req+'-'+query+'.buf',delimiter=' ',usecols=[3],names=["qlen"])
    
    qlen=df["qlen"].tolist()
    sortQlen= np.sort(qlen)
    p = 1. * np.arange(len(sortQlen))/(len(sortQlen) - 1)
    
    
    ax.plot(sortQlen,p,c=colors[alg],lw=2)
    ax.plot(sortQlen[len(sortQlen)-1],1,marker=markers[alg],markersize=10,c=colors[alg],label=algnames[alg])
    cdfs[alg]=sortQlen
    cdfs["cdf"]=p
# ax.legend(framealpha=0)
fig.tight_layout()
fig.savefig(plots_dir+'workload/buf-burst-16'+'.pdf')




#%%

######## Utilization #############

algs=list(["dcqcn", "powerInt", "hpcc", "powerDelay", "timely", "dctcp"])
results=NS3+"examples/PowerTCP/results_fairness/"

plt.rcParams.update({'font.size': 18})



alg="powerInt"

df3 = pd.read_csv(results+'result-'+alg+'.3',delimiter=' ',usecols=[5,7],names=["th","time"])
df4 = pd.read_csv(results+'result-'+alg+'.4',delimiter=' ',usecols=[5,7],names=["th","time"])

fig,ax=plt.subplots(1,1)
ax.plot(df3["time"],df3["th"]/1e9,lw=4,label="PowerTCP flow-1")
ax.plot(df4["time"],df4["th"]/1e9,lw=4,label="PowerTCP flow-2")

ax.xaxis.grid(True,ls='--')
ax.yaxis.grid(True,ls='--')

ax.set_xlabel("RTT timesteps")
ax.set_ylabel("Throughput (Gbps)")

alg="hpcc"

df3 = pd.read_csv(results+'result-'+alg+'.3',delimiter=' ',usecols=[5,7],names=["th","time"])
df4 = pd.read_csv(results+'result-'+alg+'.4',delimiter=' ',usecols=[5,7],names=["th","time"])

ax.plot(df3["time"],df3["th"]/1e9,ls='--',lw=4,label="HPCC flow-1",c='r')
ax.plot(df4["time"],df4["th"]/1e9,ls='--',lw=4,label="HPCC flow-2")


ax.set_xlim(0.599935,0.600200)

rtt=8800*1e-9
ax.set_xticks([rtt*i+0.599935 for i in range (0,34)])

labels=list()
for i in range (0,34):
    if i%5==0:
        labels.append(i)
    else:
        labels.append("")
ax.set_xticklabels(labels)

ax.axvline(rtt*12+0.599935,c='k',ls='--',alpha=0.5)
ax.axvline(rtt*15+0.599935,c='k',ls='--',alpha=0.5)
ax.axvline(rtt*24+0.599935,c='k',ls='--',alpha=0.5)

ax.set_ylim(0,25.2)

# fig.legend(ncol=2,framealpha=0,borderpad=0)
fig.tight_layout()

fig.savefig(plots_dir+'utilization.pdf')




#%%

######## RDCN TIMESERIES

direc='/home/vamsi/lakewood/TcpWien/ns-allinone-3.32/ns-3.32/examples/tcpwien/results_reconf/'

plt.rcParams.update({'font.size': 24})

tcps=list(["15", "18"])
fig,(ax,ax2,ax3) = plt.subplots(3,1,figsize=(20,8),sharex=True)
ax.xaxis.grid(True,ls='--')
ax.yaxis.grid(True,ls='--')
ax1=ax.twinx()

ax2.xaxis.grid(True,ls='--')
ax2.yaxis.grid(True,ls='--')
ax21=ax2.twinx()

ax3.xaxis.grid(True,ls='--')
ax3.yaxis.grid(True,ls='--')
ax31=ax3.twinx()


start=0.0230
end=0.027500
ax.set_yticks([0,25e9,80e9,100e9])
ax.set_yticklabels(["0","25","80","100"])
# ax.set_ylabel("Throughput (Gbps)")
ax.set_ylim(0,100e9+10e9)

ax2.set_yticks([0,25e9,80e9,100e9])
ax2.set_yticklabels(["0","25","80","100"])
ax2.set_ylabel("Throughput (Gbps)")
ax2.set_ylim(0,100e9+10e9)

ax3.set_yticks([0,25e9,80e9,100e9])
ax3.set_yticklabels(["0","25","80","100"])
# ax3.set_ylabel("Throughput (Gbps)")
ax3.set_ylim(0,100e9+10e9)

rtt=24*1e-6
xtics=[i*rtt+start for i in range(0,1000,10)]
ax.set_xticks(xtics)
xticklabels=[str(i) for i in range(0,1000,10)]
ax.set_xticklabels(xticklabels,rotation=45)

ax3.set_xlabel("Time (Base-RTTs)")

ax.set_xlim(start,end)
ax1.set_ylim(0,500)

# ax1.set_ylabel("Queue length (KB)")


ax21.set_ylim(0,500)

ax21.set_ylabel("Queue length (KB)")

ax31.set_ylim(0,500)

# ax31.set_ylabel("Queue length (KB)")


# for tcp in tcps:
dfW = pd.read_csv(direc+'result-'+'15'+'-fine.timeseries',delimiter=' ',usecols=[5,7,9,11],names=["th","qlen","time","tailLatency"])

ax.plot(dfW["time"][::1],dfW["th"][::1],label='Throughput PowerTCP',c='#1979a9',lw=4)
ax1.plot(dfW["time"],dfW["qlen"]/1000,c='r',ls='-',label='Qlen PowerTCP',lw=4)


dfW = pd.read_csv(direc+'result-'+'18'+'-fine.timeseries',delimiter=' ',usecols=[5,7,9,11],names=["th","qlen","time","tailLatency"])

ax2.plot(dfW["time"][::1],dfW["th"][::1],label='Throughput reTCP',c='#1979a9',lw=4,ls='-')
ax21.plot(dfW["time"],dfW["qlen"]/1000,c='r',label='Qlen reTCP',lw=4,ls='-')


dfW = pd.read_csv(direc+'result-'+'16'+'-fine.timeseries',delimiter=' ',usecols=[5,7,9,11],names=["th","qlen","time","tailLatency"])

ax3.plot(dfW["time"][::1],dfW["th"][::1],label='Throughput reTCP',c='#1979a9',lw=4,ls='-')
ax31.plot(dfW["time"],dfW["qlen"]/1000,c='r',label='Qlen reTCP',lw=6,ls='-')

ax.axvspan(120*rtt+start,130*rtt+start,color='grey',alpha=0.3)
ax2.axvspan(120*rtt+start,130*rtt+start,color='grey',alpha=0.3)
# fig.legend(loc=2,ncol=2)


labels=list(['Throughput','Qlen'])
colorsrdcn=list(["#1979a9","r"])
legend_elements=list()
for i in range(0,2):
    legend_elements.append(Line2D([0],[0], color=colorsrdcn[i],lw=6, label=labels[i]))

fig.legend(handles=legend_elements,loc=9,ncol=2,fontsize=28,framealpha=0)
# fig.tight_layout()
fig.savefig(plots_dir+'rdcn/timeseries.pdf')

#%%


plt.rcParams.update({'font.size': 18})
tcps=list(["15","16","18","19"])
hatches={}
hatches["15"]='/'
hatches["16"]='.'
hatches["18"]='--'
hatches["19"]='-'


# colors={}
# colors["17"]='b'
# colors["14"]='c'
# colors["15"]='g'
# colors["16"]='r'
# colors["18"]='grey'
# colors["19"]='black'
# colors["20"]='darkgrey'

colors={}
colors["17"]='b'
colors["14"]='c'
colors["15"]='#478fb5'
colors["16"]='r'
colors["18"]='grey'
colors["19"]='#478f45'
colors["20"]='darkgrey'


# colorsBurst=list(["#1979a9","red", "#478fb5","tab:brown","tab:gray"])


markers={}
markers["17"]='x'
markers["14"]='o'
markers["15"]='^'
markers["16"]='s'
markers["18"]='+'
markers["19"]='*'
markers["20"]='1'

tcpname={}
tcpname["0"]="TCP"
tcpname["14"]="DCTCP"
tcpname["15"]="PowerTCP-INT"
tcpname["16"]="HPCC"
tcpname["17"]="CUBIC"
tcpname["18"]="reTCP-1800us"
tcpname["19"]="reTCP-600us"
tcpname["20"]="reTCP-180us"

fig,ax = plt.subplots(1,1)
# ax.set_ylim(25,100)
ax.set_ylabel("Circuit utilization (%)")
ax.set_xlabel("Packet Bandwidth (Gbps)")


fig1,ax1 = plt.subplots(1,1)
ax1.set_ylabel("Average queuing latency (us)")
ax1.set_xlabel("Packet Bandwidth (Gbps)")


fig2,ax2 = plt.subplots(1,1,figsize=(4,6))
ax2.set_ylabel("Tail queuing latency (us)")
ax2.set_xlabel("Packet Bandwidth (Gbps)")

# ax.xaxis.grid(True,ls='--')
# ax.yaxis.grid(True,ls='--')

# ax1.xaxis.grid(True,ls='--')
# ax1.yaxis.grid(True,ls='--')

# ax2.xaxis.grid(True,ls='--')
# ax2.yaxis.grid(True,ls='--')


ax.set_xticks([0,1,2])
ax.set_xticklabels([10,25,50])

ax.set_yticks([0,20,40,60,80,100])

ax1.set_xticks([0,1,2])
ax1.set_xticklabels([10,25,50])
ax1.set_ylim(0,17)

ax2.set_xticks([0,1,2])
ax2.set_xticklabels([10,25,50])
ax2.set_ylim(0,70)

w=-0.4
for tcp in np.flip(tcps):
    df = pd.read_csv(direc+'result-'+tcp+'.packets',delimiter=' ',usecols=[1,2,3,4,5,6],names=["packet","circuit","days","th","avl","taill"])
    ax.bar(np.arange(len(df["packet"]))-w,[ df["th"][i]*100/(1e18*df["circuit"][i]) for i in range(0,len(df["circuit"]))],label=tcpname[tcp],color=colors[tcp],width=0.2,alpha=0.99,hatch=hatches[tcp])
    ax1.bar(np.arange(len(df["packet"]))-w,df["avl"]/1e3+1,label=tcpname[tcp],color=colors[tcp],width=0.2,alpha=0.99,hatch=hatches[tcp])
    ax2.bar(np.arange(len(df["packet"]))-w,df["taill"]/1e3+1,label=tcpname[tcp],color=colors[tcp],width=0.2,alpha=0.99,hatch=hatches[tcp])   
    w+=0.2

ax.set_xlim(0.6,2.6)
ax1.set_xlim(0.6,2.6)
ax2.set_xlim(0.6,2.6)



fig.tight_layout()
fig1.tight_layout()
fig2.tight_layout()

fig.legend(loc=9,ncol=2, framealpha=0,borderpad=-2)
fig1.legend(loc=9,ncol=2, framealpha=0,borderpad=-2)
fig2.legend(loc=9,ncol=2, framealpha=0,borderpad=-2)

fig.savefig(plots_dir+'rdcn/circuitUtil.pdf')
fig1.savefig(plots_dir+'rdcn/avLatency.pdf')
fig2.savefig(plots_dir+'rdcn/tailLatency.pdf')