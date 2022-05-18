import sys
import os.path
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                '..', '..', 'build', 'examples', 'std_shared_ptr'))

import sp

def factory():
    return sp.Foo('factory')

def use():
    a = factory()
    print a.get_datum()

use()

f = sp.Foo("hello123")
sp.function_that_takes_foo(f)
f1 = sp.function_that_returns_foo()
print f1.get_datum()
f.set_datum("xxxx")
print f1.get_datum()

