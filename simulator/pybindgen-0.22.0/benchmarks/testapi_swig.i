/* example.i */

%module(directors="1") testapi_swig
%feature("director");         

%{

#include "testapi.h"

%}

extern void func1(void);
extern double func2(double x, double y, double z);

class Multiplier
{
    double m_factor;
    
public:
    Multiplier ();
    virtual ~Multiplier ();
    Multiplier (double factor);
    void SetFactor (double f);
    void SetFactor (void);
    double GetFactor () const;
    virtual double Multiply (double value) const;
};

extern double call_virtual_from_cpp (Multiplier const *obj, double value);

