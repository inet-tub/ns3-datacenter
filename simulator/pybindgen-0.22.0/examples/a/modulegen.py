#! /usr/bin/env python

import sys

import pybindgen
from pybindgen import ReturnValue, Parameter, FileCodeSink
from pybindgen.module import Module
from pybindgen.function import Function

def my_module_gen(out_file):

    mod = Module('a')
    mod.add_include('"a.h"')

    mod.add_function('ADoA', None, [])
    mod.add_function('ADoB', None, [Parameter.new('uint32_t', 'b')])
    mod.add_function('ADoC', ReturnValue.new('uint32_t'), [])

    mod.generate(FileCodeSink(out_file) )

if __name__ == '__main__':
    my_module_gen(sys.stdout)
