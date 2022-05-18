#! /usr/bin/env python
from __future__ import unicode_literals, print_function, absolute_import

import sys
import os
import logging

import pybindgen
from pybindgen.typehandlers import base as typehandlers
from pybindgen import (ReturnValue, Parameter, Module, Function, FileCodeSink)
from pybindgen import (CppMethod, CppConstructor, CppClass, Enum)
from pybindgen.function import CustomFunctionWrapper
from pybindgen.cppmethod import CustomCppMethodWrapper
from pybindgen.castxmlparser import ModuleParser

import foomodulegen_common


def my_module_gen():
    pygccxml_mode = sys.argv[4]

    out = FileCodeSink(sys.stdout)
    pygen_file = open(sys.argv[3], "wt")
    module_parser = ModuleParser('foo2', '::')
    module_parser.enable_anonymous_containers = True

    print("PYTHON_INCLUDES:", repr(sys.argv[2]), file=sys.stderr)
    kwargs = {
        "{mode}_options".format(mode=pygccxml_mode): dict(
            include_paths=eval(sys.argv[2]),
            )
    }
    module_parser.parse_init([sys.argv[1]], includes=['"foo.h"'], pygen_sink=FileCodeSink(pygen_file),
                             **kwargs)
    module = module_parser.module
    foomodulegen_common.customize_module_pre(module)

    module.add_exception('exception', foreign_cpp_namespace='std', message_rvalue='%(EXC)s.what()')
    module_parser.scan_types()
    module_parser.scan_methods()
    module_parser.scan_functions()
    module_parser.parse_finalize()

    pygen_file.close()

    foomodulegen_common.customize_module(module)

    module.generate(out)

def main():
    logging.basicConfig(level=logging.DEBUG)
    if sys.argv[1] == '-d':
        del sys.argv[1]
        import pdb
        pdb.set_trace()
        my_module_gen()
    else:
        import os
        if "PYBINDGEN_ENABLE_PROFILING" in os.environ:
            try:
                import cProfile as profile
            except ImportError:
                my_module_gen()
            else:
                print("** running under profiler", file=sys.stderr)
                profile.run('my_module_gen()', 'foomodulegen-auto.pstat')
        else:
            my_module_gen()

if __name__ == '__main__':
    main()

