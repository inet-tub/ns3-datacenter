#include "f.h"
#include <iostream>

FBase::~FBase ()
{}
void
FBase::DoB (void)
{
  std::cout << "FBase::DoB" << std::endl;
  PrivDoB ();
}
