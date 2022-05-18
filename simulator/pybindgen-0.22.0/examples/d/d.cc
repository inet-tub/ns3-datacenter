#include "d.h"
#include <iostream>

struct D *DCreate (void)
{
  std::cout << "DCreate" << std::endl;
  return new D ();
}
void DDoA (struct D *d)
{
  std::cout << "DDoA" << std::endl;
  d++;
}
void DDoB (struct D &d)
{
  std::cout << "DDoB" << std::endl;
  d.d++;
}
void DDoC (const struct D &d)
{
  std::cout << "DDoC: " << d.d << std::endl;
}
void DDestroy (struct D *d)
{
  std::cout << "DDestroy" << std::endl;
  d++;
}
