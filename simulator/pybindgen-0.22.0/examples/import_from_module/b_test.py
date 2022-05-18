import sys
sys.path.insert(0, "../../build/examples/import_from_module")

from b import Derived

obj = Derived()
obj.do_something()

