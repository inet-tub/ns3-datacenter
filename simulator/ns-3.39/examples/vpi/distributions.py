#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Sun Jun 23 05:33:06 2024

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
import pylab

#%%

def generate_random_excluding_i(S, i):
    valid_numbers = [x for x in range(S) if x != i]
    
    if not valid_numbers:
        raise ValueError("No valid numbers to choose from.")
    
    return random.choice(valid_numbers)


def generate_exponential_excluding_i(lambd,S, i):
    while True:
        rand_num = math.floor(random.expovariate(lambd))
        if rand_num != i and rand_num < S:
            # print(rand_num)
            return rand_num
        
def generate_normal_excluding_i(mean, stddev, S, i):
    while True:
        rand_num = round(random.gauss(mean, stddev))
        if rand_num != i:
            return rand_num

#%%

s = 16

numIterations = int(1e3)

l = s
n_per_leaf = l
n = n_per_leaf*l
nF=n - n_per_leaf
#%%
ratios = list()
for iteration in range(numIterations):
    demands = {}
    uplinks=np.zeros((s,s))
    downlinks=np.zeros((s,s))
    
    for i in range(n):
        demands[i]=list()
        for j in range(nF):
            demands[i].append(generate_random_excluding_i(l, int(i/n_per_leaf)))
            # demands[i].append(generate_normal_excluding_i(l, int(i/n_per_leaf)))
            # demands[i].append(generate_exponential_excluding_i(0.01, l, int(i/n_per_leaf)))
            # demands[i].append(int(j/n_per_leaf)+1)
        # demands[i].sort()
            
    
    for i in range(n):
        rr = 0
        numDemands = np.arange(nF)
        random.shuffle(numDemands)
        for j in numDemands:
            uplinks[int(i/n_per_leaf)][rr%s] += 1
            downlinks[rr%l][demands[i][j]] += 1
            rr = rr + 1
            
    
    maxCongestion = np.max([uplinks,downlinks])
    bestCongestion = nF*n_per_leaf/s
    
    ratio = maxCongestion/bestCongestion
    ratios.append(ratio)
    print("uniform", s, iteration, ratio)
    
ratios1 = list()
for iteration in range(numIterations):
    demands = {}
    uplinks=np.zeros((s,s))
    downlinks=np.zeros((s,s))
    
    for i in range(n):
        demands[i]=list()
        for j in range(nF):
            # demands[i].append(generate_random_excluding_i(l, int(i/n_per_leaf)))
            if (int(j/n_per_leaf)< int(i/n_per_leaf)):
                demands[i].append(int(j/n_per_leaf))
            else:
                demands[i].append(int(j/n_per_leaf)+1)
        # random.shuffle(demands[i])
    
    TotalDemand = 0
    for i in range(n):
        rr = 0
        numDemands = np.arange(nF)
        # random.shuffle(numDemands)
        for j in numDemands:
            dem = 1 #np.random.exponential()
            TotalDemand += dem
            # uplinks[int(i/n_per_leaf)][random.randint(0,l-1)] += dem
            uplinks[int(i/n_per_leaf)][rr%s] += dem
            downlinks[rr%s][demands[i][j]] += dem
            # print(rr%s,demands[i][j],i,j)
            rr += 1
            
    
    maxCongestion = np.max([uplinks,downlinks])
    bestCongestion = TotalDemand/(l*s)
    
    ratio = (maxCongestion/bestCongestion - 1) * 100
    ratios1.append(ratio)
    print("a2a", s, iteration, ratio)
    
#%%

# Count unique values
unique_values, counts = np.unique(demands[33], return_counts=True)

# Print unique values and their counts
for value, count in zip(unique_values, counts):
    print(f"Value: {value}, Count: {count}")


#%%

fig,ax = plt.subplots(1,1)
x=np.sort(ratios)
y=np.arange(len(ratios))/float(len(ratios))
# ax.plot(x,y,c = 'k')
# ax.plot(x[-1],y[-1],c = 'k',marker='s', markersize = 10, label="Uniform distribution")

x=np.sort(ratios1)
y=np.arange(len(ratios1))/float(len(ratios1))
ax.plot(x,y,c = 'r')
ax.plot(x[-1],y[-1],c = 'r',marker='d', markersize = 10, label="All to  All (skewed arrival)")

ax.xaxis.grid(True,ls='--')
ax.yaxis.grid(True,ls='--')
ax.set_xlabel("Max Congestion ("+r'$\frac{ALG}{OPT}$'+")")
ax.set_ylabel("CDF")
ax.set_xlim(0.9,1.5)

ax.legend()