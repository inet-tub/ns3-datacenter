import os
import pandas as pd
import numpy as np
import sys


# servers=int(sys.argv[1])
# tors=int(sys.argv[2])

# # topof >> node_num >> switch_num >> tors >> link_num;
# # then all switch ids
# # topof >> sid;
# # then all connections
# # topof >> src >> dst >> data_rate >> link_delay >> error_rate; Number of lines = link_num

# node_num = int(servers*tors + tors)
# switch_num = int(tors) # Lol. spines are not used for now. 
# link_num = int(servers*tors + tors*(tors-1)/2)

# print (str(node_num)+" "+str(switch_num)+" "+str(tors)+" "+str(link_num),end="")
# print("\n",end="")

# for i in range (switch_num):
# 	print(str(int(servers*tors+i)),end =" ")
# print("\n",end="")

# serverTorDataRate=str(int(sys.argv[3])*1e9)
# serverTorDelay=str(sys.argv[4])+'us'
# errorRateServer=float(sys.argv[5])
# for i in range(tors):
# 	for j in range(servers):
# 		print(str(j+servers*i)+" "+str(servers*tors+i)+" "+serverTorDataRate+" "+serverTorDelay+" "+str(errorRateServer))

# TorTorDataRate=str(int(sys.argv[6])*1e9)
# TorTorDelay=str(sys.argv[7])+'us'
# errorRateTor=float(sys.argv[8])

# for i in range(tors):
# 	for j in range(i+1,tors):
# 		print(str(servers*tors+i)+" "+str(servers*tors+j)+" "+TorTorDataRate+" "+TorTorDelay+" "+str(errorRateTor))


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

print (str(node_num)+" "+str(switch_num)+" "+str(torSwitches)+" "+str(link_num),end="")
print("\n",end="")

for i in range(switch_num):
	print(str(int(serversPerTor*torSwitches+i)),end =" ")
print("\n",end="")


for i in range(torSwitches):
	for j in range(serversPerTor):
		print(str(j+serversPerTor*i)+" "+str(serversPerTor*torSwitches+i)+" "+str(serverDataRate*1e9)+" "+serverTorDelay+" "+str(errorRate))

torIndexBegin=serversPerTor*torSwitches
aggIndexBegin=serversPerTor*torSwitches+torSwitches

for i in range(numPod):
	for j in range(numToRperPod): # for each tor in pod
		for k in range(numToRperPod): # for each agg in pod
			print(str(torIndexBegin + j+numToRperPod*i)+" "+str(aggIndexBegin + k+numToRperPod*i )+" "+str(allOtherDataRate*1e9)+" "+torAggDelay+" "+str(errorRate))


coreIndexBegin=serversPerTor*torSwitches+torSwitches+aggSwitches

for i in range(aggSwitches):
	for j in range(coreSwitches):
		print(str(aggIndexBegin + i)+" "+str(coreIndexBegin + j )+" "+str(allOtherDataRate*1e9)+" "+aggCoreDelay+" "+str(errorRate))
