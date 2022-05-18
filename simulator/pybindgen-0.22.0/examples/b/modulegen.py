#! /usr/bin/env python

import sys

import pybindgen
from pybindgen import ReturnValue, Parameter, Module, Function, FileCodeSink
from pybindgen import CppMethod, CppConstructor, CppClass, Enum


def my_module_gen(out_file):

    mod = Module('b')
    mod.add_include('"b.h"')

    B = mod.add_class('B')
    B.add_constructor([])
    B.add_copy_constructor()
    B.add_instance_attribute('b_a', ReturnValue.new('uint32_t'))
    B.add_instance_attribute('b_b', ReturnValue.new('uint32_t'))

    mod.add_function('BDoA', None, [Parameter.new('B', 'b')])
    mod.add_function('BDoB', ReturnValue.new('B'), [])

    mod.generate(FileCodeSink(out_file) )

if __name__ == '__main__':
    my_module_gen(sys.stdout)
