#include <stdint.h>

class E
{
public:
  void Ref (void) const;
  void Unref (void) const;
  void Do (void);

  static E *CreateWithoutRef (void);
  static E *CreateWithRef (void);
private:
  E ();
  ~E ();
  mutable uint32_t m_count;
};
