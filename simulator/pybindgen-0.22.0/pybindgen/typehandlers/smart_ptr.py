#
# ----- boost::shared_ptr -----------
#

from .base import Parameter, ReturnValue, \
    join_ctype_and_name, CodeGenerationError, \
    param_type_matcher, return_type_matcher, CodegenErrorBase, \
    DeclarationsScope, CodeBlock, NotSupportedError, ForwardWrapperBase, ReverseWrapperBase, \
    TypeConfigurationError

from pybindgen.cppclass import SmartPointerPolicy, CppClass, CppClassParameterBase, CppClassReturnValueBase, common_shared_object_return

class BoostSharedPtr(SmartPointerPolicy):
    def __init__(self, class_name):
        """
        Create a memory policy for using boost::shared_ptr<> to manage instances of this object.

        :param class_name: the full name of the class, e.g. foo::Bar
        """
        self.class_name = class_name
        self.pointer_template = '::boost::shared_ptr< %s >'

    def get_pointer_name(self, class_name):
        return self.pointer_template % (class_name,)
        
    def get_delete_code(self, cpp_class):
        return "self->obj.~shared_ptr< %s >();" % (cpp_class.full_name,)

    def get_pointer_type(self, class_full_name):
        return self.get_pointer_name(class_full_name) + ' '

    def get_pointer_to_void_name(self, object_name):
        return "%s.get()" % object_name

    def get_instance_creation_function(self):
        return boost_shared_ptr_instance_creation_function

    def get_pystruct_init_code(self, cpp_class, obj):
        return "new(&%s->obj) %s;" % (obj, self.get_pointer_name(cpp_class.full_name),)

    def register_ptr_parameter_and_return(self, cls, name):
        class ThisClassSharedPtrParameter(CppClassSharedPtrParameter):
            """Register this C++ class as pass-by-pointer parameter"""
            CTYPES = []
            cpp_class = cls
        cls.ThisClassSharedPtrParameter = ThisClassSharedPtrParameter
        try:
            param_type_matcher.register(self.get_pointer_name(cls.full_name), cls.ThisClassSharedPtrParameter)
        except ValueError:
            pass

        class ThisClassSharedPtrReturn(CppClassSharedPtrReturnValue):
            """Register this C++ class as pointer return"""
            CTYPES = []
            cpp_class = cls
        cls.ThisClassSharedPtrReturn = ThisClassSharedPtrReturn
        try:
            return_type_matcher.register(self.get_pointer_name(cls.full_name), cls.ThisClassSharedPtrReturn)
        except ValueError:
            pass

    def register_ptr_alias_parameter_and_return(self, cls, alias):        
        alias_ptr = self.get_pointer_name(alias)
        cls.ThisClassSharedPtrParameter.CTYPES.append(alias_ptr)
        try:
            param_type_matcher.register(alias_ptr, cls.ThisClassSharedPtrParameter)
        except ValueError: pass

        cls.ThisClassSharedPtrReturn.CTYPES.append(alias_ptr)
        try:
            return_type_matcher.register(alias_ptr, cls.ThisClassSharedPtrReturn)
        except ValueError: pass

def boost_shared_ptr_instance_creation_function(cpp_class, code_block, lvalue,
                                                parameters, construct_type_name):
    """
    boost::shared_ptr "instance creation function"; it is called whenever a new
    C++ class instance needs to be created

    :param cpp_class: the CppClass object whose instance is to be created
    :param code_block: CodeBlock object on which the instance creation code should be generated
    :param lvalue: lvalue expression that should hold the result in the end
    :param parameters: stringified list of parameters
    :param construct_type_name: actual name of type to be constructed (it is
                          not always the class name, sometimes it's
                          the python helper class)
    """
    assert lvalue
    assert not lvalue.startswith('None')
    if cpp_class.incomplete_type:
        raise CodeGenerationError("%s cannot be constructed (incomplete type)"
                                  % cpp_class.full_name)
    code_block.write_code(
        "%s.reset (new %s(%s));" % (lvalue, construct_type_name, parameters))

        
class StdSharedPtr(BoostSharedPtr):
    def __init__(self, class_name):
        """
        Create a memory policy for using std::shared_ptr<> to manage instances of this object.

        :param class_name: the full name of the class, e.g. foo::Bar
        """
        self.class_name = class_name
        self.pointer_template = '::std::shared_ptr< %s >'

    def get_instance_creation_function(self):
        return std_shared_ptr_instance_creation_function

def std_shared_ptr_instance_creation_function(cpp_class, code_block, lvalue,
                                              parameters, construct_type_name):
    """
    std::shared_ptr "instance creation function"; it is called whenever a new
    C++ class instance needs to be created

    :param cpp_class: the CppClass object whose instance is to be created
    :param code_block: CodeBlock object on which the instance creation code should be generated
    :param lvalue: lvalue expression that should hold the result in the end
    :param parameters: stringified list of parameters
    :param construct_type_name: actual name of type to be constructed (it is
                          not always the class name, sometimes it's
                          the python helper class)
    """
    assert lvalue
    assert not lvalue.startswith('None')
    if cpp_class.incomplete_type:
        raise CodeGenerationError("%s cannot be constructed (incomplete type)"
                                  % cpp_class.full_name)
    code_block.write_code(
        "%s = std::make_shared<%s>(%s);" % (lvalue, construct_type_name, parameters))


class CppClassSharedPtrParameter(CppClassParameterBase):
    "Class* handlers"
    CTYPES = []
    cpp_class = None #cppclass.CppClass('dummy') # CppClass instance
    DIRECTIONS = [Parameter.DIRECTION_IN,
                  Parameter.DIRECTION_OUT,
                  Parameter.DIRECTION_INOUT]
    SUPPORTS_TRANSFORMATIONS = False

    def __init__(self, ctype, name, direction=Parameter.DIRECTION_IN, is_const=False,
                 null_ok=False, default_value=None):
        """
        Type handler for a pointer-to-class parameter (MyClass*)

        :param ctype: C type, normally 'MyClass*'
        :param name: parameter name

        :param is_const: if true, the parameter has a const attached to the leftmost

        :param null_ok: if true, None is accepted and mapped into a C NULL pointer

        :param default_value: default parameter value (as C expression
            string); probably, the only default value that makes sense
            here is probably 'NULL'.

        .. note::

            Only arguments which are instances of C++ classes
            wrapped by PyBindGen can be used as custodians.
        """
        super(CppClassSharedPtrParameter, self).__init__(
            ctype, name, direction, is_const, default_value)
        self.null_ok = null_ok


    def convert_python_to_c(self, wrapper):
        "parses python args to get C++ value"
        assert isinstance(wrapper, ForwardWrapperBase)
        assert isinstance(self.cpp_class, CppClass)

        self.py_name = wrapper.declarations.declare_variable(
            self.cpp_class.pystruct+'*', self.name,
            initializer=(self.default_value and 'NULL' or None))

        value_ptr = wrapper.declarations.declare_variable(
            self.cpp_class.memory_policy.get_pointer_name(self.cpp_class.full_name), "%s_ptr" % self.name)

        if self.null_ok:
            num = wrapper.parse_params.add_parameter('O', ['&'+self.py_name], self.name, optional=bool(self.default_value))

            wrapper.before_call.write_error_check(

                "%s && ((PyObject *) %s != Py_None) && !PyObject_IsInstance((PyObject *) %s, (PyObject *) &%s)"
                % (self.py_name, self.py_name, self.py_name, self.cpp_class.pytypestruct),

                'PyErr_SetString(PyExc_TypeError, "Parameter %i must be of type %s");' % (num, self.cpp_class.name))

            wrapper.before_call.write_code("if (%(PYNAME)s) {\n"
                                           "    if ((PyObject *) %(PYNAME)s == Py_None)\n"
                                           "        %(VALUE)s = NULL;\n"
                                           "    else\n"
                                           "        %(VALUE)s = %(PYNAME)s->obj;\n"
                                           "} else {\n"
                                           "    %(VALUE)s = NULL;\n"
                                           "}" % dict(PYNAME=self.py_name, VALUE=value_ptr))

        else:

            wrapper.parse_params.add_parameter(
                'O!', ['&'+self.cpp_class.pytypestruct, '&'+self.py_name], self.name, optional=bool(self.default_value))
            wrapper.before_call.write_code("if (%s) { %s = %s->obj; }" % (self.py_name, value_ptr, self.py_name))

        wrapper.call_params.append(value_ptr)
        


    def convert_c_to_python(self, wrapper):
        """foo"""

        ## Value transformations
        value = self.transformation.untransform(
            self, wrapper.declarations, wrapper.after_call, self.value)

        ## declare wrapper variable
        py_name = wrapper.declarations.declare_variable(
            self.cpp_class.pystruct+'*', 'py_'+self.cpp_class.name)
        self.py_name = py_name

        def write_create_new_wrapper():
            """Code path that creates a new wrapper for the parameter"""

            ## Find out what Python wrapper to use, in case
            ## automatic_type_narrowing is active and we are not forced to
            ## make a copy of the object
            if self.cpp_class.automatic_type_narrowing:

                typeid_map_name = self.cpp_class.get_type_narrowing_root().typeid_map_name
                wrapper_type = wrapper.declarations.declare_variable(
                    'PyTypeObject*', 'wrapper_type', '0')
                wrapper.before_call.write_code(
                    '%s = %s.lookup_wrapper(typeid(*%s), &%s);'
                    % (wrapper_type, typeid_map_name, value, self.cpp_class.pytypestruct))
            else:
                wrapper_type = '&'+self.cpp_class.pytypestruct

            ## Create the Python wrapper object
            self.cpp_class.write_allocate_pystruct(wrapper.before_call, py_name, wrapper_type)
            self.py_name = py_name

            wrapper.before_call.write_code("%s->flags = PYBINDGEN_WRAPPER_FLAG_NONE;" % py_name)

            ## Assign the C++ value to the Python wrapper
            wrapper.before_call.write_code("%s->obj = %s;" % (py_name, value))

        if self.cpp_class.helper_class is None:
            try:
                self.cpp_class.wrapper_registry.write_lookup_wrapper(
                    wrapper.before_call, self.cpp_class.pystruct, py_name, value)
            except NotSupportedError:
                write_create_new_wrapper()
                self.cpp_class.wrapper_registry.write_register_new_wrapper(wrapper.before_call, py_name,
                                                                           "%s->obj" % py_name)
            else:
                wrapper.before_call.write_code("if (%s == NULL)\n{" % py_name)
                wrapper.before_call.indent()
                write_create_new_wrapper()
                self.cpp_class.wrapper_registry.write_register_new_wrapper(wrapper.before_call, py_name,
                                                                           "%s->obj" % py_name)
                wrapper.before_call.unindent()
                wrapper.before_call.write_code('}')
            wrapper.build_params.add_parameter("N", [py_name])
        else:
            wrapper.before_call.write_code("if (typeid(*(%s)).name() == typeid(%s).name())\n{"
                                          % (value, self.cpp_class.helper_class.name))
            wrapper.before_call.indent()

            if self.type_traits.target_is_const:
                wrapper.before_call.write_code(
                    "%s = (%s*) (((%s*) ((%s*) %s))->m_pyself);"
                    % (py_name, self.cpp_class.pystruct,
                       self.cpp_class.helper_class.name, self.cpp_class.full_name, value))
                wrapper.before_call.write_code("%s->obj =  (%s*) (%s);" %
                                               (py_name, self.cpp_class.full_name, value))
            else:
                wrapper.before_call.write_code(
                    "%s = (%s*) (((%s*) %s)->m_pyself);"
                    % (py_name, self.cpp_class.pystruct,
                       self.cpp_class.helper_class.name, value))
                wrapper.before_call.write_code("%s->obj = %s;" % (py_name, value))
            wrapper.before_call.write_code("Py_INCREF(%s);" % py_name)
            wrapper.before_call.unindent()
            wrapper.before_call.write_code("} else {")
            wrapper.before_call.indent()

            try:
                self.cpp_class.wrapper_registry.write_lookup_wrapper(
                    wrapper.before_call, self.cpp_class.pystruct, py_name, value)
            except NotSupportedError:
                write_create_new_wrapper()
                self.cpp_class.wrapper_registry.write_register_new_wrapper(
                    wrapper.before_call, py_name, "%s->obj" % py_name)
            else:
                wrapper.before_call.write_code("if (%s == NULL)\n{" % py_name)
                wrapper.before_call.indent()
                write_create_new_wrapper()
                self.cpp_class.wrapper_registry.write_register_new_wrapper(wrapper.before_call, py_name,
                                                                           "%s->obj" % py_name)
                wrapper.before_call.unindent()
                wrapper.before_call.write_code('}') # closes if (%s == NULL)

            wrapper.before_call.unindent()
            wrapper.before_call.write_code("}") # closes if (typeid(*(%s)) == typeid(%s))\n{
            wrapper.build_params.add_parameter("N", [py_name])
            



class CppClassSharedPtrReturnValue(CppClassReturnValueBase):
    "Class* return handler"
    CTYPES = []
    SUPPORTS_TRANSFORMATIONS = True
    cpp_class = None #cppclass.CppClass('dummy') # CppClass instance

    def __init__(self, ctype, is_const=False):
        """
        :param ctype: C type, normally 'MyClass*'
        """
        super(CppClassSharedPtrReturnValue, self).__init__(ctype, is_const=is_const)

    def get_c_error_return(self): # only used in reverse wrappers
        """See ReturnValue.get_c_error_return"""
        return "return NULL;"

    def convert_c_to_python(self, wrapper):
        """See ReturnValue.convert_c_to_python"""

        ## Value transformations
        value = self.transformation.untransform(
            self, wrapper.declarations, wrapper.after_call, self.value)
        
        # if value is NULL, return None
        wrapper.after_call.write_code("if (!(%s)) {\n"
                                      "    Py_INCREF(Py_None);\n"
                                      "    return Py_None;\n"
                                      "}" % value)

        ## declare wrapper variable
        py_name = wrapper.declarations.declare_variable(
            self.cpp_class.pystruct+'*', 'py_'+self.cpp_class.name)
        self.py_name = py_name

        common_shared_object_return(value, py_name, self.cpp_class, wrapper.after_call,
                                    self.type_traits, caller_owns_return=True,
                                    reference_existing_object=False,
                                    type_is_pointer=True)

        # return the value
        wrapper.build_params.add_parameter("N", [py_name], prepend=True)
    

    def convert_python_to_c(self, wrapper):
        """See ReturnValue.convert_python_to_c"""
        name = wrapper.declarations.declare_variable(
            self.cpp_class.pystruct+'*', "tmp_%s" % self.cpp_class.name)
        wrapper.parse_params.add_parameter(
            'O!', ['&'+self.cpp_class.pytypestruct, '&'+name])

        value = self.transformation.transform(
            self, wrapper.declarations, wrapper.after_call, "%s->obj" % name)

        # caller gets a shared pointer
        wrapper.after_call.write_code("%s = %s;" % (self.value, value))

