#! /usr/bin/env python
from __future__ import unicode_literals, print_function

import sys
import re

import pybindgen
import pybindgen.utils
from pybindgen.typehandlers import base as typehandlers
from pybindgen.typehandlers.smart_ptr import BoostSharedPtr
from pybindgen import ReturnValue, Parameter, Module, Function, FileCodeSink
from pybindgen import CppMethod, CppConstructor, CppClass, Enum
from pybindgen.function import CustomFunctionWrapper
from pybindgen.cppmethod import CustomCppMethodWrapper
from pybindgen import cppclass

from pybindgen import param, retval

import pybindgen.settings
pybindgen.settings.deprecated_virtuals = False


def my_module_gen(out_file):

    mod = Module('bar')

    mod.add_include ('"bar.h"')

    Foo = mod.add_class('Foo', automatic_type_narrowing=True,
                        memory_policy=BoostSharedPtr('::Foo'))

    Foo.add_static_attribute('instance_count', ReturnValue.new('int'))
    Foo.add_constructor([Parameter.new('std::string', 'datum')])
    Foo.add_constructor([])
    Foo.add_method('get_datum', ReturnValue.new('const std::string'), [])
    Foo.add_method('is_initialized', ReturnValue.new('bool'), [], is_const=True)
    Foo.add_output_stream_operator()


    mod.add_function('function_that_takes_foo', ReturnValue.new('void'),
                               [param('boost::shared_ptr<Foo>', 'foo')])
    mod.add_function('function_that_returns_foo', retval('boost::shared_ptr<Foo>'), [])
    
    cls = mod.add_class('ClassThatTakesFoo', allow_subclassing=True)
    cls.add_constructor([Parameter.new('boost::shared_ptr<Foo>', 'foo')])
    cls.add_method('get_foo', ReturnValue.new('boost::shared_ptr<Foo>'), [])
    cls.add_method('get_modified_foo', retval('boost::shared_ptr<Foo>'),
                   [param('boost::shared_ptr<Foo>', 'foo')],
                   is_virtual=True, is_const=True)


    
    #### --- error handler ---
    class MyErrorHandler(pybindgen.settings.ErrorHandler):
        def __init__(self):
            super(MyErrorHandler, self).__init__()
            self.num_errors = 0
        def handle_error(self, wrapper, exception, traceback_):
            print("exception %s in wrapper %s" % (exception, wrapper), file=sys.stderr)
            self.num_errors += 1
            if 0: # verbose?
                import traceback
                traceback.print_tb(traceback_)
            return True
    pybindgen.settings.error_handler = MyErrorHandler()

    ## ---- finally, generate the whole thing ----
    mod.generate(FileCodeSink(out_file))


if __name__ == '__main__':
    import os
    if "PYBINDGEN_ENABLE_PROFILING" in os.environ:
        try:
            import cProfile as profile
        except ImportError:
            my_module_gen(sys.stdout)
        else:
            print("** running under profiler", file=sys.stderr)
            profile.run('my_module_gen(sys.stdout)', 'foomodulegen.pstat')
    else:
        my_module_gen(sys.stdout)

