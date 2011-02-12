#include <boost/idl/mirror_interface.hpp>
#include <boost/idl/idl.hpp>

namespace idl_definition {
    struct BaseShape
    {
        std::string name()const;
    };
    struct Shape : BaseShape
    {
        double area()const;
    };
}

/*
template<typename InterfaceDelegate = boost::idl::mirror_interface >
class InterfaceName 
{
    public:
       struct __idl_area : public InterfaceDelegate::template calculate_type<&idl_definition::InterfaceName::area>::type 
       {
            template<typename T> 
            static BOOST_TYPEOF(&T::area) get_member_on_type() 
            {
               return &T::area; 
            }
       } area;
};
*/
BOOST_IDL_INTERFACE( BaseShape, BOOST_PP_NIL, (name) )
BOOST_IDL_INTERFACE( Shape, (BaseShape, BOOST_PP_NIL), (area) )

class Circle
{
    public:
        double area()const { return 3.1415; }
        std::string name()const { return "Circle"; }
};


int main( int argc, char** argv )
{
    Circle c;
    Shape<> s(&c);
    printf( "%f\n", s.area() );
    return 0;
}
