#! /usr/bin/env python

import sys

import pybindgen
from pybindgen import ReturnValue, Parameter, Module, Function, FileCodeSink
from pybindgen import CppMethod, CppConstructor, CppClass, Enum


def my_module_gen(out_file):

    mod = Module('c')
    mod.add_include('"c.h"')

    C = mod.add_class('C')
    C.add_constructor([])
    C.add_constructor([Parameter.new('uint32_t', 'c')])
    C.add_method('DoA', None, [], is_static=True)
    C.add_method('DoB', None, [])
    C.add_method('DoC', None, [Parameter.new('uint32_t', 'c')])
    C.add_method('DoD', ReturnValue.new('uint32_t'), [])
    C.add_method('DoE', None, [], is_virtual=True)

    mod.generate(FileCodeSink(out_file) )

if __name__ == '__main__':
    my_module_gen(sys.stdout)
