import sys
sys.path.insert(0, "../../build/examples/h")
from h import *

h = H()
h.Do()
inner = H.Inner()
inner.Do()
most_inner = H.Inner.MostInner()
most_inner.Do()

