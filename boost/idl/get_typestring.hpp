/**
 *  @file get_typestring.hpp
 *
 *  This file provides a means to get a const char* name of
 *  a class without using RTTI.  The name is then cross-platform.
 *
 */

#ifndef _BOOST_IDL_GET_TYPESTRING_HPP_
#define _BOOST_IDL_GET_TYPESTRING_HPP_
#include <boost/static_assert.hpp>
#include <string>
#include <typeinfo>
#include <map>
#include <stdint.h>

namespace boost { namespace idl {

/**
 *  Inheriting from self is desired to cause error if this 
 *  template is instantiated without one of the specializations.
 */
template< typename T>
struct get_typestring 
{
    static const char* str() { return typeid(T).name(); }
    enum is_defined_enum{ is_defined = 0 };
};


template<typename K, typename V>
struct get_typestring< std::map<K,V> >
{
    enum is_defined_enum{ is_defined = get_typestring<K>::is_defined && get_typestring<V>::is_defined  };
    static const char* str()
    {
        static std::string ts( "std::map<" + std::string(get_typestring<K>::str()) +","+ std::string(get_typestring<V>::str())+">" );
        return ts.c_str();
    }
};


} } // namespace boost::idl

#include <boost/idl/get_function_signature.hpp>

#define DEFINE_GETTYPESTRING(...) \
    namespace boost { namespace idl { template<> struct get_typestring<__VA_ARGS__> { enum is_defined_enum{ is_defined = 1 }; static const char* str() { return #__VA_ARGS__; } }; } }

#define DEFINE_GETTYPESTRING_ALIAS(NAME,...) \
    namespace boost { namespace idl { template<> struct get_typestring<__VA_ARGS__> { enum is_defined_enum{ is_defined = 1 }; static const char* str() { return NAME; } }; } }

#define BOOST_IDL_TYPESTRING_ALIAS( NAME, ... ) \
    DEFINE_GETTYPESTRING_ALIAS( NAME, __VA_ARGS__ ) \
    DEFINE_GETTYPESTRING_ALIAS( "const "NAME, const __VA_ARGS__ ) \
    DEFINE_GETTYPESTRING_ALIAS( "const "NAME"&", const __VA_ARGS__& ) \
    DEFINE_GETTYPESTRING_ALIAS( "const "NAME"*", const __VA_ARGS__* ) \
    DEFINE_GETTYPESTRING_ALIAS( NAME"&", __VA_ARGS__& ) \
    DEFINE_GETTYPESTRING_ALIAS( NAME"*", __VA_ARGS__* ) \
    DEFINE_GETTYPESTRING_ALIAS( NAME"**", __VA_ARGS__** ) 
  
/**
 *  @def BOOST_IDL_TYPESTRING(...)
 *
 *  Defines the get_typestring<> template for the given type.
 */
#define BOOST_IDL_TYPESTRING(...) \
    BOOST_IDL_TYPESTRING_ALIAS( #__VA_ARGS__, __VA_ARGS__ )

DEFINE_GETTYPESTRING(void)
DEFINE_GETTYPESTRING(void*)
DEFINE_GETTYPESTRING(void**)

BOOST_IDL_TYPESTRING(float)
BOOST_IDL_TYPESTRING(double)
BOOST_IDL_TYPESTRING(bool)
BOOST_IDL_TYPESTRING(int64_t)
BOOST_IDL_TYPESTRING(uint64_t)
BOOST_IDL_TYPESTRING(int32_t)
BOOST_IDL_TYPESTRING(uint32_t)
BOOST_IDL_TYPESTRING(int16_t)
BOOST_IDL_TYPESTRING(uint16_t)
BOOST_IDL_TYPESTRING(uint8_t)
BOOST_IDL_TYPESTRING(int8_t)
BOOST_IDL_TYPESTRING(std::string)

#ifndef _MSC_VER
BOOST_IDL_TYPESTRING(char)
#endif // _MSC_VER

#endif
