// -*- Mode: C++; c-file-style: "stroustrup"; indent-tabs-mode:nil; -*-

%Module testapi_sip 0


void func1 ();
double func2 (double x, double y, double z);


class Multiplier
{

%TypeHeaderCode
#include "testapi.h"
%End
    
public:
    Multiplier ();
    Multiplier (double factor);
    virtual ~Multiplier ();
    
    void SetFactor (double f);
    void SetFactor ();
    double GetFactor () const;
    virtual double Multiply (double value) const;
};

double call_virtual_from_cpp (const Multiplier *obj, double value);
