import os
import pandas as pd
import numpy as np
import sys

#### This script is only meant for 2-ToR topology. Servers from ToR-1 send long flows to servers of ToR-2.

servers=int(sys.argv[1])
tors=2

# flowf >> flow_input.src >> flow_input.dst >> flow_input.pg >> flow_input.dport >> flow_input.maxPacketCount >> flow_input.start_time;

num_flows=servers

dport=10000;
print (str(servers))
maxBytes=1e12
for i in range (servers):
	print(str(i)+" "+str(servers+i)+" "+"3"+" "+str(dport)+" "+str(int(maxBytes))+" "+"0.1")
	dport+=1