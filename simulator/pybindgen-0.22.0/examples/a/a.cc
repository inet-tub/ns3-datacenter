#include "a.h"
#include <iostream>

void ADoA (void)
{
  std::cout << "ADoA" << std::endl;
}

void ADoB (uint32_t b)
{
  std::cout << "ADoB=" << b << std::endl;
}

uint32_t ADoC (void)
{
  std::cout << "ADoC" << std::endl;
  return 1;
}
