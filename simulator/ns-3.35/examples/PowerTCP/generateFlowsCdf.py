#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Sun Feb 28 20:17:37 2021

@author: vamsi
"""
import os
import pandas as pd
import numpy as np
import sys

serversPerTor=32
torSwitches=4
aggSwitches=torSwitches
numToRperPod = 2 # must be a multiple of torSwitches
numPod=int(torSwitches/numToRperPod)

coreSwitches=int(aggSwitches/2)


serverDataRate=25
allOtherDataRate=100

serverTorDelay="1us"
torAggDelay="1us"
aggCoreDelay="5us"

errorRate=0


switch_num = torSwitches + aggSwitches + coreSwitches
node_num = serversPerTor*torSwitches + switch_num
link_num = serversPerTor*torSwitches + (torSwitches)*numToRperPod + aggSwitches*coreSwitches

# print (str(node_num)+" "+str(switch_num)+" "+str(torSwitches)+" "+str(link_num),end="")
# print("\n",end="")

# for i in range(switch_num):
# 	print(str(int(serversPerTor*torSwitches+i)),end =" ")
# print("\n",end="")


# for i in range(torSwitches):
# 	for j in range(serversPerTor):
# 		print(str(j+serversPerTor*i)+" "+str(serversPerTor*torSwitches+i)+" "+str(serverDataRate*1e9)+" "+serverTorDelay+" "+str(errorRate))

# torIndexBegin=serversPerTor*torSwitches
# aggIndexBegin=serversPerTor*torSwitches+torSwitches

# for i in range(numPod):
# 	for j in range(numToRperPod): # for each tor in pod
# 		for k in range(numToRperPod): # for each agg in pod
# 			print(str(torIndexBegin + j+numToRperPod*i)+" "+str(aggIndexBegin + k+numToRperPod*i )+" "+str(allOtherDataRate*1e9)+" "+torAggDelay+" "+str(errorRate))


# coreIndexBegin=serversPerTor*torSwitches+torSwitches+aggSwitches

# for i in range(aggSwitches):
# 	for j in range(coreSwitches):
# 		print(str(aggIndexBegin + i)+" "+str(coreIndexBegin + j )+" "+str(allOtherDataRate*1e9)+" "+aggCoreDelay+" "+str(errorRate))


cdf=pd.read_csv('websearch.txt',delimiter=' ',usecols=[0,1],names=["size","p"])
