#include "c.h"
#include <iostream>

C::C ()
  : m_c (0)
{
  std::cout << "C::C" << std::endl;
}
C::C (uint32_t c)
  : m_c (c)
{
  std::cout << "C::C (uint32_t)" << std::endl;
}
C::~C ()
{
  std::cout << "C::~C" << std::endl;
}

void 
C::DoA (void)
{
  std::cout << "C::DoA" << std::endl;
}
void 
C::DoB (void)
{
  std::cout << "C::DoB" << std::endl;
}

void 
C::DoC (uint32_t c)
{
  std::cout << "C::DoC=" << c << std::endl;
  m_c = c;
}
uint32_t 
C::DoD (void)
{
  std::cout << "C::DoD" << std::endl;
  return m_c;
}

void 
C::DoE (void)
{
  std::cout << "C::DoE" << std::endl;
}
