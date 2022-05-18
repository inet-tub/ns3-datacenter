import sys
sys.path.insert(0, "../../build/examples/f")
from f import *


class FDerived(FBase):
    def DoA (self):
        print "FDerived::DoA"
    def PrivDoB (self):
        print "FDerived::PrivDoB"

f = FDerived ()
f.DoA ()
f.DoB ()

