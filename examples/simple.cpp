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
