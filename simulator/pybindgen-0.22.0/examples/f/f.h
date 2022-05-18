class FBase
{
public:
  virtual ~FBase ();
  virtual void DoA (void) = 0;
  void DoB (void);
 private:
  virtual void PrivDoB (void) = 0;
};
