#include "h.h"

#include <iostream>

void H::Do (void)
{
  std::cout << "H::Do" << std::endl;
}

void H::Inner::Do (void)
{
  std::cout << "H::Inner::Do" << std::endl;
}

void H::Inner::MostInner::Do (void)
{
  std::cout << "H::Inner::MostInner::Do" << std::endl;
}

