from .cppattribute import PyGetter, PySetter
from .typehandlers import codesink
from . import settings
from . import utils

class CppCustomInstanceAttributeGetter(PyGetter):
    '''
    A getter for a C++ instance attribute.
    '''
    def __init__(self, value_type, class_, attribute_name, getter, template_parameters = []):
        """
        :param value_type: a ReturnValue object handling the value type;
        :param class_: the class (CppClass object)
        :param attribute_name: name of attribute
        :param getter: None, or name of a method of the class used to get the value
        """
        super(CppCustomInstanceAttributeGetter, self).__init__(
            value_type, [], "return NULL;", "return NULL;", no_c_retval=True)
        self.class_ = class_
        self.attribute_name = attribute_name
        self.getter = getter
        self.c_function_name = "_wrap_%s__get_%s" % (self.class_.pystruct,
                                                     self.attribute_name)
        if template_parameters == []:
            value_type.value = "%s(*((%s *)self)->obj)" % (self.getter, self.class_.pystruct)
        else:
            value_type.value = "%s<%s" % (self.getter, template_parameters[0])
            if len(template_parameters) > 1:
                for x in template_parameters[1:]:
                    value_type.value += ", %s " % x
            value_type.value += ">(*((%s *)self)->obj)" % self.class_.pystruct

    def generate_call(self):
        "virtual method implementation; do not call"
        pass

    def generate(self, code_sink):
        """
        :param code_sink: a CodeSink instance that will receive the generated code
        """
        tmp_sink = codesink.MemoryCodeSink()
        self.generate_body(tmp_sink)
        code_sink.writeln("static PyObject* %s(%s *self, void * PYBINDGEN_UNUSED(closure))"
                          % (self.c_function_name, self.class_.pystruct))
        code_sink.writeln('{')
        code_sink.indent()
        tmp_sink.flush_to(code_sink)
        code_sink.unindent()
        code_sink.writeln('}')

class CppCustomInstanceAttributeSetter(PySetter):
    '''
    A setter for a C++ instance attribute.
    '''
    def __init__(self, value_type, class_, attribute_name, setter=None,
                 template_parameters = []):
        """
        :param value_type: a ReturnValue object handling the value type;
        :param class_: the class (CppClass object)
        :param attribute_name: name of attribute
        :param setter: None, or name of a method of the class used to set the value
        """
        super(CppCustomInstanceAttributeSetter, self).__init__(
            value_type, [], "return -1;")
        self.class_ = class_
        self.attribute_name = attribute_name
        self.setter = setter
        self.template_parameters = template_parameters
        self.c_function_name = "_wrap_%s__set_%s" % (self.class_.pystruct,
                                                     self.attribute_name)

    def generate(self, code_sink):
        """
        :param code_sink: a CodeSink instance that will receive the generated code
        """

        self.declarations.declare_variable('PyObject*', 'py_retval')
        self.before_call.write_code(
            'py_retval = Py_BuildValue((char *) "(O)", value);')
        self.before_call.add_cleanup_code('Py_DECREF(py_retval);')

        if self.setter is not None:
            ## if we have a setter method, redirect the value to a temporary variable
            if not self.return_value.REQUIRES_ASSIGNMENT_CONSTRUCTOR:
                value_var = self.declarations.declare_variable(self.return_value.ctype, 'tmp_value')
            else:
                value_var = self.declarations.reserve_variable('tmp_value')
            self.return_value.value = value_var
        else:
            ## else the value is written directly to a C++ instance attribute
            self.return_value.value = "self->obj->%s" % self.attribute_name
            self.return_value.REQUIRES_ASSIGNMENT_CONSTRUCTOR = False

        self.return_value.convert_python_to_c(self)

        parse_tuple_params = ['py_retval']
        params = self.parse_params.get_parameters()
        assert params[0][0] == '"'
        params[0] = '(char *) ' + params[0]
        parse_tuple_params.extend(params)
        self.before_call.write_error_check('!PyArg_ParseTuple(%s)' %
                                           (', '.join(parse_tuple_params),))

        if self.setter is not None:
            ## if we have a setter method, now is the time to call it
            if len(self.template_parameters) == 0:
                code = "%s(*((%s *)self)->obj, %s);" % (self.setter, self.class_.pystruct, value_var)
            else:
                code = "%s<%s" % (self.setter, self.template_parameters[0])
                if len(self.template_parameters) > 1:
                    for x in self.template_parameters[1:]:
                        code += ", %s " % x
                code += ">(*((%s *)self)->obj, %s);" % (self.class_.pystruct, value_var)
            self.after_call.write_code(code)

        ## cleanup and return
        self.after_call.write_cleanup()
        self.after_call.write_code('return 0;')

        ## now generate the function itself
        code_sink.writeln("static int %s(%s *self, PyObject *value, void * PYBINDGEN_UNUSED(closure))"
                          % (self.c_function_name, self.class_.pystruct))
        code_sink.writeln('{')
        code_sink.indent()

        self.declarations.get_code_sink().flush_to(code_sink)
        code_sink.writeln()
        self.before_call.sink.flush_to(code_sink)
        self.after_call.sink.flush_to(code_sink)

        code_sink.unindent()
        code_sink.writeln('}')
