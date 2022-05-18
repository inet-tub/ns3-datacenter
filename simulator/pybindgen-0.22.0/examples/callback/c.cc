#include "c.h"

void visit (Visitor visitor, void *data)
{
    for (int i = 0; i < 10; i++)
        visitor (i, data);
}


