import sys
sys.path.insert(0, "../../build/examples/c")
from c import *

c = C (10)
C.DoA ()
c.DoB ()
c.DoC (5)
c_v = c.DoD ()
c.DoE ()

