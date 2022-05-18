#! /usr/bin/env python
from __future__ import unicode_literals, print_function

import sys
import re

import pybindgen
import pybindgen.utils
from pybindgen.typehandlers import base as typehandlers
from pybindgen import ReturnValue, Parameter, Module, Function, FileCodeSink
from pybindgen import CppMethod, CppConstructor, CppClass, Enum
from pybindgen.function import CustomFunctionWrapper
from pybindgen.cppmethod import CustomCppMethodWrapper
from pybindgen import cppclass

from pybindgen import param, retval

import foomodulegen_common



def my_module_gen(out_file):

    mod = Module('foo')
    foomodulegen_common.customize_module_pre(mod)

    mod.add_include ('"foo.h"')

    mod.add_function('TypeNameGet',
                     'std::string',
                     [],
                     custom_name='IntegerTypeNameGet', template_parameters=['int'])

    Foo = mod.add_class('Foo', automatic_type_narrowing=True)

    Foo.add_static_attribute('instance_count', ReturnValue.new('int'))
    Foo.add_constructor([Parameter.new('std::string', 'datum')])
    Foo.add_constructor([])
    Foo.add_constructor([Parameter.new('const Foo&', 'foo')])
    Foo.add_method('get_datum', ReturnValue.new('const std::string'), [])
    Foo.add_method('is_initialized', ReturnValue.new('bool'), [], is_const=True)
    Foo.add_output_stream_operator()
    Foo.add_method('add_sub', ReturnValue.new('int'), [
            Parameter.new('int', 'a'),
            Parameter.new('int', 'b', default_value='3'),
            Parameter.new('bool', 'subtract', default_value='false')
            ], is_static=True)
    Foo.add_custom_instance_attribute("is_unique", "bool", getter="is_unique", is_const=True)

    Zoo = mod.add_class('Zoo', automatic_type_narrowing=True)
    Zoo.add_constructor([Parameter.new('std::string', 'datum')])
    Zoo.add_constructor([])
    Zoo.add_method('get_datum', ReturnValue.new('std::string'), [])
    Zoo.implicitly_converts_to(Foo)

    Foobar = mod.add_class('Foobar', allow_subclassing=True)
    Foobar.add_static_attribute('instance_count', ReturnValue.new('int'))


    Bar = mod.add_class('Bar', parent=Foo)
    Bar.inherit_default_constructors()
    ## a static method..
    Bar.add_method('Hooray', ReturnValue.new('std::string'), [], is_static=True)

    ## to test RTTI with a hidden subclass
    mod.add_function('get_hidden_subclass_pointer',
                     ReturnValue.new('Foo*', caller_owns_return=True),
                     [])


    ## Zbr is a reference counted class
    Zbr = mod.add_class('Zbr', memory_policy=cppclass.ReferenceCountingMethodsPolicy(
            incref_method='Ref', decref_method='Unref', peekref_method="GetReferenceCount"),
                        allow_subclassing=True)

    def helper_class_hook(helper_class):
        helper_class.add_custom_method(
            declaration="static int custom_method_added_by_a_hook(int x);",
            body="""
int %s::custom_method_added_by_a_hook(int x)
{
  return x + 1;
}
""" % helper_class.name)
        helper_class.add_post_generation_code("// this comment was written by a helper class hook function")
    Zbr.add_helper_class_hook(helper_class_hook)

    Zbr.add_constructor([])
    Zbr.add_constructor([Parameter.new('std::string', 'datum')])
    Zbr.add_method('get_datum', ReturnValue.new('std::string'), [])
    Zbr.add_method('get_int', ReturnValue.new('int'), [Parameter.new('int', 'x')],
                             is_virtual=True)
    Zbr.add_static_attribute('instance_count', ReturnValue.new('int'))
    Zbr.add_method('get_value', ReturnValue.new('int'), [Parameter.new('int*', 'x', direction=Parameter.DIRECTION_OUT)])

    mod.add_function('store_zbr', None,
                     [Parameter.new('Zbr*', 'zbr', transfer_ownership=True)])
    mod.add_function('invoke_zbr', ReturnValue.new('int'), [Parameter.new('int', 'x')])
    mod.add_function('delete_stored_zbr', None, [])


    mod.add_function('print_something', ReturnValue.new('int'),
                     [Parameter.new('const char*', 'message')],
                     deprecated=True)
    mod.add_function('print_something_else', ReturnValue.new('int'),
                     [Parameter.new('const char*', 'message2')])

    ## test overloaded functions
    mod.add_function('get_int_from_string', ReturnValue.new('int'),
                     [Parameter.new('const char*', 'from_string'),
                      Parameter.new('int', 'multiplier', default_value='1')], custom_name="get_int")
    mod.add_function('get_int_from_float', ReturnValue.new('int'),
                     [Parameter.new('double', 'from_float'),
                      Parameter.new('int', 'multiplier', default_value='1')],
                     custom_name="get_int")

    ## test free_after_copy.
    mod.add_function('return_c_string_to_be_freed',
                     ReturnValue.new('char *', free_after_copy=True),
                     [Parameter.new('int', 'size')])
    mod.add_function('return_c_string_to_not_be_freed',
                     ReturnValue.new('char *', free_after_copy=False),
                     [Parameter.new('int', 'size')])

    ToBeFreed = mod.add_class('ToBeFreed')
    ToBeFreed.add_constructor([Parameter.new('int', 'size')])
    ToBeFreed.add_copy_constructor()
    ToBeFreed.add_method('value', ReturnValue.new('char *'), [])
    mod.add_function('return_class_to_be_freed',
                     ReturnValue.new('ToBeFreed *', free_after_copy=True),
                     [Parameter.new('int', 'size')])
    mod.add_function('return_class_to_not_be_freed',
                     ReturnValue.new('ToBeFreed *', free_after_copy=False),
                     [Parameter.new('int', 'size')])

    SomeObject = mod.add_class('SomeObject', allow_subclassing=True)

    SomeObject.add_instance_attribute('foo', ReturnValue.new('Foo'),
                                      getter='get_foo_value',
                                      setter='set_foo_value')
    SomeObject.add_instance_attribute('m_prefix', ReturnValue.new('std::string'))
    SomeObject.add_static_attribute('staticData', ReturnValue.new('std::string'))

    SomeObject.add_static_attribute('instance_count', ReturnValue.new('int'))

    SomeObject.add_method('add_prefix', ReturnValue.new('int'),
                          [Parameter.new('std::string&', 'message',
                                         direction=Parameter.DIRECTION_INOUT)])
    SomeObject.add_constructor([Parameter.new('std::string', 'prefix')])
    SomeObject.add_constructor([Parameter.new('int', 'prefix_len')])

    SomeObject.add_method('operator()', ReturnValue.new('int'),
                          [Parameter.new('std::string&', 'message',
                                         direction=Parameter.DIRECTION_INOUT)],
                          custom_name='__call__')


    # --- some virtual methods ---
    SomeObject.add_method('get_prefix', ReturnValue.new('std::string'), [],
                          is_virtual=True, is_const=True)

    SomeObject.add_method('get_prefix_with_foo_value', ReturnValue.new('std::string'),
                          [Parameter.new('Foo', 'foo')],
                          is_virtual=True, is_const=True)

    SomeObject.add_method('get_prefix_with_foo_ref',
                          ReturnValue.new('std::string'),
                          [Parameter.new('const Foo&', 'foo',
                           direction=Parameter.DIRECTION_INOUT)],
                          is_virtual=True, is_const=True)

    SomeObject.add_method('get_prefix_with_foo_ptr',
                          ReturnValue.new('std::string'),
                          [Parameter.new('const Foo*', 'foo', transfer_ownership=False)],
                          is_virtual=True, is_const=True)

    ## overloaded virtual methods
    SomeObject.add_method('get_something',
                          ReturnValue.new('std::string'),
                          [],
                          is_virtual=True, is_const=True)
    SomeObject.add_method('get_something',
                          ReturnValue.new('std::string'),
                          [Parameter.new('int', 'x')],
                          is_virtual=True, is_const=True)

    SomeObject.add_method('set_pyobject', None,
                          [Parameter.new('PyObject*', 'pyobject', transfer_ownership=False)],
                          is_virtual=True)
    SomeObject.add_method('get_pyobject',
                          ReturnValue.new('PyObject*', caller_owns_return=True),
                          [],
                          is_virtual=True)

    ## add a function that appears as a method of an object
    SomeObject.add_function_as_method('some_object_get_something_prefixed',
                                      ReturnValue.new('std::string'),
                                      [Parameter.new('const SomeObject*', 'obj', transfer_ownership=False),
                                       Parameter.new('std::string', 'something')],
                                      custom_name='get_something_prefixed')

    ## add a function that appears as a method of an object
    SomeObject.add_function_as_method('some_object_val_get_something_prefixed',
                                      ReturnValue.new('std::string'),
                                      [Parameter.new('SomeObject', 'obj'),
                                       Parameter.new('std::string', 'something')],
                                      custom_name='val_get_something_prefixed')

    ## add a function that appears as a method of an object
    SomeObject.add_function_as_method('some_object_ref_get_something_prefixed',
                                      ReturnValue.new('std::string'),
                                      [Parameter.new('const SomeObject&', 'obj'),
                                       Parameter.new('std::string', 'something')],
                                      custom_name='ref_get_something_prefixed')

    # ---
    SomeObject.add_method('call_get_prefix', ReturnValue.new('std::string'), [])

    SomeObject.add_method('set_foo_value', None, [Parameter.new('Foo', 'foo')])
    SomeObject.add_method('get_foo_value', ReturnValue.new('Foo'), [])

    SomeObject.add_method('set_foo_ptr', ReturnValue.new('void'),
                          [Parameter.new('Foo*', 'foo', transfer_ownership=True)])
    SomeObject.add_method('set_foo_shared_ptr', ReturnValue.new('void'),
                          [Parameter.new('Foo*', 'foo', transfer_ownership=False)])

    SomeObject.add_method('get_foo_shared_ptr', ReturnValue.new('const Foo*', caller_owns_return=False), [])
    SomeObject.add_method('get_foo_ptr', ReturnValue.new('Foo*', caller_owns_return=True), [])

    SomeObject.add_method('set_foo_by_ref', ReturnValue.new('void'),
                          [Parameter.new('Foo&', 'foo', direction=Parameter.DIRECTION_IN)])
    SomeObject.add_method('get_foo_by_ref', ReturnValue.new('void'),
                          [Parameter.new('Foo&', 'foo', direction=Parameter.DIRECTION_OUT)])

    ## custodian/ward tests
    SomeObject.add_method('get_foobar_with_self_as_custodian',
                          ReturnValue.new('Foobar*', custodian=0, reference_existing_object=True), [])
    SomeObject.add_method('get_foobar_with_other_as_custodian',
                          ReturnValue.new('Foobar*', custodian=1, reference_existing_object=True),
                          [Parameter.new('SomeObject*', 'other', transfer_ownership=False)])
    SomeObject.add_method('set_foobar_with_self_as_custodian', ReturnValue.new('void'),
                          [Parameter.new('Foobar*', 'foobar',
                                         transfer_ownership=True, custodian=0)])

    mod.add_function('get_foobar_with_other_as_custodian',
                     ReturnValue.new('Foobar*', custodian=1, reference_existing_object=True),
                     [Parameter.new('SomeObject*', 'other', transfer_ownership=False)])

    mod.add_function('create_new_foobar', ReturnValue.new('Foobar*', caller_owns_return=True), [])

    mod.add_function('set_foobar_with_other_as_custodian', ReturnValue.new('void'),
                     [Parameter.new('Foobar*', 'foobar', transfer_ownership=True, custodian=2),
                      Parameter.new('SomeObject*', 'other', transfer_ownership=False)])

    mod.add_function('set_foobar_with_return_as_custodian',
                     ReturnValue.new('SomeObject*', caller_owns_return=True),
                     [Parameter.new('Foobar*', 'foobar',
                                    transfer_ownership=True, custodian=-1)])


    ## get/set recfcounted object Zbr
    SomeObject.add_method('get_zbr', ReturnValue.new('Zbr*', caller_owns_return=True), [])
    SomeObject.add_method('get_internal_zbr', ReturnValue.new('Zbr*', caller_owns_return=True), [])
    SomeObject.add_method('peek_zbr', ReturnValue.new('Zbr*', caller_owns_return=False), [])
    SomeObject.add_method('set_zbr_transfer', ReturnValue.new('void'),
                          [Parameter.new('Zbr*', 'zbr', transfer_ownership=True)])
    SomeObject.add_method('set_zbr_shared', ReturnValue.new('void'),
                          [Parameter.new('Zbr*', 'zbr', transfer_ownership=False)])

    ## methods with transformed types
    SomeObject.add_method('set_zbr_pholder', ReturnValue.new('void'),
                          [Parameter.new('PointerHolder<Zbr>', 'zbr')])
    SomeObject.add_method('get_zbr_pholder',
                          ReturnValue.new('PointerHolder<Zbr>'), [])

    ## test overloaded methods
    SomeObject.add_method('get_int', ReturnValue.new('int'),
                          [Parameter.new('const char*', 'from_string')],
                          custom_name="get_int")
    SomeObject.add_method('get_int', ReturnValue.new('int'),
                          [Parameter.new('double', 'from_float')],
                          custom_name="get_int")

    # Bug #508577
    SomeObject.add_method('protected_method_that_is_not_virtual',
                          ReturnValue.new('std::string'),
                          [Parameter.new('std::string', 'arg')],
                          is_const=True, visibility='protected')

    SomeObject.add_method('method_returning_cstring', ReturnValue.new('const char *'),
                          [], is_virtual=True, is_const=True)


    mod.add_function('store_some_object', ReturnValue.new('void'),
                     [Parameter.new('SomeObject*', 'obj', transfer_ownership=True)])
    mod.add_function('invoke_some_object_get_prefix', ReturnValue.new('std::string'),
                     [])
    mod.add_function('take_some_object', ReturnValue.new('SomeObject*', caller_owns_return=True), [])
    mod.add_function('delete_some_object', ReturnValue.new('void'), [])

    xpto = mod.add_cpp_namespace("xpto")
    xpto.add_function('some_function', ReturnValue.new('std::string'), [])

    ## enums..
    xpto.add_enum('FooType', ['FOO_TYPE_AAA', 'FOO_TYPE_BBB', 'FOO_TYPE_CCC'])
    xpto.add_function('get_foo_type', ReturnValue.new('FooType'), [])
    xpto.add_function('set_foo_type', ReturnValue.new('void'), [Parameter.new("FooType", 'type')])
    xpto.add_function('set_foo_type_inout', ReturnValue.new('void'),
                      [Parameter.new("FooType&", 'type', direction=Parameter.DIRECTION_INOUT)])
    xpto.add_function('set_foo_type_ptr', ReturnValue.new('void'),
                      [Parameter.new("FooType*", 'type', direction=Parameter.DIRECTION_INOUT)])


    xpto_SomeClass = xpto.add_class('SomeClass', docstring="This is the docstring for SomeClass")
    xpto_SomeClass.add_constructor([])

    xpto.add_typedef(Foo, 'FooXpto')
    xpto.add_function('get_foo_datum', 'std::string', [Parameter.new('const xpto::FooXpto&', 'foo')])

    typehandlers.add_type_alias('uint32_t', 'xpto::FlowId')
    xpto.add_function('get_flow_id', 'xpto::FlowId', [Parameter.new('xpto::FlowId', 'flowId')])

    # bug #798383
    XptoClass = xpto.add_struct('XptoClass')
    XptoClass.add_method("GetSomeClass", retval("xpto::SomeClass*", caller_owns_return=True), [])


    ## ---- some implicity conversion APIs
    mod.add_function('function_that_takes_foo', ReturnValue.new('void'),
                               [Parameter.new('Foo', 'foo')])
    mod.add_function('function_that_returns_foo', ReturnValue.new('Foo'), [])

    cls = mod.add_class('ClassThatTakesFoo')
    cls.add_constructor([Parameter.new('Foo', 'foo')])
    cls.add_method('get_foo', ReturnValue.new('Foo'), [])

    cls = mod.add_class('SingletonClass', is_singleton=True)
    cls.add_method('GetInstance', ReturnValue.new('SingletonClass*', caller_owns_return=True),
                   [], is_static=True)


    ## A class that has no public default constructor...
    cls = mod.add_class('InterfaceId', is_singleton=True)
    ## A function that returns such a class...
    mod.add_function('make_interface_id', ReturnValue.new('InterfaceId'), [])


    ## A class the cannot be constructed; this will cause late CodeGenerationError's
    cls = mod.add_class('CannotBeConstructed')
    cls.set_cannot_be_constructed("no reason")
    cls.add_method('get_value', ReturnValue.new('CannotBeConstructed'),
                             [], is_static=True)
    cls.add_method('get_ptr', ReturnValue.new('CannotBeConstructed*', caller_owns_return=True),
                   [], is_static=True)
    mod.add_function('get_cannot_be_constructed_value',
                     ReturnValue.new('CannotBeConstructed'),
                     [])
    mod.add_function('get_cannot_be_constructed_ptr',
                     ReturnValue.new('CannotBeConstructed*', caller_owns_return=True),
                     [])


    ## A nested class
    #NestedClass = mod.add_class('NestedClass', automatic_type_narrowing=True, outer_class=SomeObject)
    NestedClass = SomeObject.add_class('NestedClass', automatic_type_narrowing=True)
    NestedClass.add_static_attribute('instance_count', ReturnValue.new('int'))
    NestedClass.add_constructor([Parameter.new('std::string', 'datum')])
    NestedClass.add_constructor([])
    NestedClass.add_method('get_datum', ReturnValue.new('std::string'), [])

    ## A nested enum..
    #mod.add_enum('NestedEnum', ['FOO_TYPE_AAA', 'FOO_TYPE_BBB', 'FOO_TYPE_CCC'], outer_class=SomeObject)
    SomeObject.add_enum('NestedEnum', ['FOO_TYPE_AAA', 'FOO_TYPE_BBB', 'FOO_TYPE_CCC'])

    ## anonymous enum
    SomeObject.add_enum('', ['CONSTANT_A', 'CONSTANT_B', 'CONSTANT_C'])


    AbstractBaseClass2 = mod.add_class('AbstractBaseClass2', allow_subclassing=True)

    AbstractBaseClass2.add_method('invoke_private_virtual', ReturnValue.new('int'),
                                  [Parameter.new('int', 'x')], is_const=True)
    AbstractBaseClass2.add_method('invoke_protected_virtual', ReturnValue.new('int'),
                                  [Parameter.new('int', 'x')], is_const=True)
    AbstractBaseClass2.add_method('invoke_protected_pure_virtual', ReturnValue.new('int'),
                                  [Parameter.new('int', 'x')], is_const=True)
    AbstractBaseClass2.add_constructor([], visibility='protected')

    AbstractBaseClass2.add_method('protected_virtual',
                                  ReturnValue.new('int'),
                                  [Parameter.new('int', 'x')],
                                  is_virtual=True, visibility='protected', is_const=True)
    AbstractBaseClass2.add_method('protected_pure_virtual',
                                  ReturnValue.new('int'),
                                  [Parameter.new('int', 'x')],
                                  is_virtual=True, is_pure_virtual=True, visibility='protected', is_const=True)

    AbstractBaseClass2.add_method('private_virtual',
                                  ReturnValue.new('int'), [Parameter.new('int', 'x')],
                                  is_virtual=True, is_pure_virtual=True, visibility='private', is_const=True)




    AbstractXpto = mod.add_class('AbstractXpto', allow_subclassing=True)
    AbstractXpto.add_method('something', ReturnValue.new('void'),
                            [Parameter.new('int', 'x')], is_const=True,
                            is_virtual=True, is_pure_virtual=True)
    AbstractXpto.add_constructor([])

    AbstractXptoImpl = mod.add_class('AbstractXptoImpl', parent=AbstractXpto)
    AbstractXptoImpl.add_method('something', ReturnValue.new('void'),
                                [Parameter.new('int', 'x')], is_const=True,
                                is_virtual=True, is_pure_virtual=False)
    AbstractXptoImpl.add_constructor([])

    Word = mod.add_class('Word')
    Word.add_instance_attribute('low', 'uint8_t', is_const=False)
    Word.add_instance_attribute('high', 'uint8_t', is_const=False)
    Word.add_instance_attribute('word', 'uint16_t', is_const=False)
    Word.add_constructor([])


    mod.add_function('matrix_sum_of_elements',
                     ReturnValue.new('float'),
                     [Parameter.new("float*", 'matrix', direction=Parameter.DIRECTION_IN, array_length=6)])

    mod.add_function('matrix_identity_new',
                     ReturnValue.new('void'),
                     [Parameter.new("float*", 'matrix', direction=Parameter.DIRECTION_OUT, array_length=6)])

    top_ns = mod.add_cpp_namespace('TopNs')
    outer_base = top_ns.add_class('OuterBase')
    bottom_ns = top_ns.add_cpp_namespace('PrefixBottomNs')
    inner = bottom_ns.add_class('PrefixInner',parent=outer_base)
    inner.add_constructor([])
    inner.add_method('Do', 'void', [])


    Socket = mod.add_class('Socket', allow_subclassing=True)
    Socket.add_constructor([])
    Socket.add_method('Bind', ReturnValue.new('int'), [], is_virtual=True)
    Socket.add_method('Bind', ReturnValue.new('int'), [Parameter.new('int', 'address')], is_virtual=True)

    UdpSocket = mod.add_class('UdpSocket', parent=Socket)
    UdpSocket.add_constructor([])
    UdpSocket.add_method('Bind', ReturnValue.new('int'), [], is_virtual=True)

    simple_struct_t = mod.add_struct('simple_struct_t')
    simple_struct_t.add_instance_attribute('xpto', 'int')


    # containers...
    mod.add_container('SimpleStructList', ReturnValue.new('simple_struct_t'), 'list')
    mod.add_function('get_simple_list', ReturnValue.new('SimpleStructList'), [])
    mod.add_function('set_simple_list', 'int', [Parameter.new('SimpleStructList', 'list')])

    mod.add_container('std::set<float>', 'float', 'set')

    TestContainer = mod.add_class('TestContainer', allow_subclassing=True)
    TestContainer.add_constructor([])
    TestContainer.add_instance_attribute('m_floatSet', 'std::set<float>')
    TestContainer.add_method('get_simple_list', ReturnValue.new('SimpleStructList'), [], is_virtual=True)
    TestContainer.add_method('set_simple_list', 'int', [Parameter.new('SimpleStructList', 'list')], is_virtual=True)
    TestContainer.add_method('set_simple_list_by_ref', 'int', [Parameter.new('SimpleStructList&', 'inout_list',
                                                                             direction=Parameter.DIRECTION_INOUT)],
                             is_virtual=True)

    mod.add_container('std::vector<simple_struct_t>', ReturnValue.new('simple_struct_t'), 'vector')
    TestContainer.add_method('get_simple_vec', ReturnValue.new('std::vector<simple_struct_t>'), [], is_virtual=True)
    TestContainer.add_method('set_simple_vec', 'int', [Parameter.new('std::vector<simple_struct_t>', 'vec')], is_virtual=True)

    mod.add_container('std::vector<std::string>', 'std::string', 'vector')
    TestContainer.add_method('get_vec', 'void', [Parameter.new('std::vector<std::string> &', 'outVec',
                                                               direction=Parameter.DIRECTION_OUT)])

    TestContainer.add_method('set_vec_ptr', 'void', [Parameter.new('std::vector<std::string>*', 'inVec',
                                                                   direction=Parameter.DIRECTION_IN, transfer_ownership=True)])
    TestContainer.add_method('get_vec_ptr', 'void', [Parameter.new('std::vector<std::string>*', 'outVec',
                                                                   direction=Parameter.DIRECTION_OUT)])


    mod.add_container('std::map<std::string, simple_struct_t>',
                      (ReturnValue.new('std::string'), ReturnValue.new('simple_struct_t')),
                      'map')
    TestContainer.add_method('get_simple_map', ReturnValue.new('std::map<std::string, simple_struct_t>'), [], is_virtual=True)
    TestContainer.add_method('set_simple_map', 'int', [Parameter.new('std::map<std::string, simple_struct_t>', 'map')], is_virtual=True)

    mod.add_container('SimpleStructUnorderedMap', ('std::string', 'simple_struct_t'), container_type='map')
    typehandlers.add_type_alias('std::unordered_map< std::string, simple_struct_t >', 'SimpleStructUnorderedMap')
    typehandlers.add_type_alias('std::unordered_map< std::string, simple_struct_t >*', 'SimpleStructUnorderedMap*')
    typehandlers.add_type_alias('std::unordered_map< std::string, simple_struct_t >&', 'SimpleStructUnorderedMap&')

    TestContainer.add_method('get_simple_unordered_map', ReturnValue.new('std::unordered_map<std::string, simple_struct_t>'), [], is_virtual=True)
    TestContainer.add_method('set_simple_unordered_map', 'int', [Parameter.new('std::unordered_map<std::string, simple_struct_t>', 'map')], is_virtual=True)


    Tupl = mod.add_class('Tupl')
    Tupl.add_binary_comparison_operator('<')
    Tupl.add_binary_comparison_operator('<=')
    Tupl.add_binary_comparison_operator('>=')
    Tupl.add_binary_comparison_operator('>')
    Tupl.add_binary_comparison_operator('==')
    Tupl.add_binary_comparison_operator('!=')
    Tupl.add_binary_numeric_operator('+')
    Tupl.add_binary_numeric_operator('-')
    Tupl.add_binary_numeric_operator('*')
    Tupl.add_binary_numeric_operator('/')
    Tupl.add_instance_attribute('x', 'int', is_const=False)
    Tupl.add_instance_attribute('y', 'int', is_const=False)
    Tupl.add_constructor([Parameter.new('Tupl const &', 'arg0')])
    Tupl.add_constructor([])
    Tupl.add_inplace_numeric_operator('+=')
    Tupl.add_inplace_numeric_operator('-=')
    Tupl.add_inplace_numeric_operator('*=')
    Tupl.add_inplace_numeric_operator('/=')

    Tupl.add_unary_numeric_operator('-')

    Tupl.add_inplace_numeric_operator('+=', right='int')


    ManipulatedObject = mod.add_class('ManipulatedObject')
    ManipulatedObject.add_constructor([])
    ManipulatedObject.add_method('GetValue', 'int', [], is_const=True)
    ManipulatedObject.add_method('SetValue', 'void', [Parameter.new('int', 'value')])

    ReferenceManipulator = mod.add_class('ReferenceManipulator', allow_subclassing=True)
    ReferenceManipulator.add_constructor([])
    ReferenceManipulator.add_method('manipulate_object', 'int', [])
    ReferenceManipulator.add_method('do_manipulate_object', 'void',
                                    [Parameter.new('ManipulatedObject&', 'obj', direction=Parameter.DIRECTION_INOUT)],
                                    is_virtual=True, is_pure_virtual=True)


    VectorLike = mod.add_class('VectorLike')
    VectorLike.add_constructor([])
    VectorLike.add_constructor([Parameter.new("VectorLike&", "obj")])
    VectorLike.add_method('get_len', 'size_t', [], custom_name='__len__')
    VectorLike.add_method('add_VectorLike', 'VectorLike', [Parameter.new('VectorLike', 'rhs')], custom_name='__add__')
    VectorLike.add_method('iadd_VectorLike', 'VectorLike', [Parameter.new('VectorLike', 'rhs')], custom_name='__iadd__')
    VectorLike.add_method('mul_VectorLike', 'VectorLike', [Parameter.new('unsigned int', 'n')], custom_name='__mul__')
    VectorLike.add_method('imul_VectorLike', 'VectorLike', [Parameter.new('unsigned int', 'n')], custom_name='__imul__')
    VectorLike.add_method('set_item', 'int', [Parameter.new('int', 'index'), Parameter.new('double', 'value')],
                          custom_name='__setitem__')
    VectorLike.add_method('get_item', 'double', [Parameter.new('int', 'index')], custom_name='__getitem__')
    VectorLike.add_method('set_slice', 'int', [Parameter.new('int', 'index1'),
                                               Parameter.new('int', 'index2'),
                                               Parameter.new('VectorLike', 'values')], custom_name='__setslice__')
    VectorLike.add_method('get_slice', 'VectorLike', [Parameter.new('int', 'index1'),
                                                      Parameter.new('int', 'index2')], custom_name='__getslice__')
    VectorLike.add_method('contains_value', 'int', [Parameter.new('double', 'value')], custom_name='__contains__')
    VectorLike.add_method('append', 'void', [Parameter.new('double', 'value')])

    VectorLike2 = mod.add_class('VectorLike2')
    VectorLike2.add_constructor([])
    VectorLike2.add_method('append', 'void', [Parameter.new('double', 'value')])

    MapLike = mod.add_class('MapLike')
    MapLike.add_constructor([])
    MapLike.add_method('set', 'void', [Parameter.new('int', 'key'), Parameter.new('double', 'value')])

    Error = mod.add_exception('Error')
    DomainError = mod.add_exception('DomainError', parent=Error)

    mod.add_function('my_inverse_func', 'double', [Parameter.new('double', 'x')],
                     throw=[DomainError])

    ClassThatThrows = mod.add_class('ClassThatThrows', allow_subclassing=True)
    ClassThatThrows.add_constructor([Parameter.new('double', 'x')], throw=[DomainError])
    ClassThatThrows.add_method('my_inverse_method', 'double', [Parameter.new('double', 'x')],
                               throw=[DomainError])

    std_exception = mod.add_exception('exception', foreign_cpp_namespace='std', message_rvalue='%(EXC)s.what()')
    mod.add_function('my_inverse_func2', 'double', [Parameter.new('double', 'x')],
                     throw=[std_exception])
    ClassThatThrows.add_method('my_inverse_method2', 'double', [Parameter.new('double', 'x')],
                               throw=[std_exception])

    mod.add_function('my_inverse_func3', 'double', [Parameter.new('double', 'x')],
                     throw=[std_exception])
    ClassThatThrows.add_method('my_inverse_method3', 'double', [Parameter.new('double', 'x')],
                               throw=[std_exception])

    ClassThatThrows.add_method('throw_error', 'int', [], throw=[mod["out_of_range"]], is_const=True, is_virtual=True)

    ClassThatThrows.add_method('throw_out_of_range', 'int', [], throw=[mod["out_of_range"]])

    # https://bugs.launchpad.net/pybindgen/+bug/450255
    ProtectedConstructor = mod.add_class('ProtectedConstructor')
    ProtectedConstructor.add_constructor([])
    ProtectedConstructor.add_constructor([Parameter.new('ProtectedConstructor&', 'c')], visibility='protected')

    # https://bugs.launchpad.net/pybindgen/+bug/455689
    property_std_string = mod.add_struct('property', template_parameters=['std::string'])



    Box = mod.add_class('Box')
    Box.add_constructor([])
    Box.add_static_attribute('instance_count', ReturnValue.new('int'))
    Box.add_method('getFoobarInternalPtr', ReturnValue.new('const Foobar*', reference_existing_object=True), [])
    Box.add_method('getFoobarInternalRef', ReturnValue.new('Foobar&', reference_existing_object=True), [])
    Box.add_method('getFoobarInternalPtr2', ReturnValue.new('Foobar*', return_internal_reference=True), [])
    Box.add_method('getFoobarInternalRef2', ReturnValue.new('Foobar&', return_internal_reference=True), [])
    Box.add_instance_attribute('m_internalFoobar', ReturnValue.new('Foobar*', reference_existing_object=True))


    # multiple inheritance
    MIRoot = mod.add_class('MIRoot')
    MIRoot.add_constructor([])
    MIRoot.add_method('root_method', 'int', [], is_const=True)

    MIBase1 = mod.add_class('MIBase1', parent=MIRoot)
    MIBase1.add_constructor([])
    MIBase1.add_method('base1_method', 'int', [], is_const=True)

    MIBase2 = mod.add_class('MIBase2', parent=MIRoot)
    MIBase2.add_constructor([])
    MIBase2.add_method('base2_method', 'int', [], is_const=True)

    MIMixed = mod.add_class('MIMixed', parent=[MIBase1, MIBase2])
    MIMixed.add_constructor([])
    MIMixed.add_method('mixed_method', 'int', [], is_const=True)


    mod.add_function('my_throwing_func', 'Tupl', [], throw=[std_exception])


    IFoo = mod.add_class("IFoo", destructor_visibility='protected', allow_subclassing=True)
    IFoo.add_method("DoSomething", None, [], is_pure_virtual=True)

    IFooImpl = mod.add_class("IFooImpl", parent=IFoo, destructor_visibility='public')
    IFooImpl.add_constructor([])
    IFooImpl.add_method("DoSomething", None, [], is_virtual=True)


    mod.add_function("test_args_kwargs", "int", [param("const char *", "args"), param("const char *", "kwargs")])

    # https://github.com/gjcarneiro/pybindgen/issues/21
    cls = mod.add_class('RAStruct')
    cls.add_constructor([])
    cls.add_constructor([param('RAStruct const &', 'arg0')])
    cls.add_instance_attribute('a', 'int', is_const=False)

    cls = mod.add_class('ReturnConstRef', allow_subclassing=True)
    cls.add_constructor([])
    cls.add_constructor([param('ReturnConstRef const &', 'arg0')])
    cls.add_method('ReturnMyAStruct',
                   'RAStruct const &',
                   [],
                   is_pure_virtual=True, is_virtual=True)

    cls = mod.add_class('RAReturnConstRef', parent=mod['ReturnConstRef'])
    cls.add_constructor([])
    cls.add_constructor([param('int', 'value')])
    cls.add_constructor([param('RAReturnConstRef const &', 'arg0')])
    cls.add_method('ReturnMyAStruct',
                   'RAStruct const &',
                   [],
                   is_virtual=True)



    #### --- error handler ---
    class MyErrorHandler(pybindgen.settings.ErrorHandler):
        def __init__(self):
            super(MyErrorHandler, self).__init__()
            self.num_errors = 0
        def handle_error(self, wrapper, exception, traceback_):
            print("exception %s in wrapper %s" % (exception, wrapper), file=sys.stderr)
            self.num_errors += 1
            if 0: # verbose?
                import traceback
                traceback.print_tb(traceback_)
            return True
    pybindgen.settings.error_handler = MyErrorHandler()

    foomodulegen_common.customize_module(mod)

    ## ---- finally, generate the whole thing ----
    mod.generate(FileCodeSink(out_file))


if __name__ == '__main__':
    import os
    if "PYBINDGEN_ENABLE_PROFILING" in os.environ:
        try:
            import cProfile as profile
        except ImportError:
            my_module_gen(sys.stdout)
        else:
            print("** running under profiler", file=sys.stderr)
            profile.run('my_module_gen(sys.stdout)', 'foomodulegen.pstat')
    else:
        my_module_gen(sys.stdout)

