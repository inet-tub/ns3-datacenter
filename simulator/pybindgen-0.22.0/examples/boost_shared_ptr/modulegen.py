#! /usr/bin/env python

import sys, os
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                '..', '..'))

#import pybindgen
#import pybindgen.utils
#from pybindgen.typehandlers import base as typehandlers
from pybindgen import Module, FileCodeSink, param, retval
#from pybindgen import CppMethod, CppConstructor, CppClass, Enum
#from pybindgen.function import CustomFunctionWrapper
#from pybindgen.cppmethod import CustomCppMethodWrapper
from pybindgen import cppclass
from pybindgen.typehandlers.smart_ptr import BoostSharedPtr

#from pybindgen import param, retval

import pybindgen.settings
pybindgen.settings.deprecated_virtuals = False


def my_module_gen(out_file):

    mod = Module('bsp')

    mod.add_include ('"bsp.h"')

    Foo = mod.add_class('Foo', memory_policy=BoostSharedPtr('::Foo'))

    Foo.add_constructor([param('std::string', 'datum')])
    Foo.add_constructor([])
    Foo.add_method('get_datum', retval('const std::string'), [])
    Foo.add_method('set_datum', None, [param('const std::string', 'datum')])


    mod.add_function('function_that_takes_foo', None,
                     [param('boost::shared_ptr<Foo>', 'foo')])

    mod.add_function('function_that_returns_foo', retval('boost::shared_ptr<Foo>'), [])
    
    ## ---- finally, generate the whole thing ----
    mod.generate(FileCodeSink(out_file))


if __name__ == '__main__':
    my_module_gen(sys.stdout)

