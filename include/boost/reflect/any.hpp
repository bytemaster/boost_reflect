/**
 *  @file reflect.hpp
 *
 *  This class defines the macros and types used to implement
 *  an REFLECT interface.
 *
 */
#ifndef _BOOST_REFLECT_ANY_HPP_
#define _BOOST_REFLECT_ANY_HPP_
#include <boost/preprocessor/list/for_each.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/reflect/reflect.hpp>
#include <boost/mem_fn.hpp>
#include <boost/utility.hpp>
#include <boost/any.hpp>
#include <functional>
#include <boost/functional.hpp>
#include <boost/reflect/mirror_interface.hpp>

namespace boost { namespace reflect {

template<typename T, typename InterfaceDelegate = boost::reflect::mirror_interface>
class any {};

} } // namespace boost::reflect

#define PUBLIC_BASE( r, data, elem )   public boost::reflect::any<elem,data>,
#define BASE_INIT( r, data, elem )     elem<InterfaceDelegate>::__reflect__init(data);
#define INIT_MEMBER( r, data, elem )   elem.set_delegate( v, &T::elem );
#define DEFINE_MEMBER( r, data, elem ) \
        typename InterfaceDelegate::template calculate_type<BOOST_TYPEOF(&reflect_definition_class::elem)>::type  elem;

#define VISIT_BASE( r, data, elem ) any<elem,InterfaceDelegate>::__reflect__visit(data);
#define VISIT_MEMBER( r, data, elem ) t.template accept<data>( elem, BOOST_PP_STRINGIZE(elem) );


/**
 *  @todo 
 *      Move much of this macro into InterfaceDelegate::implementation_base<> then I can simplify
 *      the macro to just use the base implementation of operator= and init()
 *
 *      This will simplify the macro expansion and give more control to the InterfaceDelegate
 *
 *      Then I can move each function object into its own "base class" that can be selectively
 *      included based upon compile-time info, such as constness.   
 *   
 */
#define BOOST_REFLECT_BASE BOOST_PP_NIL
#define BOOST_REFLECT_ANY( NAME, INHERITS, MEMBERS ) \
BOOST_REFLECT( NAME, INHERITS, MEMBERS ) \
namespace boost { namespace reflect { \
template<typename InterfaceDelegate > \
class any<NAME,InterfaceDelegate> : BOOST_PP_SEQ_FOR_EACH( PUBLIC_BASE, InterfaceDelegate, INHERITS ) \
                                    public InterfaceDelegate::template base_class< NAME > \
{ \
    public: \
        typedef NAME reflect_definition_class; \
        any(){} \
        template<typename T> \
        any( T* v )  \
        :boost::any(v) \
        { \
            typename InterfaceDelegate::set_any_visitor vis; \
            boost::reflect::reflector<NAME>::visit( *this, *v, vis, -1 ); \
        }\
        template<typename T> \
        any( boost::shared_ptr<T> v )  \
        :boost::any(v) \
        { \
            typename InterfaceDelegate::set_any_visitor vis; \
            boost::reflect::reflector<NAME>::visit( *this, *v, vis, -1 ); \
        }\
        template<typename T> \
        any( T v )  \
        :boost::any(v) \
        { \
            typename InterfaceDelegate::set_any_visitor vis; \
            boost::reflect::reflector<NAME>::visit( *this, boost::any_cast<T&>(*this), vis, -1 ); \
        }\
        BOOST_PP_SEQ_FOR_EACH( DEFINE_MEMBER, NAME, MEMBERS ) \
        template<typename T> \
        any& operator=( boost::shared_ptr<T> v ) \
        { \
            boost::any::operator=(v); \
            typename InterfaceDelegate::set_any_visitor vis; \
            boost::reflect::reflector<NAME>::visit( *this, *v, vis, -1 ); \
            return *this; \
        }\
        template<typename T> \
        any& operator=( T* v ) {\
            boost::any::operator=(v); \
            if( v ) { \
            typename InterfaceDelegate::set_any_visitor vis; \
            boost::reflect::reflector<NAME>::visit( *this, *v, vis, -1 ); \
            } \
            return *this; \
        }\
        template<typename T> \
        any& operator=( const T& v ) { \
            boost::any::operator=(v); \
            typename InterfaceDelegate::set_any_visitor vis; \
            boost::reflect::reflector<NAME>::visit( *this, boost::any_cast<T&>(*this), vis, -1 ); \
            return *this; \
        }\
        any& operator=( const any& v ) { \
            if( this == &v ) return *this; \
            boost::any::operator=(v); \
            typename InterfaceDelegate::set_any_visitor vis; \
            boost::reflect::reflector<NAME>::visit( *this, boost::any_cast<any&>(*this), vis, -1 );return *this; } \
}; \
} } 

#endif
