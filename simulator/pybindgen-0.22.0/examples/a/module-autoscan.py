#! /usr/bin/env python

import sys

from pybindgen import FileCodeSink
from pybindgen.castxmlparser import ModuleParser

def my_module_gen():
    module_parser = ModuleParser('a2', '::')
    module_parser.parse([sys.argv[1]], includes=['"a.h"'], pygen_sink=FileCodeSink(sys.stdout))

if __name__ == '__main__':
    my_module_gen()
