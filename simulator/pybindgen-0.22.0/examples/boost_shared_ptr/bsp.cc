#include "bsp.h"

boost::shared_ptr<Foo> g_foo;

void function_that_takes_foo(boost::shared_ptr<Foo> foo)
{
    g_foo = foo;
}

boost::shared_ptr<Foo> function_that_returns_foo()
{
    return g_foo;
}



