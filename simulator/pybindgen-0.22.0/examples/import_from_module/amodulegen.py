#! /usr/bin/env python

import sys

import pybindgen
from pybindgen import ReturnValue, Parameter, Module, Function, FileCodeSink
from pybindgen import CppMethod, CppConstructor, CppClass, Enum


def my_module_gen(out_file):
    mod = Module('a')
    mod.add_include('"a.h"')

    # Base
    Base = mod.add_class("Base", allow_subclassing = True)
    Base.add_constructor([])
    Base.add_method("do_something", None, [], is_virtual=True)

    mod.generate(FileCodeSink(out_file) )

if __name__ == '__main__':
    my_module_gen(sys.stdout)
