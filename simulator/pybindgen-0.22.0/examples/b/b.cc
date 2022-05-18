#include "b.h"
#include <iostream>

void BDoA (struct B b)
{
  std::cout << "BDoA b_a=" << b.b_a << ", b_b=" << b.b_b << std::endl;
}
struct B BDoB (void)
{
  std::cout << "BDoB" << std::endl;
  struct B b;
  b.b_a = 1;
  b.b_b = 2;
  return b;
}
