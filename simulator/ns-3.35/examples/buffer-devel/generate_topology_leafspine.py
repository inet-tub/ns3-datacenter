import os
import pandas as pd
import numpy as np
import sys

# leaf spine topology
serversPerTor=24
torSwitches=32

spineSwitches=8

serverDataRate=40
allOtherDataRate=40

serverTorDelay="2us"
torSpineDelay="2us"

errorRate=0


switch_num = torSwitches + spineSwitches
node_num = serversPerTor*torSwitches + switch_num
link_num = serversPerTor*torSwitches + (torSwitches)*spineSwitches

print (str(node_num)+" "+str(switch_num)+" "+str(torSwitches)+" "+str(link_num),end="")
print("\n",end="")

for i in range(switch_num):
	print(str(int(serversPerTor*torSwitches+i)),end =" ")
print("\n",end="")


for i in range(torSwitches):
	for j in range(serversPerTor):
		print(str(j+serversPerTor*i)+" "+str(serversPerTor*torSwitches+i)+" "+str(serverDataRate*1e9)+" "+serverTorDelay+" "+str(errorRate))

torIndexBegin=serversPerTor*torSwitches
spineIndexBegin=serversPerTor*torSwitches+torSwitches

for i in range(torSwitches):
	for j in range(spineSwitches):
		print(str(torIndexBegin + i)+" "+str(spineIndexBegin + j )+" "+str(allOtherDataRate*1e9)+" "+torSpineDelay+" "+str(errorRate))