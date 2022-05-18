#include "g.h"

#include <iostream>

void GDoA (void)
{
  std::cout << "GDoA" << std::endl;
}

void G::GDoB (void)
{
  std::cout << "G::GDoB" << std::endl;
}

void G::GInner::GDoC (void)
{
  std::cout << "G::Inner::GDoC" << std::endl;
}

