#include <stdint.h>

class C
{
public:
  C ();
  C (uint32_t c);
  virtual ~C ();

  static void DoA (void);
  void DoB (void);
  void DoC (uint32_t c);
  uint32_t DoD (void);
  virtual void DoE (void);
private:
  uint32_t m_c;
};
