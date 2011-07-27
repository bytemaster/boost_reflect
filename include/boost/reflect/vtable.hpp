/**
 *  @file reflect.hpp
 *
 *  This class defines the macros and types used to implement
 *  an REFLECT interface.
 *
 */
#ifndef _BOOST_REFLECT_VTABLE_HPP_
#define _BOOST_REFLECT_VTABLE_HPP_
#include <boost/preprocessor/arithmetic/inc.hpp>
#include <boost/preprocessor/seq/push_front.hpp>
#include <boost/preprocessor/seq/seq.hpp>
#include <boost/preprocessor/seq/enum.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/comparison/equal.hpp>
#include <boost/preprocessor/seq/transform.hpp>
#include <boost/preprocessor/seq/to_tuple.hpp>
#include <boost/preprocessor/tuple/to_list.hpp>
#include <boost/preprocessor/list/enum.hpp>

#include <boost/preprocessor/stringize.hpp>
#include <boost/reflect/reflect.hpp>

namespace boost { namespace reflect {

  struct mirror_interface;
  template<typename t>
  class vtable_base{};

  /**
   *  @brief Contains functors defined by InterfaceDelegate for each reflected member of T
   *
   *  Use the @ref BOOST_REFLECT_ANY(NAME,INHERITS,MEMBERS) to define the vtable for your
   *  type.
   */
  template<typename T = void, typename InterfaceDelegate = boost::reflect::mirror_interface>
  class vtable {};

  template<typename T> 
  struct vtable_reflector {
    template<typename Visitor, typename InterfaceDelegate>
    static void visit( const boost::reflect::vtable<T,InterfaceDelegate>* vtbl, const Visitor& v ) {}
  };
} } // namespace boost::reflect

#define BOOST_REFLECT_VTABLE_PUBLIC_BASE( r, data, elem )  boost::reflect::vtable<elem,data>,

#define BOOST_REFLECT_VTABLE_DEFINE_MEMBER( r, data, elem ) \
  struct BOOST_PP_CAT( __reflect__, elem) : \
    public InterfaceDelegate::template calculate_type<BOOST_TYPEOF(&interface_type::elem)>::type  \
  {  \
      typedef typename InterfaceDelegate::template calculate_type<BOOST_TYPEOF(&interface_type::elem)>::type base_type;  \
      using base_type::operator=;\
      static const char* name() { return BOOST_PP_STRINGIZE(data); } \
      template<typename Type, typename AssignType> \
      static void get_member_ptr( AssignType v ) {  v = &Type::elem; } \
  } elem;


#define BOOST_REFLECT_VTABLE_VISIT_BASE( r, visitor, name ) \
  vtable_reflector<name>::visit( (const boost::reflect::vtable<name,InterfaceDelegate>*)vtbl, visitor );

#define BOOST_REFLECT_VTABLE_VISIT_MEMBER( r, visitor, elem ) \
  visitor( &vtable_type::elem, BOOST_PP_STRINGIZE(elem) );

#define BOOST_REFLECT_SEQ_ENUM(X) \
BOOST_PP_LIST_ENUM( \
  BOOST_PP_LIST_REST( \
    BOOST_PP_TUPLE_TO_LIST( BOOST_PP_INC(BOOST_PP_SEQ_SIZE(X)), BOOST_PP_SEQ_TO_TUPLE( \
    BOOST_PP_SEQ_TRANSFORM( BOOST_REFLECT_VTABLE_PUBLIC_BASE, InterfaceDelegate, BOOST_PP_SEQ_PUSH_FRONT(X,(null)) ) ) )  \
  ) \
)

#define BOOST_REFLECT_BASE BOOST_PP_SEQ_NIL
#define BOOST_REFLECT_ANY_DERIVED( NAME, INHERITS, MEMBERS ) \
BOOST_REFLECT_DERIVED( NAME, INHERITS, MEMBERS ) \
namespace boost { namespace reflect { \
template<typename InterfaceDelegate > \
struct vtable<NAME,InterfaceDelegate> : BOOST_PP_SEQ_FOR_EACH( BOOST_REFLECT_VTABLE_PUBLIC_BASE, InterfaceDelegate, INHERITS ) private vtable_base<NAME> { \
    typedef NAME interface_type; \
    BOOST_PP_SEQ_FOR_EACH( BOOST_REFLECT_VTABLE_DEFINE_MEMBER, NAME, MEMBERS ) \
}; \
template<> struct vtable_reflector<NAME> { \
    typedef NAME interface_type; \
    template<typename Visitor, typename InterfaceDelegate> \
    static void visit( const boost::reflect::vtable<NAME,InterfaceDelegate>* vtbl,  \
                       const Visitor& visitor ) { \
        typedef boost::reflect::vtable<NAME,InterfaceDelegate> vtable_type; \
        BOOST_PP_SEQ_FOR_EACH( BOOST_REFLECT_VTABLE_VISIT_BASE, visitor, INHERITS ) \
        BOOST_PP_SEQ_FOR_EACH( BOOST_REFLECT_VTABLE_VISIT_MEMBER, visitor, MEMBERS ) \
    } \
};\
\
} } 

#define BOOST_REFLECT_ANY( NAME, MEMBERS ) \
    BOOST_REFLECT_ANY_DERIVED( NAME, BOOST_PP_SEQ_NIL, MEMBERS )

#endif
