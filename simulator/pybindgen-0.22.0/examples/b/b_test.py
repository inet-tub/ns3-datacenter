import sys
sys.path.insert(0, "../../build/examples/b")
from b import *

b = B ()
b.b_a = 10
b.b_b = 5
BDoA (b)
b = BDoB ()
