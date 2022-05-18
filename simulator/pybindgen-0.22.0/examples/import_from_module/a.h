#include <iostream>

class Base {
public:
  Base() { std::cerr << "Base::Base()" << std::endl; }
  virtual ~Base() { std::cerr << "Base::~Base()" << std::endl; }
  virtual void do_something() const { std::cerr << "Base::do_something()" << std::endl; }
};

