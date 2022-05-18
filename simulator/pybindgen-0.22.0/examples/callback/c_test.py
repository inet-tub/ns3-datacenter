import sys
sys.path.insert(0, "../../build/examples/callback")
import c

def visitor(value):
    print value

c.visit(visitor)

