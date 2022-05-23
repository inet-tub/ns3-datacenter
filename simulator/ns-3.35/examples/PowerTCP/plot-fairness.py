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
    fig.savefig(plots_dir+'fairness/'+alg+'.pdf')
    fig.savefig(plots_dir+'fairness/'+alg+'.png')

figlegend.tight_layout()
figlegend.legend(handles=lenged_elements,loc=9,ncol=5, framealpha=0,fontsize=48)
# figlegend.savefig(plots_dir+'/fairness/fair-legend.pdf')
