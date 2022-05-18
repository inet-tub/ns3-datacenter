#include "foo.h"
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <stdexcept>

int print_something(const char *message)
{
    std::cout << "MESSAGE1: " << message << std::endl;
    return strlen(message);
}

int print_something_else(const char *message2)
{
    std::cout << "MESSAGE2: " << message2 << std::endl;
    return strlen(message2);
}

int get_int_from_string(const char *from_string, int multiplier)
{
    return atoi(from_string)*multiplier;
}

int get_int_from_float(double from_float, int multiplier)
{
    return (int) from_float*multiplier;
}

std::string SomeObject::staticData = std::string("Hello Static World!");

SomeObject::~SomeObject ()
{
    SomeObject::instance_count--;
    delete m_foo_ptr;
    if (m_zbr)
        m_zbr->Unref ();
    if (m_internal_zbr) {
        m_internal_zbr->Unref ();
        m_internal_zbr = NULL;
    }
    delete m_foobar;
    m_foobar = NULL;
}

SomeObject::SomeObject (const SomeObject &other)
    : m_prefix (other.m_prefix),
      m_foobar (NULL)
{
    if (other.m_foo_ptr)
        m_foo_ptr = new Foo (*other.m_foo_ptr);
    else
        m_foo_ptr = NULL;
    m_foo_shared_ptr = NULL;

    if (other.m_zbr) {
        m_zbr = other.m_zbr;
        m_zbr->Ref();
    } else
        m_zbr = NULL;

    m_internal_zbr = new Zbr;
    m_pyobject = NULL;
    m_foobar = NULL;
    SomeObject::instance_count++;
}

SomeObject::SomeObject (std::string const prefix)
    : m_prefix (prefix), m_foo_ptr (0),
      m_foo_shared_ptr (0), m_zbr (0),
      m_internal_zbr (new Zbr),
      m_pyobject (NULL),
      m_foobar (NULL)
{
    SomeObject::instance_count++;
}

SomeObject::SomeObject (int prefix_len)
    : m_prefix (prefix_len, 'X'), m_foo_ptr (0),
      m_foo_shared_ptr (0), m_zbr (0),
      m_internal_zbr (new Zbr),
      m_pyobject (NULL),
      m_foobar (NULL)
{
    SomeObject::instance_count++;
}

int SomeObject::get_int (const char *from_string)
{
    return atoi(from_string);
}

int SomeObject::get_int (double from_float)
{
    return (int) from_float;
}

std::string
SomeObject::protected_method_that_is_not_virtual (std::string arg) const
{
    return m_prefix + arg;
}


int SomeObject::instance_count = 0;

class HiddenClass : public Bar
{
};


Foo*
get_hidden_subclass_pointer ()
{
    return new HiddenClass;
}


static SomeObject *g_someObject = 0;

// Transfer ownership of 'obj' to the library
void store_some_object(SomeObject *obj)
{
    delete g_someObject;
    g_someObject = obj;
}

// Invokes the virtual method in the stored SomeObject
std::string invoke_some_object_get_prefix()
{
    if (g_someObject)
        return g_someObject->get_prefix();
    else
        return std::string();
}

// Transfer ownership of 'obj' away from the library
SomeObject* take_some_object()
{
    SomeObject *retval = g_someObject;
    g_someObject = 0;
    return retval;
}

// Deletes the contained object, if any
void delete_some_object()
{
    delete g_someObject;
    g_someObject = 0;
}


namespace xpto
{
    std::string some_function()
    {
        return "hello";
    }

    std::string get_foo_datum(FooXpto const &foo)
    {
        return foo.get_datum();
    }

}


Foo g_foo;

void function_that_takes_foo(Foo foo)
{
    g_foo = foo;
}

Foo function_that_returns_foo()
{
    return g_foo;
}

int Foo::instance_count = 0;
int SomeObject::NestedClass::instance_count = 0;
int Foobar::instance_count = 0;
int Box::instance_count = 0;

Foobar* get_foobar_with_other_as_custodian (SomeObject *other)
{
    return other->get_foobar_with_self_as_custodian();
}

Foobar* create_new_foobar()
{
    return new Foobar;
}

void set_foobar_with_other_as_custodian(Foobar *foobar, SomeObject *other)
{
    other->set_foobar_with_self_as_custodian(foobar);
}

SomeObject * set_foobar_with_return_as_custodian(Foobar *foobar)
{
    SomeObject *some = new SomeObject("xxx");
    some->set_foobar_with_self_as_custodian(foobar);
    return some;
}

std::string some_object_get_something_prefixed(const SomeObject *obj, const std::string something)
{
    return obj->get_prefix() + something;
}

std::string some_object_val_get_something_prefixed(SomeObject obj, const std::string something)
{
    return obj.get_prefix() + something;
}

std::string some_object_ref_get_something_prefixed(const SomeObject &obj, const std::string something)
{
    return obj.get_prefix() + something;
}

namespace xpto
{
    FooType g_fooType;
    FooType get_foo_type ()
    {
        return g_fooType;
    }
    void set_foo_type (FooType type)
    {
        g_fooType = type;
    }
    void set_foo_type_inout (FooType &type)
    {
        FooType oldfooType = g_fooType;
        g_fooType = type;
        type = oldfooType;
    }

    void set_foo_type_ptr (FooType *type)
    {
        FooType oldfooType = g_fooType;
        g_fooType = *type;
        *type = oldfooType;
    }

}


SingletonClass *SingletonClass::m_instance = NULL;

InterfaceId make_interface_id ()
{
    return InterfaceId ();
}

template <> std::string TypeNameGet<int> (void)
{
    return "int";
}


static Zbr *g_zbr = NULL;
int Zbr::instance_count = 0;

void store_zbr (Zbr *zbr)
{
    if (g_zbr)
        g_zbr->Unref ();
    g_zbr = zbr;
}

int invoke_zbr (int x)
{
    return g_zbr->get_int (x);
}

void delete_stored_zbr (void)
{
    if (g_zbr)
        g_zbr->Unref ();
    g_zbr = NULL;
}

float matrix_sum_of_elements (float *matrix)
{
    float result = 0;
    for (int i = 0; i < 6; i++)
        result += matrix[i];
    return result;
}

void matrix_identity_new (float *matrix)
{
    matrix[0] = 1;
    matrix[1] = 0;
    matrix[2] = 0;
    matrix[3] = 0;
    matrix[4] = 1;
    matrix[5] = 0;
}

static SimpleStructList g_simpleList;

SimpleStructList get_simple_list ()
{
    SimpleStructList retval;
    for (int i = 0; i < 10; i++)
    {
        simple_struct_t val = {i};
        retval.push_back(val);
    }
    return retval;
}

int set_simple_list (SimpleStructList list)
{
    int count = 0;
    g_simpleList = list;
    for (SimpleStructList::iterator iter = g_simpleList.begin(); iter != g_simpleList.end(); iter++)
        count += iter->xpto;
    return count;
}


SimpleStructList
TestContainer::get_simple_list ()
{
    SimpleStructList retval;
    for (int i = 0; i < 10; i++)
    {
        simple_struct_t val = {i};
        retval.push_back(val);
    }
    return retval;
}

int
TestContainer::set_simple_list (SimpleStructList list)
{
    int count = 0;
    m_simpleList = list;
    for (SimpleStructList::iterator iter = m_simpleList.begin(); iter != m_simpleList.end(); iter++)
        count += iter->xpto;
    return count;
}

int
TestContainer::set_simple_list_by_ref (SimpleStructList &inout_list)
{
    int count = 0;
    m_simpleList = inout_list;
    for (SimpleStructList::iterator iter = inout_list.begin(); iter != inout_list.end(); iter++)
    {
        iter->xpto *= 2;
        count += iter->xpto;
    }
    return count;
}



std::vector<simple_struct_t>
TestContainer::get_simple_vec ()
{
    std::vector<simple_struct_t> retval;
    for (int i = 0; i < 10; i++)
    {
        simple_struct_t val = {i};
        retval.push_back(val);
    }
    return retval;
}

int
TestContainer::set_simple_vec (std::vector<simple_struct_t> list)
{
    int count = 0;
    m_simpleList = list;
    for (std::vector<simple_struct_t>::iterator iter = m_simpleList.begin(); iter != m_simpleList.end(); iter++)
        count += iter->xpto;
    return count;
}


SimpleStructMap
TestContainer::get_simple_map ()
{
    SimpleStructMap retval;
    for (int i = 0; i < 10; i++)
    {
        simple_struct_t val = {i};
        std::ostringstream os;
        os << i;
        retval[os.str()] = val;
    }
    return retval;
}

int
TestContainer::set_simple_map (SimpleStructMap map)
{
    int count = 0;
    m_simpleMap = map;
    for (SimpleStructMap::iterator iter = m_simpleMap.begin(); iter != m_simpleMap.end(); iter++)
        count += iter->second.xpto;
    return count;
}

SimpleStructUnorderedMap
TestContainer::get_simple_unordered_map ()
{
    SimpleStructUnorderedMap retval;
    for (int i = 0; i < 10; i++)
    {
        simple_struct_t val = {i};
        std::ostringstream os;
        os << i;
        retval[os.str()] = val;
    }
    return retval;
}

int
TestContainer::set_simple_unordered_map (SimpleStructUnorderedMap map)
{
    int count = 0;
    m_simpleUnorderedMap = map;
    for (SimpleStructUnorderedMap::iterator iter = m_simpleUnorderedMap.begin(); iter != m_simpleUnorderedMap.end(); iter++)
        count += iter->second.xpto;
    return count;
}

void
TestContainer::get_vec (std::vector<std::string> &outVec)
{
    outVec.clear ();
    outVec.push_back ("hello");
    outVec.push_back ("world");
}

void
TestContainer::set_vec_ptr (std::vector<std::string> *inVec)
{
    m_vec = inVec;
}


void
TestContainer::get_vec_ptr (std::vector<std::string> *outVec)
{
    *outVec = *m_vec;
}


std::map<std::string, int>
get_map ()
{
    std::map<std::string, int> rv;
    rv["123"] = 123;
    rv["456"] = 456;
    return rv;
}

std::set<uint32_t>
get_set ()
{
    std::set<uint32_t> rv;
    return rv;
}

namespace xpto
{
    FlowId
    get_flow_id (FlowId flowId)
    {
        return flowId + 1;
    }
}

double
my_inverse_func (double x) throw (DomainError)
{
    if (x == 0)
    {
        DomainError ex;
        ex.message = "value must be != 0";
        throw ex;
    }
    return 1/x;
}

double
ClassThatThrows::my_inverse_method (double x) throw (DomainError)
{
    if (x == 0)
    {
        DomainError ex;
        ex.message = "value must be != 0";
        throw ex;
    }
    return 1/x;
}


ClassThatThrows::ClassThatThrows (double x) throw (DomainError)
{
    if (x == 0)
    {
        DomainError ex;
        ex.message = "value must be != 0";
        throw ex;
    }
}

int
ClassThatThrows::throw_out_of_range() throw (std::out_of_range)
{
    throw std::out_of_range("imaginary index out of range");
}

int
ClassThatThrows::throw_error () const throw (std::exception)
{
    throw std::runtime_error ("an error");
}


double
my_inverse_func2 (double x) throw (std::exception)
{
    if (x == 0)
    {
        throw std::runtime_error ("value must be != 0");
    }
    return 1/x;
}

double
my_inverse_func3 (double x)
{
    if (x == 0)
    {
        throw std::runtime_error ("value must be != 0");
    }
    return 1/x;
}

double
ClassThatThrows::my_inverse_method2 (double x) throw (std::exception)
{
    if (x == 0)
    {
        throw std::runtime_error ("value must be != 0");
    }
    return 1/x;
}

double
ClassThatThrows::my_inverse_method3 (double x)
{
    if (x == 0)
    {
        throw std::runtime_error ("value must be != 0");
    }
    return 1/x;
}


Tupl
my_throwing_func () throw (std::exception)
{
    Tupl retval;
    return retval;
}

int
test_args_kwargs(const char *args, const char *kwargs)
{
    return (int) (kwargs - args);
}

ToBeFreed::ToBeFreed(int size) {
    m_size = size;
    m = return_c_string_to_be_freed(size);
}

ToBeFreed::~ToBeFreed() {
    free(m);
    m = NULL;
}

 ToBeFreed::ToBeFreed(const ToBeFreed& from) {
     m_size = from.m_size;
     m = return_c_string_to_be_freed(m_size);
 }

char *ToBeFreed::value()  {
        return m;
}

char *return_c_string_to_be_freed(int size)
{
    char *test = (char *)malloc(size * sizeof(char));
    strcpy(test, "testingonly");
    return test;
}

ToBeFreed *
return_class_to_be_freed(int size)
{
   return new ToBeFreed(size);
}

char *
return_c_string_to_not_be_freed(int size)
{
   return return_c_string_to_be_freed(size);
}

ToBeFreed *
return_class_to_not_be_freed(int size)
{
   return new ToBeFreed(size);
}

void Add (const std::string filePath,
          double defaultZ,
          char delimiter)
{
}
