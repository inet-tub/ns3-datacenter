#include "e.h"
#include <iostream>

E::E ()
  : m_count (0)
{
  std::cout << "E::E" << std::endl;
}
void 
E::Ref (void) const
{
  std::cout << "E::Ref" << std::endl;
  m_count++;
}
void 
E::Unref (void) const
{
  std::cout << "E::Unref" << std::endl;
  m_count--;
  if (m_count == 0)
    {
      delete this;
    }
}
void 
E::Do (void)
{
  std::cout << "E::Do" << std::endl;
}
E::~E ()
{
  std::cout << "E::~E" << std::endl;
}

E *
E::CreateWithoutRef (void)
{
  std::cout << "E::CreateWithoutRef" << std::endl;
  return new E ();
}

E *
E::CreateWithRef (void)
{
  std::cout << "E::CreateWithRef" << std::endl;
  E *e = new E ();
  e->Ref ();
  return e;
}
