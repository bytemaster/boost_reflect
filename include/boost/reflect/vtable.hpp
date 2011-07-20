/**
 *  @file reflect.hpp
 *
 *  This class defines the macros and types used to implement
 *  an REFLECT interface.
 *
 */
#ifndef _BOOST_REFLECT_VTABLE_HPP_
#define _BOOST_REFLECT_VTABLE_HPP_
#include <boost/preprocessor/list/for_each.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/reflect/reflect.hpp>

namespace boost { namespace reflect {

    template<typename T>
    struct vtable_base{ };
    struct mirror_interface;

    template<typename T, typename InterfaceDelegate = boost::reflect::mirror_interface>
    class vtable {};

} } // namespace boost::reflect

#define PUBLIC_BASE( r, data, elem )  boost::reflect::vtable<elem,data>,
#define DEFINE_MEMBER( r, data, elem ) \
        struct BOOST_PP_CAT( __reflect__, elem) : public InterfaceDelegate::template calculate_type<BOOST_TYPEOF(&reflect_definition_class::elem)>::type  \
        {  \
            typedef typename InterfaceDelegate::template calculate_type<BOOST_TYPEOF(&reflect_definition_class::elem)>::type base_type;  \
            using base_type::operator=;\
            static const char* name() { return BOOST_PP_STRINGIZE(data); } \
            template<typename Type, typename AssignType> \
            static void get_member_ptr( AssignType v ) {  v = &Type::elem; } \
        } elem;


#define BOOST_REFLECT_BASE BOOST_PP_SEQ_NIL
#define BOOST_REFLECT_ANY( NAME, INHERITS, MEMBERS ) \
BOOST_REFLECT( NAME, INHERITS, MEMBERS ) \
namespace boost { namespace reflect { \
template<typename InterfaceDelegate > \
struct vtable<NAME,InterfaceDelegate> :BOOST_PP_SEQ_FOR_EACH( PUBLIC_BASE, InterfaceDelegate, INHERITS ) vtable_base<vtable<NAME,InterfaceDelegate> > { \
    typedef NAME reflect_definition_class; \
    BOOST_PP_SEQ_FOR_EACH( DEFINE_MEMBER, NAME, MEMBERS ) \
}; \
\
} } 


#endif
