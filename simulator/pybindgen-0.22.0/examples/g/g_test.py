import sys
sys.path.insert(0, "../../build/examples/g")
from g import *
from g.G import GDoB
from g.G.GInner import GDoC

GDoA()
G.GDoB()
GDoB()
G.GInner.GDoC()
GDoC()

