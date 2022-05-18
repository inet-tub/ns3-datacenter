#ifndef __HELLO_H__
#define __HELLO_H__

#ifdef __cplusplus
extern "C"
{
#endif

typedef int Bool;

Bool   hello_print_message (const char *message);
double hello_sum           (double      x,
                            double      y);

typedef struct _HelloFoo HelloFoo;


HelloFoo*    hello_foo_new             (void);
HelloFoo*    hello_foo_new_from_data   (const char *data);
HelloFoo*    hello_foo_new_with_spaces (int         num_spaces);
void         hello_foo_ref             (HelloFoo   *foo);
void         hello_foo_unref           (HelloFoo   *foo);
void         hello_foo_set_data        (HelloFoo   *foo,
                                        const char *data);
const char * hello_foo_get_data        (HelloFoo   *foo);

const HelloFoo* hello_foo_get_self  (HelloFoo *foo);


/* -#- @foo(null_ok=true, default_value=NULL) -#- */
int hello_get_hash  (const HelloFoo *foo);


#ifdef __cplusplus
}
#endif

#endif /* __HELLO_H__ */
