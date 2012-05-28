
/**
 * @file mace/reflect/reflect.hpp
 *
 * @brief Defines types and macros used to provide reflection.
 *
 */
#ifndef _MACE_REFLECT_HPP_
#define _MACE_REFLECT_HPP_

#include <boost/static_assert.hpp>
#include <boost/preprocessor/seq/for_each_i.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/enum.hpp>
#include <boost/preprocessor/seq/seq.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/facilities/empty.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/type_traits/remove_pointer.hpp>
#include <string>
#include <typeinfo>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <stdint.h>
#include <boost/fusion/container/vector.hpp>
#include <boost/function_types/result_type.hpp>


#include <mace/reflect/void.hpp>
#include <mace/reflect/typeinfo.hpp>

namespace mace { namespace reflect {

/**
 *  @brief defines visit functions for T
 *  Unless this is specialized, visit() will not be defined for T.
 *
 *  The @ref MACE_REFLECT(TYPE,INHERITS,MEMBERS) macro is used to specialize this
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

} } // namespace mace::reflect

#ifndef DOXYGEN

#define MACE_REFLECT_VISIT_BASE(r, visitor, base) \
  mace::reflect::reflector<base>::visit( visitor );

#define MACE_REFLECT_VISIT_MEMBER( r, visitor, elem ) \
  visitor.template operator()<BOOST_TYPEOF(&type::elem), &type::elem>( BOOST_PP_STRINGIZE(elem) );

#define MACE_REFLECT_BASE_MEMBER_COUNT( r, OP, elem ) \
  OP mace::reflect::reflector<elem>::member_count

#define MACE_REFLECT_DERIVED_IMPL_INLINE( TYPE, INHERITS, MEMBERS ) \
template<typename Visitor>\
static inline void visit( const Visitor& v ) { \
    BOOST_PP_SEQ_FOR_EACH( MACE_REFLECT_VISIT_BASE, v, INHERITS ) \
    BOOST_PP_SEQ_FOR_EACH( MACE_REFLECT_VISIT_MEMBER, v, MEMBERS ) \
} 

#define MACE_REFLECT_DERIVED_IMPL_EXT( TYPE, INHERITS, MEMBERS ) \
template<typename Visitor>\
void mace::reflect::reflector<TYPE>::visit( const Visitor& v ) { \
    BOOST_PP_SEQ_FOR_EACH( MACE_REFLECT_VISIT_BASE, v, INHERITS ) \
    BOOST_PP_SEQ_FOR_EACH( MACE_REFLECT_VISIT_MEMBER, v, MEMBERS ) \
} 

#endif // DOXYGEN

/**
 *  @brief Specializes mace::reflect::reflector for TYPE where 
 *         type inherits other reflected classes
 *
 *  @param INHERITS - a sequence of base class names (basea)(baseb)(basec)
 *  @param MEMBERS - a sequence of member names.  (field1)(field2)(field3)
 */
#define MACE_REFLECT_DERIVED( TYPE, INHERITS, MEMBERS ) \
MACE_REFLECT_TYPEINFO(TYPE) \
namespace mace { namespace reflect { \
template<> struct reflector<TYPE> {\
    typedef TYPE type; \
    typedef boost::true_type is_defined; \
    enum  member_count_enum {  \
      local_member_count = BOOST_PP_SEQ_SIZE(MEMBERS), \
      total_member_count = local_member_count BOOST_PP_SEQ_FOR_EACH( MACE_REFLECT_BASE_MEMBER_COUNT, +, INHERITS )\
    }; \
    MACE_REFLECT_DERIVED_IMPL_INLINE( TYPE, INHERITS, MEMBERS ) \
}; } }


/**
 *  @brief Specializes mace::reflect::reflector for TYPE
 *
 *  @param MEMBERS - a sequence of member names.  (field1)(field2)(field3)
 *
 *  @see MACE_REFLECT_DERIVED
 */
#define MACE_REFLECT( TYPE, MEMBERS ) \
    MACE_REFLECT_DERIVED( TYPE, BOOST_PP_SEQ_NIL, MEMBERS )

#define MACE_REFLECT_FWD( TYPE ) \
MACE_REFLECT_TYPEINFO(TYPE) \
namespace mace { namespace reflect { \
template<> struct reflector<TYPE> {\
    typedef TYPE type; \
    typedef boost::true_type is_defined; \
    enum  member_count_enum {  \
      local_member_count = BOOST_PP_SEQ_SIZE(MEMBERS), \
      total_member_count = local_member_count BOOST_PP_SEQ_FOR_EACH( MACE_REFLECT_BASE_MEMBER_COUNT, +, INHERITS )\
    }; \
    typedef boost::fusion::vector<BOOST_PP_SEQ_ENUM(INHERITS)> base_class_types; \
    template<typename Visitor> static void visit( const Visitor& v ); \
}; } }


#define MACE_REFLECT_DERIVED_IMPL( TYPE, MEMBERS ) \
    MACE_REFLECT_IMPL_DERIVED_EXT( TYPE, BOOST_PP_SEQ_NIL, MEMBERS )

#define MACE_REFLECT_IMPL( TYPE, MEMBERS ) \
    MACE_REFLECT_DERIVED_IMPL_EXT( TYPE, BOOST_PP_SEQ_NIL, MEMBERS )



#endif
