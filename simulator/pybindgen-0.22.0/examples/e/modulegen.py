#! /usr/bin/env python

import sys

import pybindgen
from pybindgen import ReturnValue, Parameter, Module, Function, FileCodeSink
from pybindgen import CppMethod, CppConstructor, CppClass, Enum
from pybindgen import cppclass

def my_module_gen(out_file):

    mod = Module('e')
    mod.add_include('"e.h"')

    E = mod.add_class('E', memory_policy=cppclass.ReferenceCountingMethodsPolicy(decref_method='Unref', incref_method='Ref'))
    if 1:
        E.add_function_as_constructor("E::CreateWithRef", ReturnValue.new("E*", caller_owns_return=True), [])
    else:
        ## alternative:
        E.add_function_as_constructor("E::CreateWithoutRef", ReturnValue.new("E*", caller_owns_return=False), [])
    E.add_method("Do", None, [])


    mod.generate(FileCodeSink(out_file) )

if __name__ == '__main__':
    my_module_gen(sys.stdout)
