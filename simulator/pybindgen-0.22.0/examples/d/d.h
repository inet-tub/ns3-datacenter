struct D
{
  bool d;
};


struct D *DCreate (void);
void DDoA (struct D *d);
void DDoB (struct D &d);
void DDoC (const struct D &d);
void DDestroy (struct D *d);
