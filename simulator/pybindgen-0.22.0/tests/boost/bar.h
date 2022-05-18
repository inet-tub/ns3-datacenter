// -*- Mode: C++; c-file-style: "stroustrup"; indent-tabs-mode:nil; -*-
#ifndef   	BAR_H_
# define   	BAR_H_

#include <string>
#include <boost/shared_ptr.hpp>


class Foo
{
    std::string m_datum;
    bool m_initialized;
public:
    static int instance_count;

    Foo () : m_datum (""), m_initialized (false)
        { Foo::instance_count++; }

    Foo (std::string const &datum) : m_datum (datum), m_initialized (false)
        { Foo::instance_count++; }
    const std::string get_datum () const { return m_datum; }

    Foo (Foo const & other) : m_datum (other.get_datum ()), m_initialized (false)
        { Foo::instance_count++; }

    void initialize () { m_initialized = true; }
    bool is_initialized () const { return m_initialized; }

    virtual ~Foo() { Foo::instance_count--; }

};

inline std::ostream & operator << (std::ostream &os, Foo const &foo)
{
    os << foo.get_datum ();
    return os;
}


void function_that_takes_foo (boost::shared_ptr<Foo> foo);
boost::shared_ptr<Foo> function_that_returns_foo ();


class ClassThatTakesFoo
{
    boost::shared_ptr<Foo> m_foo;
public:
    ClassThatTakesFoo(boost::shared_ptr<Foo> foo) : m_foo(foo) {}
    boost::shared_ptr<Foo> get_foo () const { return m_foo; }

    virtual boost::shared_ptr<Foo> get_modified_foo (boost::shared_ptr<Foo> foo) const { return m_foo; }
    virtual ~ClassThatTakesFoo() {}
};


#endif 	    /* !FOO_H_ */
