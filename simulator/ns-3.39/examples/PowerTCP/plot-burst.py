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

# Change these paths to your local paths
NS3="/home/vamsi/ns3-datacenter/simulator/ns-3.39/"
# Create a directory for storing plots if it doesn't exist
# and specify the corresponding path
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
    ax.set_xlim(0.1495,0.154)
    ax.plot(df["time"],df["th"],label="Throughput",c='#1979a9',lw=2)
    ax1.set_ylim(0,2000)
    ax1.set_ylabel("Queue length (KB)")
    ax1.plot(df["time"],df["qlen"]/(1000),c='r',label="Qlen",lw=2)
    # ax.legend(loc=1)
    # ax1.legend(loc=3)
    # fig.legend(loc=2,ncol=2,framealpha=0,borderpad=-0.1)
    fig.tight_layout()    
    fig.savefig(plots_dir+'burst/'+alg+'.pdf')
    fig.savefig(plots_dir+'burst/'+alg+'.png')

    fig1,ax2 = plt.subplots(1,1)
    # fig.suptitle(alg)
    ax2.xaxis.grid(True,ls='--')
    ax2.yaxis.grid(True,ls='--')
    ax3=ax2.twinx()
    ax2.set_yticks([10e9,25e9,40e9,80e9,100e9])
    ax2.set_yticklabels(["10","25","40","80","100"])
    ax2.set_ylabel("Throughput (Gbps)")
    
    start=0.15
    xtics=[i*0.001+start for i in range(0,6)]
    ax2.set_xticks(xtics)
    xticklabels=[str(i) for i in range(0,6)]
    ax2.set_xticklabels(xticklabels)
    ax2.set_xlabel("Time (ms)")
    ax2.set_xlim(0.1495,0.154)
    ax2.plot(df["time"],df["th"],label="Throughput",c='#1979a9',lw=2)
    ax3.set_ylabel("Normalized Power")
    ax3.set_ylim(0,2)
    ax3.plot(df["time"],df["power"],c='g',label="NormPower",lw=2)
    fig1.tight_layout()
    fig1.savefig(plots_dir+'burst/'+alg+'-power.pdf')
    fig1.savefig(plots_dir+'burst/'+alg+'-power.png')


figlegend.tight_layout()
figlegend.legend(handles=lenged_elements,loc=9,ncol=2, framealpha=0,fontsize=48)
# figlegend.savefig(plots_dir+'/burst/burst-legend.pdf') 