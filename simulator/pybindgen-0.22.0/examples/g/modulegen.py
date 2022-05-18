#! /usr/bin/env python

import sys

import pybindgen
from pybindgen import ReturnValue, Parameter, Module, Function, FileCodeSink

def my_module_gen(out_file):
    mod = Module('g')
    mod.add_include('"g.h"')

    mod.add_function('GDoA', None, [])
    G = mod.add_cpp_namespace("G")
    G.add_function('GDoB', None, [])
    GInner = G.add_cpp_namespace("GInner")
    GInner.add_function('GDoC', None, [])

    G.add_include('<fstream>')

    ofstream = G.add_class('ofstream', foreign_cpp_namespace='::std')
    ofstream.add_enum('openmode', [
            ('app', 'std::ios_base::app'),
            ('ate', 'std::ios_base::ate'),
            ('binary', 'std::ios_base::binary'),
            ('in', 'std::ios_base::in'),
            ('out', 'std::ios_base::out'),
            ('trunc', 'std::ios_base::trunc'),
            ])
    ofstream.add_constructor([Parameter.new("const char *", 'filename'),
                              Parameter.new("::std::ofstream::openmode", 'mode', default_value="std::ios_base::out")])
    ofstream.add_method('close', None, [])

    mod.generate(FileCodeSink(out_file))

if __name__ == '__main__':
    my_module_gen(sys.stdout)
