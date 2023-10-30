import os
import pandas as pd
import numpy as np
import sys

# leaf spine topology
serversPerTor=16
torSwitches=16
spineSwitches=4
serverTorDelay="2us"
torSpineDelay="2us"
serverDataRate=25
allOtherDataRate=25
errorRate=0

switch_num = torSwitches + spineSwitches
node_num = serversPerTor*torSwitches + switch_num
link_num = serversPerTor*torSwitches + (torSwitches)*spineSwitches

print (str(node_num)+" "+str(switch_num)+" "+str(torSwitches)+" "+str(link_num)+" "+str(int(serverDataRate*1e9))+" "+str(int(allOtherDataRate*1e9)),end="")
print("\n",end="")

for i in range(switch_num):
	print(str(int(serversPerTor*torSwitches+i)),end =" ")
print("\n",end="")


for i in range(torSwitches):
	for j in range(serversPerTor):
		print(str(j+serversPerTor*i)+" "+str(serversPerTor*torSwitches+i)+" "+str(int(serverDataRate*1e9))+" "+serverTorDelay+" "+str(errorRate))

torIndexBegin=serversPerTor*torSwitches
spineIndexBegin=serversPerTor*torSwitches+torSwitches

for i in range(torSwitches):
	for j in range(spineSwitches):
		print(str(torIndexBegin + i)+" "+str(spineIndexBegin + j )+" "+str(int(allOtherDataRate*1e9))+" "+torSpineDelay+" "+str(errorRate))


