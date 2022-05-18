#! /usr/bin/env python
from __future__ import unicode_literals, print_function

import sys
import os.path

import pybindgen
from pybindgen.typehandlers import base as typehandlers
from pybindgen import (ReturnValue, Parameter, Module, Function, FileCodeSink)
from pybindgen import (CppMethod, CppConstructor, CppClass, Enum)
from pybindgen.function import CustomFunctionWrapper
from pybindgen.cppmethod import CustomCppMethodWrapper
from pybindgen.castxmlparser import ModuleParser, PygenClassifier, PygenSection

import foomodulegen_common




def my_module_gen():

    class MyPygenClassifier(PygenClassifier):
        def classify(self, pygccxml_definition):
            if pygccxml_definition.name and pygccxml_definition.name.lower() <= 'l':
                return 'foomodulegen_module1'
            else:
                return 'foomodulegen_module2'

    pygen = [
        PygenSection('__main__', FileCodeSink(open(sys.argv[3], "wt"))),
        PygenSection('foomodulegen_module1', FileCodeSink(open(sys.argv[4], "wt")),
                     'foomodulegen_module1_local'),
        PygenSection('foomodulegen_module2', FileCodeSink(open(sys.argv[5], "wt")),
                     'foomodulegen_module2_local'),
        ]
    module_parser = ModuleParser('foo4', '::')
    module_parser.enable_anonymous_containers = True

    gccxml_options = dict(
        include_paths=eval(sys.argv[2]),
        )

    module_parser.parse_init([sys.argv[1]], includes=['"foo.h"'], pygen_sink=pygen, pygen_classifier=MyPygenClassifier(),
                             gccxml_options=gccxml_options)
    module = module_parser.module
    foomodulegen_common.customize_module_pre(module)
    module.add_exception('exception', foreign_cpp_namespace='std', message_rvalue='%(EXC)s.what()')
    module_parser.scan_types()
    module_parser.scan_methods()
    module_parser.scan_functions()
    module_parser.parse_finalize()

    for sect in pygen:
        sect.code_sink.file.close()


if __name__ == '__main__':
    import os
    if "PYBINDGEN_ENABLE_PROFILING" in os.environ:
        try:
            import cProfile as profile
        except ImportError:
            my_module_gen()
        else:
            print("** running under profiler", file=sys.stderr)
            profile.run('my_module_gen()', 'foomodulegen-auto-split.pstat')
    else:
        my_module_gen()

