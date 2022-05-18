#! /usr/bin/env python

import sys

import pybindgen
from pybindgen import ReturnValue, Parameter, Module, Function, FileCodeSink
from pybindgen import CppMethod, CppConstructor, CppClass, Enum
from pybindgen.typehandlers.base import ForwardWrapperBase


class VisitorParam(Parameter):

    DIRECTIONS = [Parameter.DIRECTION_IN]
    CTYPES = ['Visitor']

    def convert_python_to_c(self, wrapper):
        assert isinstance(wrapper, ForwardWrapperBase)

        py_cb = wrapper.declarations.declare_variable("PyObject*", self.name)
        wrapper.parse_params.add_parameter('O', ['&'+py_cb], self.name)

        wrapper.before_call.write_error_check("!PyCallable_Check(%s)" % py_cb,
                                              """PyErr_SetString(PyExc_TypeError, "visitor parameter must be callable");""")
        wrapper.call_params.append("_wrap_Visit")
        wrapper.before_call.write_code("Py_INCREF(%s);" % py_cb)
        wrapper.before_call.add_cleanup_code("Py_DECREF(%s);" % py_cb)
        wrapper.call_params.append(py_cb)


    def convert_c_to_python(self, wrapper):
        raise NotImplementedError



def my_module_gen(out_file):

    mod = Module('c')
    mod.add_include('"c.h"')

    mod.header.writeln("""void _wrap_Visit(int value, void *data);""")
    mod.body.writeln("""
void _wrap_Visit(int value, void *data)
{
    PyObject *callback = (PyObject*) data;
    PyObject_CallFunction(callback, (char*) "i", value);
}
""")

    mod.add_function("visit", None, [Parameter.new("Visitor", "visitor")]
                     # the 'data' parameter is inserted automatically
                     # by the custom callback type handler
                     )

    mod.generate(FileCodeSink(out_file))

if __name__ == '__main__':
    my_module_gen(sys.stdout)
