#include "hello.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Bool hello_print_message(const char *message)
{
    printf("Hello: %s\n", message);
    return 0;
}

double hello_sum(double x, double y)
{
    return x + y;
}


struct _HelloFoo
{
    int refcount;
    char *data;
};

HelloFoo*
hello_foo_new(void)
{
    HelloFoo *foo;
    foo = (HelloFoo *) malloc(sizeof(HelloFoo));
    foo->refcount = 1;
    foo->data = NULL;
    return foo;
}

HelloFoo*
hello_foo_new_from_data(const char *data)
{
    HelloFoo* foo;

    foo = hello_foo_new();
    hello_foo_set_data(foo, data);
    return foo;
}

HelloFoo*
hello_foo_new_with_spaces (int num_spaces)
{
    int i;
    HelloFoo *foo;
    foo = hello_foo_new();
    foo->data = malloc(num_spaces + 1);
    for (i = 0; i < num_spaces; i++)
	foo->data[i] = ' ';
    foo->data[i] = '\0';
    return foo;
}

void
hello_foo_ref(HelloFoo *foo)
{
    foo->refcount++;
}

void
hello_foo_unref(HelloFoo *foo)
{
    if (--foo->refcount > 0)
        return;
    
    if (foo->data)
	    free(foo->data);
    free(foo);
}

void
hello_foo_set_data(HelloFoo   *foo,
		   const char *data)
{
    if (foo->data)
	free(foo->data);
    foo->data = strdup(data);
}

const char *
hello_foo_get_data(HelloFoo *foo)
{
    return foo->data;
}

const HelloFoo* hello_foo_get_self  (HelloFoo *foo)
{
    return foo;
}

int hello_get_hash  (const HelloFoo *foo)
{
    if (foo)
    {
        return (int) (long) foo;
    } else {
        return -1;
    }
}

