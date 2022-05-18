#! /usr/bin/env python

import sys

import pybindgen
from pybindgen import FileCodeSink
from pybindgen.castxmlparser import ModuleParser

def my_module_gen():
    module_parser = ModuleParser('a1', '::')
    module = module_parser.parse([sys.argv[1]])
    module.add_include('"a.h"')

    module.generate(FileCodeSink(sys.stdout))

if __name__ == '__main__':
    my_module_gen()
