// -*- Mode: C++; c-file-style: "stroustrup"; indent-tabs-mode:nil; -*-
#ifndef   	SP_H_
# define   	SP_H_

#include <string>
#include <iostream>
#include <memory>


class Foo
{
    std::string m_datum;
public:

    Foo () : m_datum ("") {
        std::cout << "Created empty foo" << std::endl;
    }

    Foo (std::string const &datum) : m_datum (datum) {
        std::cout << "Created foo with datum " << datum << std::endl;
    }

    const std::string get_datum () const { return m_datum; }
    
    void set_datum (std::string const &datum) { m_datum = datum; }

    virtual ~Foo() {
        std::cout << "Destroyed foo with datum " << m_datum << std::endl;
    }

};

void function_that_takes_foo (std::shared_ptr<Foo> foo);
std::shared_ptr<Foo> function_that_returns_foo ();

#endif 	    /* !FOO_H_ */
