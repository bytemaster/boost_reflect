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

namespace boost { namespace reflect {

template<typename T>
struct get_typeinfo { 
    enum is_defined_enum{ is_defined = 0 };
    static const char* name() { return typeid(T).name(); }
};

/**
 *  Removes all decorations and returns the typename.
 */
template<typename T>
inline const char* get_typename() 
{ 
    return get_typeinfo<typename boost::remove_const<
                        typename boost::remove_reference<
                        typename boost::remove_pointer<T>::type>::type>::type>::name();
}

// used to specify templay get_typeinfo
struct dummy_arg{};

template<typename TP, template<typename> class C> 
struct get_typeinfo< C<TP> > { 
    enum is_defined_enum{ is_defined = get_typeinfo<TP>::is_defined }; 
    static const char* name() 
    {  
        static std::string n = std::string( get_typename<C<dummy_arg> >() ) + std::string("<") + std::string(get_typename<TP>()) +  ">"; 
        return n.c_str(); 
    } 
}; 

template<typename TP, typename TP2, template<typename, typename> class C> 
struct get_typeinfo< C<TP,TP2> > { 
    enum is_defined_enum{ is_defined = get_typeinfo<TP>::is_defined && get_typeinfo<TP2>::is_defined }; 
    static const char* name() 
    {  
        static std::string n = std::string( get_typename< C<dummy_arg,dummy_arg> >() ) + "<" + std::string(get_typename<TP>()) +  "," + 
                               std::string( get_typename<TP>() ) + ">"; 
        return n.c_str(); 
    } 
};


#define BOOST_REFLECT_TYPEINFO( NAME ) \
namespace boost { namespace reflect { \
template<> \
struct get_typeinfo<NAME> { \
    enum is_defined_enum{ is_defined = 1 }; \
    static const char* name() { return BOOST_PP_STRINGIZE(NAME); } \
}; } }


#define BOOST_REFLECT_TEMPLATE_TYPEINFO( NAME ) \
namespace boost { namespace reflect {\
template<>\
struct get_typeinfo<NAME<dummy_arg> > { \
    enum is_defined_enum{ is_defined = 1 }; \
    static const char* name() { return BOOST_PP_STRINGIZE(NAME); } \
}; } }

#define BOOST_REFLECT_TEMPLATE2_TYPEINFO( NAME ) \
namespace boost { namespace reflect {\
template<>\
struct get_typeinfo<NAME<dummy_arg,dummy_arg> > { \
    enum is_defined_enum{ is_defined = 1 }; \
    static const char* name() { return BOOST_PP_STRINGIZE(NAME); } \
}; } }


#define CUSTOM_MEMBER_CASES(r, data, i, elem)\
case BOOST_PP_TUPLE_ELEM( 3, 1, elem ):\
   v.accept_member( name, &data::BOOST_PP_TUPLE_ELEM(3, 0, elem), BOOST_PP_STRINGIZE( BOOST_PP_TUPLE_ELEM(3,0,elem) ), BOOST_PP_TUPLE_ELEM(3, 2,elem) );\
   break;

#define CUSTOM_MEMBER_ALL(r, data, i, elem)\
   v.accept_member( name, &data::BOOST_PP_TUPLE_ELEM(3, 0, elem), BOOST_PP_STRINGIZE( BOOST_PP_TUPLE_ELEM(3,0,elem) ), BOOST_PP_TUPLE_ELEM(3, 2,elem) );\

#define MEMBER_CASES(r, data, i, elem)\
case i:\
   v.accept_member( name, &data::elem, BOOST_PP_STRINGIZE( elem ), i ); \
   break;

#define MEMBER_ALL(r, data, i, elem)\
   v.accept_member( name, &data::elem, BOOST_PP_STRINGIZE( elem ), i );

#define INHERITS (baseA)(baseB)

#define ACCEPT_BASE(r, data, i, elem) \
    v.accept_base( *static_cast<elem*>(&name), BOOST_PP_STRINGIZE( elem ), field );


#define BOOST_REFLECT_IMPL( CONST,TYPE, INHERITS, MEMBERS ) \
template<typename Visitor>\
static inline void visit( CONST TYPE& name, Visitor& v, uint32_t field = -1 ) { \
    v.start(name, BOOST_PP_STRINGIZE(TYPE) );\
    BOOST_PP_SEQ_FOR_EACH_I( ACCEPT_BASE, TYPE, INHERITS ) \
    switch( field ) { \
        case -1: \
            BOOST_PP_SEQ_FOR_EACH_I( MEMBER_ALL, TYPE, MEMBERS ) \
            break; \
        BOOST_PP_SEQ_FOR_EACH_I( MEMBER_CASES, TYPE, MEMBERS ) \
        default: \
            v.not_found( name, field );\
    }\
    v.end(name, BOOST_PP_STRINGIZE(TYPE) );\
} \

#define BOOST_IDL_CUSTOM_REFLECT_IMPL( CONST,TYPE, INHERITS, MEMBERS ) \
template<typename Visitor>\
static inline void visit( CONST TYPE& name, Visitor& v, uint32_t field = -1 ) { \
    v.start(name, BOOST_PP_STRINGIZE(TYPE) );\
    BOOST_PP_SEQ_FOR_EACH_I( ACCEPT_BASE, TYPE, INHERITS ) \
    switch( field ) { \
        case -1: \
            BOOST_PP_SEQ_FOR_EACH_I( CUSTOM_MEMBER_ALL, TYPE, MEMBERS ) \
            break; \
        BOOST_PP_SEQ_FOR_EACH_I( CUSTOM_MEMBER_CASES, TYPE, MEMBERS ) \
        default: \
            v.not_found( name, field );\
    }\
    v.end(name, BOOST_PP_STRINGIZE(TYPE) );\
} \

#define BOOST_IDL_EMPTY

/**
 *  @param MEMBERS - a sequence of member names.  (field1)(field2)(field3)
 */
#define BOOST_REFLECT( TYPE, INHERITS, MEMBERS ) \
    BOOST_REFLECT_TYPEINFO(TYPE) \
namespace boost { namespace reflect { \
template<> struct reflector<TYPE> {\
    BOOST_REFLECT_IMPL( const, TYPE, INHERITS, MEMBERS ) \
    BOOST_REFLECT_IMPL( BOOST_IDL_EMPTY, TYPE, INHERITS, MEMBERS ) \
}; } }

/*
template<> struct reflector<TYPE> { \
} } }
*/


/**
 *   This macro is identical to BOOST_REFLECT, except that it gives you
 *   the ability to customize field numbers and flags.
 *
 *   @param MEMBERS  - a sequence of 3 param tuples.
 *                    ((field_name, NUMBER, FLAGS))((field_name1, NUMBER, FLAGS))
 *
 */
#define BOOST_IDL_CUSTOM_REFLECT( TYPE, INHERITS, MEMBERS ) \
    BOOST_REFLECT_TYPEINFO(TYPE) \
namespace boost { namespace reflect { \
template<> struct reflector<TYPE> { \
    BOOST_IDL_CUSTOM_REFLECT_IMPL( const, TYPE, INHERITS, MEMBERS ) \
    BOOST_IDL_CUSTOM_REFLECT_IMPL( BOOST_IDL_EMPTY, TYPE, INHERITS, MEMBERS ) \
} } }



/**
 *  Unless this is specialized, visit() will not be defined for T.
 *
 *  Boost.Fusion sequences have special handling because we can provide
 *  automatic "reflection" on fusion sequences.
 */
template<typename T>
struct reflector 
{
};

} } // namespace boost::reflect

// these macros specify namespace boost::reflect 
BOOST_REFLECT_TYPEINFO( void )
BOOST_REFLECT_TYPEINFO( bool )
BOOST_REFLECT_TYPEINFO( uint8_t )
BOOST_REFLECT_TYPEINFO( uint16_t )
BOOST_REFLECT_TYPEINFO( uint32_t )
BOOST_REFLECT_TYPEINFO( uint64_t )
BOOST_REFLECT_TYPEINFO( int8_t )
BOOST_REFLECT_TYPEINFO( int16_t )
BOOST_REFLECT_TYPEINFO( int32_t )
BOOST_REFLECT_TYPEINFO( int64_t )
BOOST_REFLECT_TYPEINFO( double )
BOOST_REFLECT_TYPEINFO( float )
BOOST_REFLECT_TYPEINFO( std::string )
BOOST_REFLECT_TEMPLATE_TYPEINFO( std::vector )
BOOST_REFLECT_TEMPLATE_TYPEINFO( std::set )
BOOST_REFLECT_TEMPLATE_TYPEINFO( std::list )
BOOST_REFLECT_TEMPLATE2_TYPEINFO( std::map )
BOOST_REFLECT_TEMPLATE2_TYPEINFO( std::pair )

#include <boost/reflect/reflect_function_signature.hpp>

#endif
