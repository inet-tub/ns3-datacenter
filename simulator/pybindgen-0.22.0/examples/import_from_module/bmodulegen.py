#! /usr/bin/env python

import sys

import pybindgen
from pybindgen import ReturnValue, Parameter, Module, Function, FileCodeSink
from pybindgen import CppMethod, CppConstructor, CppClass, Enum


def my_module_gen(out_file):
    mod = Module('b')
    mod.add_include('"b.h"')

    # Base
    Base = mod.add_class("Base", allow_subclassing = True, import_from_module='a')
    Base.add_constructor([])
    Base.add_method("do_something", None, [], is_virtual=True)

    # Derived
    Derived = mod.add_class("Derived", allow_subclassing=True, parent=Base)
    Derived.add_constructor([])
    Derived.add_method("do_something", None, [], is_virtual=True)

    mod.generate(FileCodeSink(out_file) )

if __name__ == '__main__':
    my_module_gen(sys.stdout)
