#ifndef _BOOST_REFLECT_TYPEINFO_HPP
#define _BOOST_REFLECT_TYPEINFO_HPP
namespace boost { namespace reflect {

/**
 *  @brief provides compile time access to name of a type.
 */
template<typename T>
struct get_typeinfo { 
  enum is_defined_enum{ is_defined = 0 };
  static const char* name() { return typeid(T).name(); }
};

/**
 *  @brief Removes all decorations and returns the typename.
 */
template<typename T>
inline const char* get_typename()  {
  return get_typeinfo<typename boost::remove_const<
                      typename boost::remove_reference<
                      typename boost::remove_pointer<T>::type>::type>::type>::name();
}
#ifndef DOXYGEN
template<typename TP, template<typename> class C> 
struct get_typeinfo< C<TP> > { 
  // used to specify template get_typeinfo

  enum is_defined_enum{ is_defined = get_typeinfo<TP>::is_defined }; 
  static const char* name() 
  {  
    static std::string n = std::string( get_typename<C<void_t> >() ) + 
                               "<" + std::string(get_typename<TP>()) +  ">"; 
    return n.c_str(); 
  } 
}; 

template<typename TP, typename TP2, template<typename, typename> class C> 
struct get_typeinfo< C<TP,TP2> > { 
  enum is_defined_enum{ is_defined = get_typeinfo<TP>::is_defined && get_typeinfo<TP2>::is_defined }; 
  static const char* name() 
  {  
    static std::string n = std::string( get_typename< C<void_t,void_t> >() ) + 
                                "<" + std::string(get_typename<TP>()) +  "," + 
                                      std::string( get_typename<TP>() ) + ">"; 
    return n.c_str(); 
  } 
};
#endif

/**
 *  @brief Allows the typename to be accessed via get_typename<T>() in a 
 *  cross-platform, demangled way that std::typeinfo cannot do.
 */
#define BOOST_REFLECT_TYPEINFO( NAME ) \
namespace boost { namespace reflect { \
template<> \
struct get_typeinfo<NAME> { \
    enum is_defined_enum{ is_defined = 1 }; \
    static const char* name() { return BOOST_PP_STRINGIZE(NAME); } \
}; } }


/**
 *  @brief Same as BOOST_REFLECT_TYPEINFO except works for templates with 1 paramater.
 */
#define BOOST_REFLECT_TEMPLATE_TYPEINFO( NAME ) \
namespace boost { namespace reflect {\
template<>\
struct get_typeinfo<NAME<void_t> > { \
    enum is_defined_enum{ is_defined = 1 }; \
    static const char* name() { return BOOST_PP_STRINGIZE(NAME); } \
}; } }

/**
 *  @brief Same as BOOST_REFLECT_TYPEINFO except works for templates with 2 paramaters.
 */
#define BOOST_REFLECT_TEMPLATE2_TYPEINFO( NAME ) \
namespace boost { namespace reflect {\
template<>\
struct get_typeinfo<NAME<void_t,void_t> > { \
    enum is_defined_enum{ is_defined = 1 }; \
    static const char* name() { return BOOST_PP_STRINGIZE(NAME); } \
}; } }

} } // boost::reflect


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
BOOST_REFLECT_TYPEINFO( void_t )
BOOST_REFLECT_TYPEINFO( std::string )
BOOST_REFLECT_TEMPLATE_TYPEINFO( std::vector )
BOOST_REFLECT_TEMPLATE_TYPEINFO( std::set )
BOOST_REFLECT_TEMPLATE_TYPEINFO( std::list )
BOOST_REFLECT_TEMPLATE2_TYPEINFO( std::map )
BOOST_REFLECT_TEMPLATE2_TYPEINFO( std::pair )

#include <boost/reflect/reflect_function_signature.hpp>

#endif
