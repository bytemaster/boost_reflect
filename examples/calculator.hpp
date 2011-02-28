#ifndef _BOOST_IDL_CALCULATOR_HPP
#define _BOOST_IDL_CALCULATOR_HPP

#include <boost/idl/mirror_interface.hpp>
#include <boost/idl/idl.hpp>
struct Service
{
    std::string name()const;
    int         exit();
};
struct Calculator : Service
{
    double add( double v );           
    double add2( double v, double v2 );
    double sub( double v );           
    double mult( double v );           
    double div( double v );           
    double result()const;
};

BOOST_IDL_INTERFACE( Service, BOOST_PP_SEQ_NIL, (name)(exit) )
BOOST_IDL_INTERFACE( Calculator, (Service), (add)(add2)(sub)(mult)(div)(result) )

#endif // _BOOST_IDL_CALCULATOR_HPP
