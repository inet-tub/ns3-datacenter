#include "c.h"

static const int LEN = 1024*1024;

static unsigned short int buffer[LEN] = {0,};

unsigned short int* GetBuffer()
{
    return buffer;
}

int GetBufferLen()
{
    return LEN;
}

unsigned short int GetBufferChecksum()
{
    unsigned short int sum = 0;
    for (int i = 0; i < LEN; sum += buffer[i++]);
    return sum;
}



