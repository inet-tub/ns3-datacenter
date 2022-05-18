#!/usr/bin/env python
from __future__ import unicode_literals, print_function

import sys, os

import pybindgen
from pybindgen import typehandlers
from pybindgen.typehandlers import codesink
from pybindgen.typehandlers.base import Parameter, ReturnValue
from pybindgen.function import Function
from pybindgen.module import Module
from pybindgen import cppclass
from pybindgen.typehandlers.base import NotSupportedError

import pybindgen.settings
pybindgen.settings.deprecated_virtuals = False

import re

stdint_rx = re.compile(".*int\\d+_t.*")

class MyReverseWrapper(typehandlers.base.ReverseWrapperBase):
    def generate_python_call(self):
        params = ['NULL'] # function object to call
        build_params = self.build_params.get_parameters()
        if build_params[0][0] == '"':
            build_params[0] = '(char *) ' + build_params[0]
        params.extend(build_params)
        self.before_call.write_code('py_retval = PyObject_CallFunction(%s);' % (', '.join(params),))
        self.before_call.write_error_check('py_retval == NULL')
        self.before_call.add_cleanup_code('Py_DECREF(py_retval);')


def type_blacklisted(type_name):
    if type_name.startswith('Glib::'):
        return True
    return False

        

def test():
    code_out = codesink.FileCodeSink(sys.stdout)
    pybindgen.write_preamble(code_out)
    sys.stdout.write("""
#include <string>
#include <stdint.h>
""")
    ## Declare a dummy class
    sys.stdout.write('''
class Foo
{
    std::string m_datum;
public:
    Foo () : m_datum ("")
        {}
    Foo (std::string datum) : m_datum (datum)
        {}
    std::string get_datum () const { return m_datum; }

    Foo (Foo const & other) : m_datum (other.get_datum ())
        {}
};
''')

    module = Module("foo")

    ## Register type handlers for the class
    Foo = module.add_class('Foo')
    Foo.add_constructor([Parameter.new("const Foo&", "foo")])
    #Foo.full_name = Foo.name # normally adding the class to a module would take care of this
    Foo.generate_forward_declarations(code_out, module)

    wrapper_number = 0

    ## test return type handlers of reverse wrappers
    for return_type, return_handler in list(typehandlers.base.return_type_matcher.items()):
        if type_blacklisted(return_type):
            continue
        if os.name == 'nt':
            if stdint_rx.search(return_type):
                continue # win32 does not support the u?int\d+_t types (defined in <stdint.h>)

        code_out.writeln("/* Test %s (%s) return type  */" % (return_type, return_handler))
        if issubclass(return_handler, (cppclass.CppClassPtrReturnValue,
                                       typehandlers.pyobjecttype.PyObjectReturnValue)):
            for caller_owns_return in True, False:
                retval = return_handler(return_type, caller_owns_return=caller_owns_return)
                wrapper = MyReverseWrapper(retval, [])
                wrapper_number += 1
                try:
                    wrapper.generate(code_out,
                                     '_test_wrapper_number_%i' % (wrapper_number,),
                                     ['static'])
                except NotImplementedError:
                    sys.stderr.write("ReverseWrapper %s(void) (caller_owns_return=%r)"
                                     " could not be generated: not implemented\n"
                                     % (retval.ctype, caller_owns_return))
                sys.stdout.write("\n")
        else:
            retval = return_handler(return_type)
            try:
                wrapper = MyReverseWrapper(retval, [])
            except NotSupportedError:
                continue
            except NotImplementedError:
                sys.stderr.write("ReverseWrapper %s(void) could not be generated: not implemented\n"
                                 % (retval.ctype))
                continue
            wrapper_number += 1
            memsink = codesink.MemoryCodeSink()
            try:
                wrapper.generate(memsink,
                                 '_test_wrapper_number_%i' % (wrapper_number,),
                                 ['static'])
            except NotImplementedError:
                sys.stderr.write("ReverseWrapper %s xxx (void) could not be generated: not implemented\n"
                                 % (retval.ctype,))
                continue
            except NotSupportedError:
                continue
            else:
                memsink.flush_to(code_out)
            sys.stdout.write("\n")


    ## test parameter type handlers of reverse wrappers
    for param_type, param_handler in list(typehandlers.base.param_type_matcher.items()):
        if type_blacklisted(param_type):
            continue
        if os.name == 'nt':
            if stdint_rx.search(param_type):
                continue # win32 does not support the u?int\d+_t types (defined in <stdint.h>)
        for direction in param_handler.DIRECTIONS:
            if direction == (Parameter.DIRECTION_IN):
                param_name = 'param'
            elif direction == (Parameter.DIRECTION_IN|Parameter.DIRECTION_OUT):
                param_name = 'param_inout'
            elif direction == (Parameter.DIRECTION_OUT):
                param_name = 'param_out'

                def try_wrapper(param, wrapper_number):
                    if 'const' in param.ctype and direction&Parameter.DIRECTION_OUT:
                        return
                    code_out.writeln("/* Test %s (%s) param type  */" % (param_type, param_handler))
                    wrapper = MyReverseWrapper(ReturnValue.new('void'), [param])
                    try:
                        wrapper.generate(code_out,
                                         '_test_wrapper_number_%i' % (wrapper_number,),
                                         ['static'])
                    except NotImplementedError:
                        sys.stderr.write("ReverseWrapper void(%s) could not be generated: not implemented"
                                         % (param.ctype))
                    sys.stdout.write("\n")

                if issubclass(param_handler, (cppclass.CppClassPtrParameter,
                                              typehandlers.pyobjecttype.PyObjectParam)):
                    for transfer_ownership in True, False:
                        try:
                            param = param_handler(param_type, param_name, transfer_ownership=transfer_ownership)
                        except TypeError:
                            sys.stderr.write("ERROR -----> param_handler(param_type=%r, "
                                             "transfer_ownership=%r, is_const=%r)\n"
                                             % (param_type, transfer_ownership, is_const))
                        wrapper_number += 1
                        try_wrapper(param, wrapper_number)
                else:
                    param = param_handler(param_type, param_name, direction)
                    wrapper_number += 1
                    try_wrapper(param, wrapper_number)

    
    ## test generic forward wrappers, and module

    for return_type, return_handler in list(typehandlers.base.return_type_matcher.items()):
        if type_blacklisted(return_type):
            continue
        if os.name == 'nt':
            if stdint_rx.search(return_type):
                continue # win32 does not support the u?int\d+_t types (defined in <stdint.h>)
        wrapper_number += 1
        function_name = 'foo_function_%i' % (wrapper_number,)
        ## declare a fake prototype
        sys.stdout.write("%s %s(void);\n\n" % (return_type, function_name))

        if issubclass(return_handler, (cppclass.CppClassPtrReturnValue,
                                       typehandlers.pyobjecttype.PyObjectReturnValue)):
            retval = return_handler(return_type, caller_owns_return=True)
        else:
            retval = return_handler(return_type)

        module.add_function(function_name, retval, [])
    
    for param_type, param_handler in list(typehandlers.base.param_type_matcher.items()):
        if type_blacklisted(param_type):
            continue
        if os.name == 'nt':
            if stdint_rx.search(param_type):
                continue # win32 does not support the u?int\d+_t types (defined in <stdint.h>)

        for is_const in [True, False]:
            for direction in param_handler.DIRECTIONS:
                if direction == (Parameter.DIRECTION_IN):
                    param_name = 'param'
                elif direction == (Parameter.DIRECTION_IN|Parameter.DIRECTION_OUT):
                    param_name = 'param_inout'
                elif direction == (Parameter.DIRECTION_OUT):
                    param_name = 'param_out'

                if is_const and direction & Parameter.DIRECTION_OUT:
                    continue # const and output parameter makes no sense

                if is_const:
                    if '&' in param_type: # const references not allowed
                        continue
                    param_type_with_const = "const %s" % (param_type,)
                else:
                    param_type_with_const = param_type

                if issubclass(param_handler, (cppclass.CppClassPtrParameter,
                                              typehandlers.pyobjecttype.PyObjectParam)):
                    for transfer_ownership in True, False:
                        name = param_name + (transfer_ownership and '_transfer' or '_notransfer')
                        try:
                            param = param_handler(param_type, name, transfer_ownership=transfer_ownership)
                        except TypeError:
                            sys.stderr.write("ERROR -----> param_handler(param_type=%r, "
                                             "name=%r, transfer_ownership=%r, is_const=%r)\n"
                                             % (param_type, name, transfer_ownership, is_const))
                        wrapper_number += 1
                        function_name = 'foo_function_%i' % (wrapper_number,)
                        ## declare a fake prototype
                        sys.stdout.write("void %s(%s %s);\n\n" % (function_name, param_type_with_const, name))
                        module.add_function(function_name, ReturnValue.new('void'), [param])
                else:
                    param = param_handler(param_type, param_name, direction)
                    wrapper_number += 1
                    function_name = 'foo_function_%i' % (wrapper_number,)
                    ## declare a fake prototype
                    sys.stdout.write("void %s(%s);\n\n" % (function_name, param_type_with_const))
                    module.add_function(function_name, ReturnValue.new('void'), [param])

    module.generate(code_out)



if __name__ == '__main__':
    test()
