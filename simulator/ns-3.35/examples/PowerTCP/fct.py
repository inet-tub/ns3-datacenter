import os
import requests
import pandas as pd
import numpy as np
import math
import matplotlib.pyplot as plt


NS3="/home/vamsi/src/phd/ns3-datacenter/simulator/ns-3.35/"
plots_dir="/home/vamsi/Powertcp-NSDI/"

results=NS3+"/home/vamsi/src/phd/ns3-datacenter/simulator/ns-3.35/examples/PowerTCP/results_workload/"
plt.rcParams.update({'font.size': 18})

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
for load in ["0.2","0.6"]:
    fig,ax = plt.subplots(1,1)
    ax.set_label("Flow size")
    ax.set_ylabel("99-pct FCT slowdown")
    ax.set_xlabel("Flow size")
    # fig.suptitle("load="+load+" query="+query)
    # ax.set_xscale('log')
    ax.set_yscale('log')
    ax.set_ylim(1,100)
    ax.set_xticks(fS)
    ax.set_xticklabels(flowSteps,rotation=45)
    ax.set_yticks([1,10,100])
    ax.set_yticklabels([1,10,100])
    
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
        ax.plot(fS,lfct99,label=alg,marker=markers[alg],lw=2,markersize=10,c=colors[alg])
        # ax1.plot(flowSteps,lfct95,label=alg)
        # ax2.plot(flowSteps,lfct50,label=alg)
    ax.legend(ncol=2,framealpha=0,loc='upper center',borderpad=-0.1)
    fig.tight_layout()
    fig.savefig(plots_dir+'fct-'+alg+'-'+load+'.pdf')
    # ax1.legend()
    # ax2.legend()

#%%


#######################

# LOAD VS FCT - SHORT FLOWS

#######################

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

for alg in algs:
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
        fcts99.append(short[int(len(short)*0.99)])
        
    ax.plot(loadInt,fcts99,label=alg,marker=markers[alg],lw=2,markersize=10,c=colors[alg])
    # ax.plot(loadInt,[i*1e6 for i in fcts99],label=alg)
    # ax.set_yscale('log')
    # ax.set_ylim(1,1.8)

ax.set_xlabel('load (%)')
ax.set_ylabel('99-pct FCT slowdown')
# fig.savefig(plots_dir+'fct'+'.pdf')
ax.legend()

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

for alg in algs:
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
        fcts99.append(short[int(len(short)*0.99)])
        
    ax.plot(loadInt,fcts99,label=alg,marker=markers[alg],lw=2,markersize=10,c=colors[alg])
    # ax.plot(loadInt,[i*1e6 for i in fcts99],label=alg)
    # ax.set_yscale('log')
    # ax.set_ylim(1,1.8)

ax.set_xlabel('load (%)')
ax.set_ylabel('99-pct FCT slowdown')
# fig.savefig(plots_dir+'fct'+'.pdf')
ax.legend()

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

load="0.8"

for alg in algs:
    fcts99=list()
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
        fcts99.append(short[int(len(short)*0.99)])
        
    ax.plot(REQ_RATE,fcts99,label=alg,marker=markers[alg],lw=2,markersize=10,c=colors[alg])
    # ax.plot(loadInt,[i*1e6 for i in fcts99],label=alg)
    # ax.set_yscale('log')
    # ax.set_ylim(1,1.8)

ax.set_xlabel('Request rate')
ax.set_ylabel('99-pct FCT slowdown')
# fig.savefig(plots_dir+'fct'+'.pdf')
ax.legend()

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

load="0.8"

for alg in algs:
    fcts99=list()
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
        fcts99.append(short[int(len(short)*0.99)])
        
    ax.plot(REQ_RATE,fcts99,label=alg,marker=markers[alg],lw=2,markersize=10,c=colors[alg])
    # ax.plot(loadInt,[i*1e6 for i in fcts99],label=alg)
    # ax.set_yscale('log')
    # ax.set_ylim(1,1.8)

ax.set_xlabel('Request rate')
ax.set_ylabel('99-pct FCT slowdown')
# fig.savefig(plots_dir+'fct'+'.pdf')
ax.legend()

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

load="0.8"

for alg in algs:
    fcts99=list()
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
        fcts99.append(short[int(len(short)*0.99)])
        
    ax.plot(np.arange(len(REQ_SIZE)),fcts99,label=alg,marker=markers[alg],lw=2,markersize=10,c=colors[alg])
    # ax.plot(loadInt,[i*1e6 for i in fcts99],label=alg)
    # ax.set_yscale('log')
    # ax.set_ylim(1,1.8)

ax.set_xlabel('Request size (MB)')
ax.set_ylabel('99-pct FCT slowdown')
# fig.savefig(plots_dir+'fct'+'.pdf')
ax.legend()

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

load="0.8"

for alg in algs:
    fcts99=list()
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
        fcts99.append(short[int(len(short)*0.99)])
        
    ax.plot(np.arange(len(REQ_SIZE)),fcts99,label=alg,marker=markers[alg],lw=2,markersize=10,c=colors[alg])
    # ax.plot(loadInt,[i*1e6 for i in fcts99],label=alg)
    # ax.set_yscale('log')
    # ax.set_ylim(1,1.8)

ax.set_xlabel('Request size (MB)')
ax.set_ylabel('99-pct FCT slowdown')
# fig.savefig(plots_dir+'fct'+'.pdf')
ax.legend()


#%%

# algs=list([ "powerInt", "hpcc", "dcqcn","timely"])
algs=list(["dcqcn", "powerInt", "hpcc", "powerDelay", "timely"])
alg="powerInt"
load="0.8"
query="2000000"
req="8"

query="0"
req="0"


fig,ax=plt.subplots(1,1)
ax.set_xticks([0,0.5*M,1*M,1.5*M,2*M])
ax.set_xticklabels(["0","0.5","1","1.5","2"])
ax.set_ylabel("CDF")
ax.set_xlabel("Buffer occupancy (MB)")

for alg in algs:
    df = pd.read_csv(results+'result-'+alg+'-'+load+'-'+req+'-'+query+'.buf',delimiter=' ',usecols=[3],names=["qlen"])
    
    qlen=df["qlen"].tolist()
    sortQlen= np.sort(qlen)
    p = 1. * np.arange(len(sortQlen))/(len(sortQlen) - 1)
    
    
    ax.plot(sortQlen,p,c=colors[alg])
    ax.plot(sortQlen[len(sortQlen)-1],1,marker=markers[alg],markersize=10,c=colors[alg],label=alg)

# ax.set_xscale('log')
ax.legend()

#%%


# algs=list([ "powerInt", "hpcc", "dcqcn","timely"])
algs=list(["dcqcn", "powerInt", "hpcc", "powerDelay", "timely"])

alg="powerInt"
load="0.8"
query="2000000"
req="16"



fig,ax=plt.subplots(1,1)
ax.set_xticks([0,0.5*M,1*M,1.5*M,2*M,2.5*M])
ax.set_xticklabels(["0","0.5","1","1.5","2","2.5"])
ax.set_ylabel("CDF")
ax.set_xlabel("Buffer occupancy (MB)")

for alg in algs:
    df = pd.read_csv(results+'result-'+alg+'-'+load+'-'+req+'-'+query+'.buf',delimiter=' ',usecols=[3],names=["qlen"])
    
    qlen=df["qlen"].tolist()
    sortQlen= np.sort(qlen)
    p = 1. * np.arange(len(sortQlen))/(len(sortQlen) - 1)
    
    
    ax.plot(sortQlen,p,c=colors[alg])
    ax.plot(sortQlen[len(sortQlen)-1],1,marker=markers[alg],markersize=10,c=colors[alg],label=alg)

ax.legend()