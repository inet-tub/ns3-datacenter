#include <iostream>

#include "a.h"

class Derived: public Base {
public:
  Derived(): Base() { std::cerr << "Derived::Derived()" << std::endl; }
  virtual ~Derived() { std::cerr << "Derived::~Derived()" << std::endl; }
  virtual void do_something() const { std::cerr << "Derived::do_something()" << std::endl; }
};

