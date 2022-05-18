// -*- Mode: C++; c-file-style: "stroustrup"; indent-tabs-mode:nil; -*-
#include "testapi.h"

#include <boost/python.hpp>

using namespace boost::python;

void (Multiplier::*factor1) () = &Multiplier::SetFactor;
void (Multiplier::*factor2) (double) = &Multiplier::SetFactor;

struct MultiplierWrap : Multiplier, wrapper<Multiplier>
{
    MultiplierWrap () : Multiplier ()
        {
        }
    MultiplierWrap (double factor) : Multiplier (factor)  
        {
        }
    
    double Multiply (double value) const
    {
        return this->get_override("Multiply") (value);
    }

    double default_Multiply (double value) const
    {
        return this->Multiplier::Multiply (value);
    }

};

BOOST_PYTHON_MODULE(testapi_boost)
{
    def("func1", func1);
    def("func2", func2);
    def("call_virtual_from_cpp", call_virtual_from_cpp);

    class_<MultiplierWrap, boost::noncopyable>("Multiplier")
        .def(init<double>())
        .def("GetFactor", &Multiplier::GetFactor) 
        .def("SetFactor", factor1)
        .def("SetFactor", factor2)
        .def("Multiply", &Multiplier::Multiply, &MultiplierWrap::default_Multiply)
        ;
    
}

