#ifndef __MACE_REFLECT_VOID_HPP
#define __MACE_REFLECT_VOID_HPP
#include <iostream>

namespace mace { namespace reflect {
    /**
     * @brief A type to replace void in generic code.
     *
     * void cannot be treated like any other type and thus always introduces
     * many 'special cases' to generic code.  This type is used in generic code any
     * all functors that return void can be adapted to return void_t before being
     * used with generic code via boost::reflect::adapt_void.
     */
    struct void_t{
        friend std::ostream& operator<<(std::ostream& os,const void_t&) {return os;}
        friend std::istream& operator>>(std::istream& is,void_t&)       {return is;}
    };

} }

#endif
