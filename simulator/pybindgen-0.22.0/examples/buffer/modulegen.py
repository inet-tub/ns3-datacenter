#! /usr/bin/env python

import sys

import pybindgen
from pybindgen import ReturnValue, Parameter, Module, Function, FileCodeSink
from pybindgen import CppMethod, CppConstructor, CppClass, Enum
from pybindgen.typehandlers.base import ForwardWrapperBase


class BufferReturn(ReturnValue):
    CTYPES = []

    def __init__(self, ctype, length_expression):
        super(BufferReturn, self).__init__(ctype, is_const=False)
        self.length_expression = length_expression

    def convert_c_to_python(self, wrapper):
        pybuf = wrapper.after_call.declare_variable("PyObject*", "pybuf")
        wrapper.after_call.write_code("%s = PyBuffer_FromReadWriteMemory(retval, (%s)*sizeof(short int));" % (pybuf, self.length_expression))
        wrapper.build_params.add_parameter("N", [pybuf], prepend=True)


def my_module_gen(out_file):

    mod = Module('c')
    mod.add_include('"c.h"')

    mod.add_function("GetBuffer", BufferReturn("unsigned short int*", "GetBufferLen()"), [])
    mod.add_function("GetBufferLen", ReturnValue.new("int"), [])
    mod.add_function("GetBufferChecksum", ReturnValue.new("unsigned short"), [])

    mod.generate(FileCodeSink(out_file))

if __name__ == '__main__':
    my_module_gen(sys.stdout)
