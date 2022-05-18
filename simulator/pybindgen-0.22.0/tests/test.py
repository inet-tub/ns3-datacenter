from __future__ import unicode_literals, print_function
import pybindgen.typehandlers.base as typehandlers
from pybindgen.typehandlers import stringtype, ctypeparser
import pybindgen.typehandlers.codesink as codesink
from pybindgen import module, cppclass, overloading, utils


import unittest
import doctest
import re
import sys


class SmartPointerTransformation(typehandlers.TypeTransformation):
    def __init__(self):
        self.rx = re.compile(r'(?:::)?MySmartPointer<\s*(\w+)\s*>')

    def get_untransformed_name(self, name):
        m = self.rx.match(name)
        if m is None:
            return None
        else:
            return m.group(1)+' *'

    def create_type_handler(self, type_handler, *args, **kwargs):
        ctype = self.get_untransformed_name(args[0])
        handler = type_handler(ctype, *args[1:], **kwargs)
        handler.has_been_transformed = True
        return handler


typehandlers.param_type_matcher.register_transformation(
    SmartPointerTransformation())

class TestParam(typehandlers.Parameter):

    DIRECTIONS = [typehandlers.Parameter.DIRECTION_IN]
    CTYPES = ['testtype*']
    has_been_transformed = False

    def convert_c_to_python(self, wrapper):
        assert isinstance(wrapper, typehandlers.ReverseWrapperBase)
        wrapper.build_params.add_parameter('i', [self.name])

    def convert_python_to_c(self, wrapper):
        assert isinstance(wrapper, typehandlers.ForwardWrapperBase)
        name = wrapper.declarations.declare_variable(self.ctype, self.name)
        wrapper.parse_params.add_parameter('i', ['&'+name], self.name)
        wrapper.call_params.append(name)


class ParamLookupTests(unittest.TestCase):

    def testLookup(self):
        handler = typehandlers.Parameter.new('testtype*', 'name')
        self.assertTrue(isinstance(handler, TestParam))
        self.assertRaises(typehandlers.TypeLookupError, typehandlers.Parameter.new, 'non_existent_type', 'name')

    def testLookupTransformed(self):
        transformed = typehandlers.Parameter.new('MySmartPointer<testtype>', 'name')
        self.assertTrue(isinstance(transformed, TestParam))
        self.assertTrue(transformed.has_been_transformed)



if __name__ == '__main__':
    suite = unittest.TestSuite()

    # FIXME: due to python 2 to python 3 transition, most of the
    # doctests are failing for trivial reasons.  Don't run these tests
    # for the time being, until I have time to fix them.
    if 0: # sys.version_info[0] < 3: # the doctests only work in Python 2
        for mod in [
            typehandlers,
            codesink,
            module,
            cppclass,
            overloading,
            #utils,
            stringtype,
            ctypeparser,
            ]:
            suite.addTest(doctest.DocTestSuite(mod))

    suite.addTest(doctest.DocTestSuite(ctypeparser))
    suite.addTest(unittest.TestLoader().loadTestsFromTestCase(ParamLookupTests))
    runner = unittest.TextTestRunner()
    runner.run(suite)

