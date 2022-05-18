import sys
import os.path
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                '..', '..', 'build', 'examples', 'boost_shared_ptr'))

import bsp


f = bsp.Foo("hello123")
bsp.function_that_takes_foo(f)
f1 = bsp.function_that_returns_foo()
print f1.get_datum()
f.set_datum("xxxx")
print f1.get_datum()

