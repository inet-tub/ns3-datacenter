#! /usr/bin/env python
from __future__ import unicode_literals, print_function

import sys
import re

import pybindgen
import pybindgen.settings
pybindgen.settings.deprecated_virtuals = False

from pybindgen.typehandlers import base as typehandlers
from pybindgen import ReturnValue, Parameter, Module, Function, FileCodeSink
from pybindgen import CppMethod, CppConstructor, CppClass, Enum
from pybindgen.function import CustomFunctionWrapper
from pybindgen.cppmethod import CustomCppMethodWrapper



class PointerHolderTransformation(typehandlers.TypeTransformation):
    def __init__(self):
        self.rx = re.compile(r'(?:::)?PointerHolder<\s*(\w+)\s*>')

    def get_untransformed_name(self, name):
        m = self.rx.match(name)
        if m is None:
            return None
        else:
            return m.group(1)+' *'

    def create_type_handler(self, type_handler, *args, **kwargs):
        if issubclass(type_handler, Parameter):
            kwargs['transfer_ownership'] = False
        elif issubclass(type_handler, ReturnValue):
            kwargs['caller_owns_return'] = True
        else:
            raise AssertionError
        handler = type_handler(*args, **kwargs)
        handler.set_transformation(self, self.get_untransformed_name(args[0]))
        return handler

    def untransform(self, type_handler, declarations, code_block, expression):
        return '(%s).thePointer' % (expression,)

    def transform(self, type_handler, declarations, code_block, expression):
        assert type_handler.untransformed_ctype[-1] == '*'
        var = declarations.declare_variable(
            'PointerHolder<%s>' % type_handler.untransformed_ctype[:-1], 'tmp')
        return '(%s.thePointer = (%s), %s)' % (var, expression, var)

transf = PointerHolderTransformation()
typehandlers.return_type_matcher.register_transformation(transf)
typehandlers.param_type_matcher.register_transformation(transf)
del transf

def customize_module_pre(module):
    standard_error = module.add_exception('out_of_range', foreign_cpp_namespace='std',
                                          custom_name='IndexError', is_standard_error=True)


def customize_module(module):
    pybindgen.settings.wrapper_registry = pybindgen.settings.StdMapWrapperRegistry

    wrapper_body = '''
static PyObject *
_wrap_foofunction_that_takes_foo_from_string(PyObject * PYBINDGEN_UNUSED(dummy), PyObject *args,
                                             PyObject *kwargs, PyObject **return_exception)
{
    PyObject *py_retval;
    char *datum;
    const char *keywords[] = {"foo", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, (char *) "s", (char **) keywords, &datum)) {
        {
            PyObject *exc_type, *traceback;
            PyErr_Fetch(&exc_type, return_exception, &traceback);
            Py_XDECREF(exc_type);
            Py_XDECREF(traceback);
        }
        return NULL;
    }
    function_that_takes_foo(Foo(datum));
    Py_INCREF(Py_None);
    py_retval = Py_None;
    return py_retval;
}
'''

    if len(module.classes) == 0:
        # Some tests, eg. foomodulegen4.py, don't have Bar and Foo classes
        # defined.
        return

    module.add_custom_function_wrapper('function_that_takes_foo',
                                       '_wrap_foofunction_that_takes_foo_from_string',
                                       wrapper_body,
                                       docstring="I'm awake you rascals!")


    ## test a custom method wrapper
    Bar, = [cls for cls in module.classes if cls.name == 'Bar']
    wrapper_body = '''
static PyObject *
_wrap_PyBar_Hooray_lenx(PyBar *PYBINDGEN_UNUSED(dummy), PyObject *args, PyObject *kwargs,
                        PyObject **return_exception)
{
    PyObject *py_retval;
    int x;
    const char *keywords[] = {"x", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, (char *) "i", (char **) keywords, &x)) {
        PyObject *exc_type, *traceback;
        PyErr_Fetch(&exc_type, return_exception, &traceback);
        Py_XDECREF(exc_type);
        Py_XDECREF(traceback);
        return NULL;
    }

    std::string retval;
    retval = Bar::Hooray();
    py_retval = Py_BuildValue((char *) "i", int(retval.size() + x));
    return py_retval;
}
'''
    Bar.add_custom_method_wrapper("Hooray", "_wrap_PyBar_Hooray_lenx",
                                  wrapper_body,
                                  flags=["METH_VARARGS", "METH_KEYWORDS", "METH_STATIC"],
                                  docstring='Zzzz.... Have good dreams.')


    Foo, = [cls for cls in module.classes if cls.name == 'Foo']
    def Foo_instance_creation_function(dummy_cpp_class, code_block, lvalue,
                                       parameters, construct_type_name):
        code_block.write_code(
            "%s = new %s(%s);" % (lvalue, construct_type_name, parameters))
        code_block.write_code("%s->initialize();" % (lvalue,))
    Foo.set_instance_creation_function(Foo_instance_creation_function)

    VectorLike2, = [cls for cls in module.classes if cls.name == 'VectorLike2']
    VectorLike2.add_container_traits(ReturnValue.new('double'), begin_method='Begin', end_method='End', iterator_type='Iterator')

    MapLike, = [cls for cls in module.classes if cls.name == 'MapLike']
    MapLike.add_container_traits((ReturnValue.new('int'), ReturnValue.new('double')),
                                 begin_method='Begin', end_method='End', iterator_type='Iterator',
                                 is_mapping=True)

    # just a compilation test, this won't actually work in runtime
    #module.add_include('<stdio.h>')
    #module.add_class(name="FILE", foreign_cpp_namespace="", import_from_module="__builtin__ named file")
    #module.add_enum("reg_errcode_t",   ["REG_NOERROR", "REG_NOMATCH"], import_from_module="__builtin__")

