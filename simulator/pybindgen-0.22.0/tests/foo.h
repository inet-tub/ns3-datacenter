// -*- Mode: C++; c-file-style: "stroustrup"; indent-tabs-mode:nil; -*-
#ifndef   	FOO_H_
# define   	FOO_H_

#include <Python.h>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <exception>
#include <algorithm>
#include <stdexcept>

#include <stdint.h>

// Deprecation warnings look ugly and confusing; better to just
// disable them and change this macro when we want to specifically
// test them.
#define ENABLE_DEPRECATIONS 0

#ifndef DEPRECATED
# if ENABLE_DEPRECATIONS && __GNUC__ > 2
#  define DEPRECATED  __attribute__((deprecated))
# else
#  define DEPRECATED
# endif
#endif

// Yes, this code is stupid, I know; it is only meant as an example!

int print_something(const char *message) DEPRECATED;
int print_something_else(const char *message2);

/* -#- name=get_int -#- */
int get_int_from_string(const char *from_string, int multiplier=1);
// -#- name=get_int; @multiplier(default_value=1) -#-
int get_int_from_float(double from_float, int multiplier);

// In this example PointerHolder<T> automatically implies
// caller_owns_return=True when used as ReturnValue, and
// transfer_ownership=False when used as parameter.
template <typename T>
struct PointerHolder
{
    T *thePointer;
};


// -#- automatic_type_narrowing=True -#-
class Foo
{
    std::string m_datum;
    bool m_initialized;
public:
    static int instance_count;

    Foo () : m_datum (""), m_initialized (false)
        { Foo::instance_count++; }

    Foo (int xpto)  DEPRECATED : m_initialized (false) { xpto++; }

    Foo (std::string const &datum) : m_datum (datum), m_initialized (false)
        { Foo::instance_count++; }
    const std::string get_datum () const { return m_datum; }

    std::string get_datum_deprecated () const DEPRECATED { return m_datum; }

    Foo (Foo const & other) : m_datum (other.get_datum ()), m_initialized (false)
        { Foo::instance_count++; }

    void initialize () { m_initialized = true; }
    bool is_initialized () const { return m_initialized; }

    virtual ~Foo() { Foo::instance_count--; }

    static int add_sub (int a, int b=3, bool subtract=false)
        {
            return a+b;
        }
};

inline std::ostream & operator << (std::ostream &os, Foo const &foo)
{
    os << foo.get_datum ();
    return os;
}

inline bool is_unique(const Foo& foo)
{
    return foo.instance_count == 1;
}

// -#- automatic_type_narrowing=True -#-
class Zoo
{
    std::string m_datum;
public:
    Zoo () : m_datum ("")
        {}
    Zoo (std::string datum) : m_datum (datum)
        {}
    virtual ~Zoo() {}
    std::string get_datum () const { return m_datum; }

    operator Foo() const {
        return Foo(m_datum);
    }
};


class ClassThatTakesFoo
{
    Foo m_foo;
public:
    ClassThatTakesFoo(Foo foo) : m_foo(foo) {}
    Foo get_foo () const { return m_foo; }
};

extern Foo g_foo;

void function_that_takes_foo(Foo foo);
Foo function_that_returns_foo();


class Bar : public Foo
{
public:
    static std::string Hooray () {
        return std::string ("Hooray!");
    }
    virtual ~Bar() {}
};

// caller owns return
// -#- @return(caller_owns_return=true) -#-
Foo* get_hidden_subclass_pointer ();

// -#- incref_method=Ref; decref_method=Unref; peekref_method=GetReferenceCount -#-
class Zbr
{
    int m_refcount;
    std::string m_datum;
public:
    Zbr () : m_refcount (1), m_datum ("")
        { Zbr::instance_count++; }
    Zbr (std::string datum) :  m_refcount (1), m_datum (datum)
        { Zbr::instance_count++; }

    std::string get_datum () const { return m_datum; }

    Zbr (Zbr const & other) :
        m_refcount (1), m_datum (other.get_datum ())
        {Zbr::instance_count++;}

    void Ref () {
        // std::cerr << "Ref Zbr " << this << " from " << m_refcount << std::endl;
        ++m_refcount;
    }
    void Unref () {
        // std::cerr << "Unref Zbr " << this << " from " << m_refcount << std::endl;
        if (--m_refcount == 0)
            delete this;
    }
    int GetReferenceCount () const { return m_refcount; }

    virtual int get_int (int x) {
        return x;
    }

    static int instance_count;

    virtual ~Zbr () {
        --Zbr::instance_count;
    }

    // -#- @foobaz(transfer_ownership=true, direction=out) -#-
    int get_value (int* foobaz) { *foobaz = 123; return -1; }
};

// -#- @zbr(transfer_ownership=true) -#-
void store_zbr (Zbr *zbr);
int invoke_zbr (int x);
void delete_stored_zbr (void);


// -#- allow_subclassing=true -#-
class Foobar
{
    Foobar (const Foobar &);
    Foobar& operator= (const Foobar &);

public:
    static int instance_count;

    Foobar ()
        { Foobar::instance_count++; }

    virtual ~Foobar() { Foobar::instance_count--; }
};


class SomeObject
{
public:
    std::string m_prefix;

    enum {
        TYPE_FOO,
        TYPE_BAR,
    } type;

    static int instance_count;


    // A nested class
    // -#- automatic_type_narrowing=True -#-
    class NestedClass
    {
        std::string m_datum;
    public:
        static int instance_count;

        NestedClass () : m_datum ("")
            { Foo::instance_count++; }
        NestedClass (std::string datum) : m_datum (datum)
            { Foo::instance_count++; }
        std::string get_datum () const { return m_datum; }

        NestedClass (NestedClass const & other) : m_datum (other.get_datum ())
            { Foo::instance_count++; }

        virtual ~NestedClass() { NestedClass::instance_count--; }
    };

    // A nested enum
    enum NestedEnum {
        FOO_TYPE_AAA,
        FOO_TYPE_BBB,
        FOO_TYPE_CCC,
    };


    // An anonymous nested enum
    enum  {
        CONSTANT_A,
        CONSTANT_B,
        CONSTANT_C
    };


private:
    Foo m_foo_value;
    Foo *m_foo_ptr;
    Foo *m_foo_shared_ptr;
    Zbr *m_zbr;
    Zbr *m_internal_zbr;

    PyObject *m_pyobject;
    Foobar *m_foobar;

    SomeObject ();

public:

    static std::string staticData;

    virtual ~SomeObject ();
    SomeObject (const SomeObject &other);
    SomeObject (std::string const prefix);
    SomeObject (int prefix_len);

    // -#- @message(direction=inout) -#-
    int add_prefix (std::string& message) {
        message = m_prefix + message;
        return message.size ();
    }

    // -#- @message(direction=inout) -#-
    int operator() (std::string& message) {
        message = m_prefix + message;
        return message.size ();
    }

    // --------  Virtual methods ----------
    virtual std::string get_prefix () const {
        return m_prefix;
    }

    std::string call_get_prefix () const {
        return get_prefix();
    }

    virtual std::string get_prefix_with_foo_value (Foo foo) const {
        return m_prefix + foo.get_datum();
    }

    // -#- @foo(direction=inout) -#-
    virtual std::string get_prefix_with_foo_ref (const Foo &foo) const {
        return m_prefix + foo.get_datum ();
    }

    virtual std::string get_prefix_with_foo_ptr (const Foo *foo) const {
        return m_prefix + foo->get_datum ();
    }


    // A couple of overloaded virtual methods
    virtual std::string get_something () const {
        return "something";
    }
    virtual std::string get_something (int x) const {
        std::stringstream out;
        out << x;
        return out.str ();
    }

    // -#- @pyobject(transfer_ownership=false) -#-
    virtual void set_pyobject (PyObject *pyobject) {
        if (m_pyobject) {
            Py_DECREF(m_pyobject);
        }
        Py_INCREF(pyobject);
        m_pyobject = pyobject;
    }

    // -#- @return(caller_owns_return=true) -#-
    virtual PyObject* get_pyobject (void) {
        if (m_pyobject) {
            Py_INCREF(m_pyobject);
            return m_pyobject;
        } else {
            return NULL;
        }
    }

    // pass by value, direction=in
    void set_foo_value (Foo foo) {
        m_foo_value = foo;
    }

    // pass by reference, direction=in
    void set_foo_by_ref (const Foo& foo) {
        m_foo_value = foo;
    }

    // pass by reference, direction=out
    // -#- @foo(direction=out) -#-
    void get_foo_by_ref (Foo& foo) {
        foo = m_foo_value;
    }

    // -#- @foo(transfer_ownership=true) -#-
    void set_foo_ptr (Foo *foo) {
        if (m_foo_ptr)
            delete m_foo_ptr;
        m_foo_ptr = foo;
    }

    // -#- @foo(transfer_ownership=false) -#-
    void set_foo_shared_ptr (Foo *foo) {
        m_foo_shared_ptr = foo;
    }

    // return value
    Foo get_foo_value () {
        return m_foo_value;
    }

    // -#- @return(caller_owns_return=false) -#-
    const Foo * get_foo_shared_ptr () {
        return m_foo_shared_ptr;
    }

    // -#- @return(caller_owns_return=true) -#-
    Foo * get_foo_ptr () {
        Foo *foo = m_foo_ptr;
        m_foo_ptr = NULL;
        return foo;
    }

    // -#- @return(caller_owns_return=true) -#-
    Zbr* get_zbr () {
        if (m_zbr)
        {
            m_zbr->Ref ();
            return m_zbr;
        } else
            return NULL;
    }

    // -#- @return(caller_owns_return=true) -#-
    Zbr* get_internal_zbr () {
        m_internal_zbr->Ref ();
        return m_internal_zbr;
    }

    // return reference counted object, caller does not own return
    // -#- @return(caller_owns_return=false) -#-
    Zbr* peek_zbr () { return m_zbr; }

    // pass reference counted object, transfer ownership
    // -#- @zbr(transfer_ownership=true) -#-
    void set_zbr_transfer (Zbr *zbr) {
        if (m_zbr)
            m_zbr->Unref ();
        m_zbr = zbr;
    }

    // pass reference counted object, does not transfer ownership
    // -#- @zbr(transfer_ownership=false) -#-
    void set_zbr_shared (Zbr *zbr) {
        if (m_zbr)
            m_zbr->Unref ();
        zbr->Ref ();
        m_zbr = zbr;
    }


    // return reference counted object, caller does not own return
    PointerHolder<Zbr> get_zbr_pholder () {
        PointerHolder<Zbr> foo = { m_zbr };
        m_zbr->Ref ();
        return foo;
    }

    // pass reference counted object, transfer ownership
    void set_zbr_pholder (PointerHolder<Zbr> zbr) {
        if (m_zbr)
            m_zbr->Unref ();
        m_zbr = zbr.thePointer;
        m_zbr->Ref ();
    }

    int get_int (const char *from_string);
    int get_int (double from_float);

    // custodian/ward tests

    // -#- @return(custodian=0, reference_existing_object=true) -#-
    Foobar* get_foobar_with_self_as_custodian () {
        if (m_foobar == NULL) {
            m_foobar = new Foobar;
        }
        return m_foobar;
    }
    // -#- @return(custodian=1, reference_existing_object=true); @other(transfer_ownership=false) -#-
    Foobar* get_foobar_with_other_as_custodian (SomeObject *other) {
        return other->get_foobar_with_self_as_custodian ();
    }
    // -#- @foobar(custodian=0, transfer_ownership=True) -#-
    void set_foobar_with_self_as_custodian (Foobar *foobar) {
        delete m_foobar;
        m_foobar = foobar;
    }

    virtual const char* method_returning_cstring() const { return "foobar"; }

protected:
    std::string protected_method_that_is_not_virtual (std::string arg) const;
};


// A function that will appear as a method of SomeObject
// -#- as_method=get_something_prefixed; of_class=SomeObject -#-
// -#- @obj(transfer_ownership=false) -#-
std::string some_object_get_something_prefixed(const SomeObject *obj, const std::string something);

// -#- as_method=val_get_something_prefixed; of_class=SomeObject -#-
std::string some_object_val_get_something_prefixed(SomeObject obj, const std::string something);
// -#- as_method=ref_get_something_prefixed; of_class=SomeObject -#-
std::string some_object_ref_get_something_prefixed(const SomeObject &obj, const std::string something);


// Transfer ownership of 'obj' to the library
// -#- @obj(transfer_ownership=true) -#-
void store_some_object(SomeObject *obj);

// Invokes the virtual method in the stored SomeObject
std::string invoke_some_object_get_prefix();

// Transfer ownership of 'obj' away from the library
// -#- @return(caller_owns_return=true) -#-
SomeObject* take_some_object();

// Deletes the contained object, if any
void delete_some_object();


namespace xpto
{
    typedef uint32_t FlowId;
    FlowId get_flow_id (FlowId flowId);

    std::string some_function();

    class SomeClass
    {
    public:
        SomeClass() {}
    };

    enum FooType {
        FOO_TYPE_AAA,
        FOO_TYPE_BBB,
        FOO_TYPE_CCC,
    };

    FooType get_foo_type ();
    void set_foo_type (FooType type);

    // -#- @type(direction=inout) -#-
    void set_foo_type_ptr (FooType *type);

    // -#- @type(direction=inout) -#-
    void set_foo_type_inout (FooType &type);

    typedef Foo FooXpto;

    std::string get_foo_datum(FooXpto const &foo);

    struct XptoClass
    {
        // -#- @return(caller_owns_return=true) -#-
        SomeClass* GetSomeClass()
            {
                return NULL;
            }
    };
}

// -#- @return(custodian=1, reference_existing_object=true); @other(transfer_ownership=false) -#-
Foobar* get_foobar_with_other_as_custodian(SomeObject *other);

// -#- @return(caller_owns_return=true) -#-
Foobar* create_new_foobar();
// -#- @foobar(custodian=2, transfer_ownership=true); @other(transfer_ownership=false) -#-
void set_foobar_with_other_as_custodian(Foobar *foobar, SomeObject *other);
// -#- @foobar(custodian=-1, transfer_ownership=true); @return(caller_owns_return=true) -#-
SomeObject * set_foobar_with_return_as_custodian(Foobar *foobar);

// -#- is_singleton=true -#-
class SingletonClass
{
private:
    static SingletonClass *m_instance;

    SingletonClass () {}
    ~SingletonClass () {}

public:
    // -#- @return(caller_owns_return=true) -#-
    static SingletonClass *GetInstance () {
            if (!m_instance)
                m_instance = new SingletonClass;
            return m_instance;
        }
};


class InterfaceId
{
public:
    ~InterfaceId () {}
private:
    InterfaceId () {}
    friend InterfaceId make_interface_id ();
    // -#- ignore -#-
    friend InterfaceId make_object_interface_id ();
};

InterfaceId make_interface_id ();

template <typename T> std::string TypeNameGet (void)
{
  return "unknown";
}


// -#- template_instance_names=int=>IntegerTypeNameGet -#-
template <> std::string TypeNameGet<int> (void);


// just to force a template instantiation
struct __foo__
{
     std::string get ()
        {
            return TypeNameGet<int> ();
        }
};


// Test code generation errors and error handling

class CannotBeConstructed
{
public:
    ~CannotBeConstructed () {}

    // This static method cannot be generated
    static CannotBeConstructed get_value () {
        return CannotBeConstructed ();
    }

    // This static method can be generated because caller-owns-return=true
    // -#- @return(caller_owns_return=true) -#-
    static CannotBeConstructed* get_ptr () {
        return new CannotBeConstructed ();
    }
private:
    CannotBeConstructed () {}
};

// This function cannot be generated
inline CannotBeConstructed get_cannot_be_constructed_value () {
    return CannotBeConstructed::get_value ();
}

// This static method can be generated because caller-owns-return=true
// -#- @return(caller_owns_return=true) -#-
inline CannotBeConstructed* get_cannot_be_constructed_ptr () {
    return CannotBeConstructed::get_ptr ();
}

class AbstractBaseClass
{
public:
    virtual ~AbstractBaseClass () {}

protected:
    // A pure virtual method with a parameter type so strange that it
    // cannot be possibly be automatically wrapped by pybindgen; this
    // will cause the cannot_be_constructed flag on the class to be
    // set to true, and allow us to test code generation error
    // handling (see below).
    virtual void do_something (char ****) = 0;

    AbstractBaseClass () {}
};

class AbstractBaseClassImpl : public AbstractBaseClass
{
public:
    virtual ~AbstractBaseClassImpl () {}

    // -#- @return(caller_owns_return=true) -#-
    static AbstractBaseClass* get_abstract_base_class_ptr1 ()
        {
            return new AbstractBaseClassImpl;
        }

    // This method will be scanned by castxmlparser and generate an error
    // that is only detected in code generation time.
    // -#- @return(caller_owns_return=false) -#-
    static AbstractBaseClass* get_abstract_base_class_ptr2 ()
        {
            static AbstractBaseClassImpl *singleton = NULL;
            if (!singleton)
                singleton = new AbstractBaseClassImpl;
            return singleton;
        }

protected:
    virtual void do_something (char ****) {}

    AbstractBaseClassImpl () {}
};


// -#- @return(caller_owns_return=true) -#-
inline AbstractBaseClass* get_abstract_base_class_ptr1 ()
{
    return AbstractBaseClassImpl::get_abstract_base_class_ptr1 ();
}

// This function will be scanned by castxmlparser and generate an error
// that is only detected in code generation time.
// -#- @return(caller_owns_return=false) -#-
inline AbstractBaseClass* get_abstract_base_class_ptr2 ()
{
    static AbstractBaseClass *singleton = NULL;
    if (!singleton)
        singleton = AbstractBaseClassImpl::get_abstract_base_class_ptr1 ();
    return singleton;
}


// Class to test private/protected virtual methods
class AbstractBaseClass2
{
protected:
    AbstractBaseClass2 () {}

public:
    virtual ~AbstractBaseClass2 () {}

    int invoke_private_virtual (int x) const {
        return private_virtual (x);
    }

    int invoke_protected_virtual (int x) const {
        return protected_virtual (x);
    }

    int invoke_protected_pure_virtual (int x) const {
        return protected_pure_virtual (x);
    }

protected:
    virtual int protected_virtual (int x) const { return x+1; }

    virtual int protected_pure_virtual (int x) const = 0;

private:
    virtual int private_virtual (int x) const = 0;

    AbstractBaseClass2 (const AbstractBaseClass2 &other) { other.invoke_protected_virtual(0); }
};

// -#- allow_subclassing=true -#-
class AbstractXpto
{
public:
    AbstractXpto () {}
    virtual ~AbstractXpto () {}

    virtual void something (int x) const = 0;
};

class AbstractXptoImpl : public AbstractXpto
{
public:
    AbstractXptoImpl () {}

    virtual void something (int x) const { x++; }
};


// Test anonymous structures
union Word
{
    uint16_t word;
    struct
    {
        uint8_t low, high;
    };
};

// Test float array
// -#- @matrix(direction=in, array_length=6) -#-
float matrix_sum_of_elements (float *matrix);

// -#- @matrix(direction=out, array_length=6) -#-
void matrix_identity_new (float *matrix);

namespace TopNs
{

    class OuterBase
    {
    public:
    };

    namespace PrefixBottomNs {
        class PrefixInner : public OuterBase
        {
        public:
            PrefixInner () {}
            void Do (void) {}
        };
    }

}


// Bug #245097
typedef void (*Callback) (void);
void function_that_takes_callback (Callback cb);


// <Bug #246069>
struct Socket
{
    virtual int Bind () { return -1; }
    virtual int Bind (int address) { return address; }

    virtual ~Socket () {}
};

struct UdpSocket : public Socket
{
    virtual int Bind () { return 0; }
};
// </Bug #246069>


struct simple_struct_t
{
    int xpto;
};

// -- Containers:

typedef std::vector<simple_struct_t> SimpleStructList;
typedef std::vector<simple_struct_t> SimpleStructVec;
typedef std::map<std::string, simple_struct_t> SimpleStructMap;
typedef std::unordered_map<std::string, simple_struct_t> SimpleStructUnorderedMap;


SimpleStructList get_simple_list ();
int set_simple_list (SimpleStructList list);

class TestContainer
{
public:

    std::set<float> m_floatSet;

    TestContainer () : m_vec (NULL) {
        m_floatSet.insert (1.0);
        m_floatSet.insert (2.0);
        m_floatSet.insert (3.0);
    }
    virtual ~TestContainer () {
        delete m_vec;
    }

    virtual SimpleStructList get_simple_list ();
    virtual int set_simple_list (SimpleStructList list);

    // -#- @inout_list(direction=inout) -#-
    virtual int set_simple_list_by_ref (SimpleStructList &inout_list);

    virtual SimpleStructVec get_simple_vec ();
    virtual int set_simple_vec (SimpleStructVec vec);

    virtual SimpleStructMap get_simple_map ();
    virtual int set_simple_map (SimpleStructMap map);

    virtual SimpleStructUnorderedMap get_simple_unordered_map ();
    virtual int set_simple_unordered_map (SimpleStructUnorderedMap map);

    // -#- @outVec(direction=out) -#-
    void get_vec (std::vector<std::string> &outVec);

    // -#- @inVec(direction=in, transfer_ownership=true) -#-
    void set_vec_ptr (std::vector<std::string> *inVec);

    // -#- @outVec(direction=out) -#-
    void get_vec_ptr (std::vector<std::string> *outVec);

private:
    SimpleStructList m_simpleList;
    SimpleStructMap m_simpleMap;
    SimpleStructUnorderedMap m_simpleUnorderedMap;

    std::vector<std::string> *m_vec;

};

std::map<std::string, int> get_map ();

std::set<uint32_t> get_set ();


// test binary operators

struct Tupl
{
    int x, y;

    inline Tupl operator * (Tupl const &b)
        {
            Tupl retval;
            retval.x = x * b.x;
            retval.y = y * b.y;
            return retval;
        }

    inline Tupl operator / (Tupl const &b)
        {
            Tupl retval;
            retval.x = x / b.x;
            retval.y = y / b.y;
            return retval;
        }

    inline bool operator == (Tupl const &b)
        {
            return (x == b.x && y == b.y);
        }

    inline bool operator != (Tupl const &b)
        {
            return (x != b.x || y != b.y);
        }


    inline Tupl operator - ()
        {
            Tupl retval;
            retval.x = -x;
            retval.y = -y;
            return retval;
        }

};

inline bool operator < (Tupl const &a, Tupl const &b)
{
    return (a.x < b.x || (a.x == b.x && a.y < b.y));
}

inline bool operator <= (Tupl const &a, Tupl const &b)
{
    return (a.x <= b.x || (a.x == b.x && a.y <= b.y));
}

inline bool operator > (Tupl const &a, Tupl const &b)
{
    return (a.x > b.x || (a.x == b.x && a.y > b.y));
}

inline bool operator >= (Tupl const &a, Tupl const &b)
{
    return (a.x >= b.x || (a.x == b.x && a.y >= b.y));
}

inline Tupl operator + (Tupl const &a, Tupl const &b)
{
    Tupl retval;
    retval.x = a.x + b.x;
    retval.y = a.y + b.y;
    return retval;
}

inline Tupl operator - (Tupl const &a, Tupl const &b)
{
    Tupl retval;
    retval.x = a.x - b.x;
    retval.y = a.y - b.y;
    return retval;
}

inline Tupl & operator += (Tupl &a, Tupl const &b)
{
    a.x += b.x;
    a.y += b.y;
    return a;
}

inline Tupl & operator += (Tupl &a, int b)
{
    a.x += b;
    a.y += b;
    return a;
}

inline Tupl & operator -= (Tupl &a, Tupl const &b)
{
    a.x -= b.x;
    a.y -= b.y;
    return a;
}

inline Tupl & operator *= (Tupl &a, Tupl const &b)
{
    a.x *= b.x;
    a.y *= b.y;
    return a;
}

inline Tupl & operator /= (Tupl &a, Tupl const &b)
{
    a.x /= b.x;
    a.y /= b.y;
    return a;
}

class ManipulatedObject
{
private:
    int m_value;

    // disable the copy constructor
    ManipulatedObject (const ManipulatedObject &ctor_arg);

public:
    ManipulatedObject () : m_value (0) {}
    void SetValue (int value) { m_value = value; }
    int GetValue () const { return m_value; }
};


class ReferenceManipulator
{
    ManipulatedObject m_obj;

public:
    ReferenceManipulator () {}
    virtual ~ReferenceManipulator () {}

    int manipulate_object () {
        do_manipulate_object (m_obj);
        return m_obj.GetValue ();
    }

    // -#- @obj(direction=inout) -#-
    virtual void do_manipulate_object (ManipulatedObject &obj) = 0;

};


class VectorLike
{
    std::vector<double> m_vec;

public:
    VectorLike ()
        {
        }
    // -#- name=__len__ -#-
    std::vector<double>::size_type get_len () const
        {
            return m_vec.size ();
        }
    // -#- name=__add__-#-
    VectorLike add_VectorLike(const VectorLike& rhs)
        {
            VectorLike result(*this);
            std::copy(rhs.m_vec.begin(), rhs.m_vec.end(), std::back_inserter(result.m_vec));
            return result;
        }
    // -#- name=__iadd__-#-
    VectorLike& iadd_VectorLike(const VectorLike& rhs)
        {
            std::copy(rhs.m_vec.begin(), rhs.m_vec.end(), std::back_inserter(m_vec));
            return *this;
        }
    // -#- name=__mul__-#-
    VectorLike mul_VectorLike(const unsigned n)
        {
            VectorLike result;
            if (n > 0)
            {
                result.m_vec.reserve(n * m_vec.size());
                for (unsigned i = 0; i != n; ++i)
                {
                    std::copy(m_vec.begin(), m_vec.end(), std::back_inserter(result.m_vec));
                }
            }
            return result;
        }
    // -#- name=__imul__-#-
    VectorLike& imul_VectorLike(const unsigned n)
        {
            if (n > 0)
            {
                const unsigned n0 = m_vec.size();
                m_vec.reserve(n * n0);
                for (unsigned i = 0; i < n - 1; ++i)
                {
                    std::copy(m_vec.begin(), m_vec.begin() + n0, std::back_inserter(m_vec));
                }
            }
            else
            {
                *this = VectorLike();
            }
            return *this;
        }
    // -#- name=__setitem__ -#-
    int set_item (int index, double value)
        {
            const int n = this->get_len();
            index = (index < 0 ? (n + 1 + index) : index);
            if (index >= n) return -1;
            m_vec[index] = value;
            return 0;
        }
    // -#- name=__getitem__ -#-
    double get_item (int index) const
        {
            const int n = this->get_len();
            index = (index < 0 ? (n + 1 + index) : index);
            try
            {
                return m_vec.at(index);
            }
            catch (std::out_of_range)
            {
                PyErr_SetString(PyExc_IndexError, "Container index out of range");
                return 0.0;
            }
        }
    // -#- name=__setslice__ -#-
    int set_slice (int index1, int index2, const VectorLike& values)
        {
            const int n = this->get_len();
            index1 = (index1 < 0 ? (n + 1 + index1) : index1);
            index2 = (index2 < 0 ? (n + 1 + index2) : index2);
            if (index1 >= n or index2 > n) return -1;
            for (int i = index1; i < index2; ++i)
            {
                if (this->set_item(i, values.get_item(i - index1)) != 0) return -1;
            }
            return 0;
        }
    // -#- name=__getslice__ -#-
    VectorLike get_slice (int index1, int index2) const
        {
            VectorLike result;
            const int n = this->get_len();
            index1 = (index1 < 0 ? (n + 1 + index1) : index1);
            index2 = (index2 < 0 ? (n + 1 + index2) : index2);
            for (int i = index1; i < index2; ++i)
            {
                result.m_vec.push_back(this->get_item(i));
            }
            return result;
        }
    // -#- name=__contains__ -#-
    int contains_value (double value) const
        {
            return ((std::find(m_vec.begin(), m_vec.end(), value) == m_vec.end()) ?
                    0 :
                    1);
        }

    void append (double value)
        {
            m_vec.push_back (value);
        }

    typedef std::vector<double>::iterator Iterator;
    Iterator Begin () { return m_vec.begin (); }
    Iterator End () { return m_vec.end (); }
};



class VectorLike2
{
    std::vector<double> m_vec;

public:
    VectorLike2 ()
        {
        }

    void append (double value)
        {
            m_vec.push_back (value);
        }

    typedef std::vector<double>::iterator Iterator;
    Iterator Begin () { return m_vec.begin (); }
    Iterator End () { return m_vec.end (); }
};


class MapLike
{
    std::map<int, double> m_map;

public:
    MapLike () {}

    void set (int key, double value) { m_map[key] = value; }

    typedef std::map<int, double>::iterator Iterator;
    Iterator Begin () { return m_map.begin (); }
    Iterator End () { return m_map.end (); }
};


// -#- exception -#-
struct Error
{
    std::string message;
};

// -#- exception -#-
struct DomainError : public Error
{
};


// returns 1/x, raises DomainError if x == 0
double my_inverse_func (double x) throw (DomainError);
double my_inverse_func2 (double x) throw (std::exception);

// the following function throws an exception but forgets to declare it
// -#- throw=std::exception -#-
double my_inverse_func3 (double x);

class ClassThatThrows
{
public:
    ClassThatThrows (double x) throw (DomainError);

    // returns 1/x, raises DomainError if x == 0
    double my_inverse_method (double x) throw (DomainError);
    double my_inverse_method2 (double x) throw (std::exception);

    // the following method throws an exception but forgets to declare it
    // -#- throw=std::exception -#-
    double my_inverse_method3 (double x);

    // -#- throw=std::out_of_range -#-
    int throw_out_of_range() throw (std::out_of_range);

    virtual int throw_error () const throw (std::exception);

    virtual ~ClassThatThrows () {}
};


// https://bugs.launchpad.net/pybindgen/+bug/450255
class ProtectedConstructor
{
public:
    ProtectedConstructor() {}
protected:
    // Not to be implemented
    ProtectedConstructor(ProtectedConstructor&);
    void operator=(ProtectedConstructor&);
};



template<typename ValueType>
class property
{
private:
    ValueType m_value;

public:
    property<ValueType> ()
    {
    }

    operator ValueType()

    {
        return m_value;
    }

    const ValueType &operator =(const ValueType &value)
    {
        m_value = value;
        return value;
    }
};

static inline std::string __property_inst()
{
    property<std::string> prop;
    prop = std::string("foo");
    return (std::string) prop;
}



class Box
{
    Foobar m_foobar;

public:
    static int instance_count;

    // -#- return_internal_reference=true -#-
    Foobar *m_internalFoobar;

    Box () { ++instance_count; m_internalFoobar = &m_foobar; }
    virtual ~Box () { --instance_count;}

    // -#- @return(reference_existing_object=true) -#-
    const Foobar* getFoobarInternalPtr () { return &m_foobar; }

    // -#- @return(reference_existing_object=true) -#-
    Foobar& getFoobarInternalRef () { return m_foobar; }

    // -#- @return(return_internal_reference=true) -#-
    Foobar* getFoobarInternalPtr2 () { return &m_foobar; }

    // -#- @return(return_internal_reference=true) -#-
    Foobar& getFoobarInternalRef2 () { return m_foobar; }

};


// multiple inheritance
class MIRoot
{
public:
    MIRoot () {}
    int root_method () const { return -1; }
};


class MIBase1 : public virtual MIRoot
{
    int m_value;
public:
    MIBase1 () : m_value (1) {}
    int base1_method () const { return m_value; }
};

class MIBase2 : public virtual MIRoot
{
    int m_value;
public:
    MIBase2 ()  : m_value (2) {}
    int base2_method () const { return m_value; }
};

class MIMixed : public MIBase1, public MIBase2
{
public:
    MIMixed () {}
    int mixed_method () const { return 3; }
};


Tupl my_throwing_func () throw (std::exception);


class IFoo
{
public:
    virtual void DoSomething() = 0;
protected:
    virtual ~IFoo() {}
};


class IFooImpl : public IFoo
{
public:
    virtual void DoSomething() {}
    ~IFooImpl() {}
};


int test_args_kwargs(const char *args, const char *kwargs);


struct RAStruct
{
  int a;
};


class ReturnConstRef
{
public:
  virtual const RAStruct & ReturnMyAStruct (void) = 0;
  virtual ~ReturnConstRef() {}
};


class RAReturnConstRef : public ReturnConstRef
{
public:
  RAReturnConstRef(int value) { m_astruct.a = value; }
  RAReturnConstRef() { m_astruct.a = 0; }
  const RAStruct & ReturnMyAStruct (void) { return m_astruct; }
  virtual ~RAReturnConstRef() {}

private:
  RAStruct m_astruct;
};

// -#- name=return_c_string_to_be_freed; @return(free_after_copy=true) -#-
char *return_c_string_to_be_freed(int size);

// -#- name=return_c_string_to_not_be_freed; @return(free_after_copy=false) -#-
char *return_c_string_to_not_be_freed(int size);

class ToBeFreed
{
public:
    ToBeFreed(int size);
    ~ToBeFreed();
    ToBeFreed(const ToBeFreed&);
    char *value();
private:
    char *m;
    int m_size;
};

// -#- name=return_class_to_be_freed; @return(free_after_copy=true) -#-
ToBeFreed *return_class_to_be_freed(int size);

// -#- name=return_class_to_not_be_freed; @return(free_after_copy=false) -#-
ToBeFreed *return_class_to_not_be_freed(int size);


void Add (const std::string filePath,
          double defaultZ = 0,
          char delimiter = ',');


#endif 	    /* !FOO_H_ */
