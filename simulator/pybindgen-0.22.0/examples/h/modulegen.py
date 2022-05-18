#! /usr/bin/env python

import sys

import pybindgen
from pybindgen import ReturnValue, Parameter, Module, CppClass, CppMethod, CppConstructor, FileCodeSink

def my_module_gen(out_file):

    mod = Module('h')
    mod.add_include('"h.h"')

    H = mod.add_class('H')
    H.add_constructor([])
    H.add_method('Do', None, [])

    Inner = mod.add_class('Inner', outer_class=H)
    Inner.add_constructor([])
    Inner.add_method('Do', None, [])

    MostInner = mod.add_class('MostInner', outer_class=Inner)
    MostInner.add_constructor([])
    MostInner.add_method('Do', None, [])

    mod.generate(FileCodeSink(out_file))

if __name__ == '__main__':
    my_module_gen(sys.stdout)
