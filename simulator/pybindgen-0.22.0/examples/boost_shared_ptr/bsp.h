// -*- Mode: C++; c-file-style: "stroustrup"; indent-tabs-mode:nil; -*-
#ifndef   	BSP_H_
# define   	BSP_H_

#include <string>
#include <boost/shared_ptr.hpp>


class Foo
{
    std::string m_datum;
public:

    Foo () : m_datum ("") {}

    Foo (std::string const &datum) : m_datum (datum) {}

    const std::string get_datum () const { return m_datum; }
    
    void set_datum (std::string const &datum) { m_datum = datum; }

    virtual ~Foo() {}

};

void function_that_takes_foo (boost::shared_ptr<Foo> foo);
boost::shared_ptr<Foo> function_that_returns_foo ();

#endif 	    /* !FOO_H_ */
