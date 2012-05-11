
/**
 * @file boost/reflect/reflect.hpp
 *
 * @brief Defines types and macros used to provide reflection.
 *
 */
#ifndef _BOOST_REFLECT_HPP_
#define _BOOST_REFLECT_HPP_

#include <boost/static_assert.hpp>
#include <boost/preprocessor/seq/for_each_i.hpp>
#include <boost/preprocessor/seq/seq.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/facilities/empty.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/type_traits/remove_pointer.hpp>
#include <string>
#include <typeinfo>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <stdint.h>
#include <boost/fusion/container/vector.hpp>
#include <boost/reflect/void.hpp>
#include <boost/reflect/vtable.hpp>
#include <boost/reflect/typeinfo.hpp>

namespace boost { namespace reflect {

/**
 *  @brief defines visit functions for T
 *  Unless this is specialized, visit() will not be defined for T.
 *
 *  The @ref BOOST_REFLECT(TYPE,INHERITS,MEMBERS) macro is used to specialize this
 *  class for your type.
 */
template<typename T>
struct reflector{
    typedef T type;
    typedef boost::fusion::vector<> base_class_types;
    typedef boost::false_type is_defined;
    template<typename Visitor>
    static inline void visit( const Visitor&  ){}; 
};

} } // namespace boost::reflect

#ifndef DOXYGEN

#define BOOST_REFLECT_VISIT_BASE(r, visitor, base) \
  boost::reflect::reflector<base>::visit( visitor );

#define BOOST_REFLECT_VISIT_MEMBER( r, visitor, elem ) \
  visitor.template operator()<BOOST_TYPEOF(type::elem),type,&type::elem>( BOOST_PP_STRINGIZE(elem) );

#define BOOST_REFLECT_DERIVED_IMPL_INLINE( TYPE, INHERITS, MEMBERS ) \
template<typename Visitor>\
static inline void visit( const Visitor& v ) { \
    BOOST_PP_SEQ_FOR_EACH( BOOST_REFLECT_VISIT_BASE, v, INHERITS ) \
    BOOST_PP_SEQ_FOR_EACH( BOOST_REFLECT_VISIT_MEMBER, v, MEMBERS ) \
} 

#define BOOST_REFLECT_DERIVED_IMPL_EXT( TYPE, INHERITS, MEMBERS ) \
template<typename Visitor>\
void boost::reflect::reflector<TYPE>::visit( const Visitor& v ) { \
    BOOST_PP_SEQ_FOR_EACH( BOOST_REFLECT_VISIT_BASE, v, INHERITS ) \
    BOOST_PP_SEQ_FOR_EACH( BOOST_REFLECT_VISIT_MEMBER, v, MEMBERS ) \
} 

#endif // DOXYGEN

/**
 *  @brief Specializes boost::reflect::reflector for TYPE where 
 *         type inherits other reflected classes
 *
 *  @param INHERITS - a sequence of base class names (basea)(baseb)(basec)
 *  @param MEMBERS - a sequence of member names.  (field1)(field2)(field3)
 */
#define BOOST_REFLECT_DERIVED( TYPE, INHERITS, MEMBERS ) \
BOOST_REFLECT_TYPEINFO(TYPE) \
namespace boost { namespace reflect { \
template<> struct reflector<TYPE> {\
    typedef TYPE type; \
    typedef boost::true_type is_defined; \
    BOOST_REFLECT_DERIVED_IMPL_INLINE( TYPE, INHERITS, MEMBERS ) \
}; } }


/**
 *  @brief Specializes boost::reflect::reflector for TYPE
 *
 *  @param MEMBERS - a sequence of member names.  (field1)(field2)(field3)
 *
 *  @see BOOST_REFLECT_DERIVED
 */
#define BOOST_REFLECT( TYPE, MEMBERS ) \
    BOOST_REFLECT_DERIVED( TYPE, BOOST_PP_SEQ_NIL, MEMBERS )

#define BOOST_REFLECT_FWD( TYPE ) \
BOOST_REFLECT_TYPEINFO(TYPE) \
namespace boost { namespace reflect { \
template<> struct reflector<TYPE> {\
    typedef TYPE type; \
    typedef boost::true_type is_defined; \
    template<typename Visitor> static void visit( const Visitor& v ); \
}; } }


#define BOOST_REFLECT_DERIVED_IMPL( TYPE, MEMBERS ) \
    BOOST_REFLECT_IMPL_DERIVED_EXT( TYPE, BOOST_PP_SEQ_NIL, MEMBERS )

#define BOOST_REFLECT_IMPL( TYPE, MEMBERS ) \
    BOOST_REFLECT_DERIVED_IMPL_EXT( TYPE, BOOST_PP_SEQ_NIL, MEMBERS )



#endif
