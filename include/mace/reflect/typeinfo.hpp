#ifndef _MACE_REFLECT_TYPEINFO_HPP
#define _MACE_REFLECT_TYPEINFO_HPP
namespace mace { namespace reflect {

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
template<typename TP> 
struct get_typeinfo< std::vector<TP> > { 
  enum is_defined_enum{ is_defined = get_typeinfo<TP>::is_defined }; 
  static const char* name() 
  {  
    static std::string n = "std::vector<" + std::string(get_typename<TP>()) +  ">"; 
    return n.c_str(); 
  } 
}; 
template<typename TP> 
struct get_typeinfo< std::list<TP> > { 
  enum is_defined_enum{ is_defined = get_typeinfo<TP>::is_defined }; 
  static const char* name() 
  {  
    static std::string n = "std::list<" + std::string(get_typename<TP>()) +  ">"; 
    return n.c_str(); 
  } 
}; 

template<typename TP, typename TP2>
struct get_typeinfo< std::map<TP,TP2> > { 
  enum is_defined_enum{ is_defined = get_typeinfo<TP>::is_defined && get_typeinfo<TP2>::is_defined }; 
  static const char* name() 
  {  
    // TODO: make thread safe... 
    static std::string n = "std::map<" + std::string(get_typename<TP>()) +  "," + 
                                      std::string(get_typename<TP2>() ) + ">"; 
    return n.c_str(); 
  } 
};
#endif

/**
 *  @brief Allows the typename to be accessed via get_typename<T>() in a 
 *  cross-platform, demangled way that std::typeinfo cannot do.
 */
#define MACE_REFLECT_TYPEINFO( NAME ) \
namespace mace { namespace reflect { \
template<> struct get_typeinfo<NAME> { \
    enum is_defined_enum{ is_defined = 1 }; \
    static const char* name() { return BOOST_PP_STRINGIZE(NAME); } \
}; } }

#define MACE_REFLECT_TYPEINFO_ALIAS( NAME, STR_NAME ) \
namespace mace { namespace reflect { \
template<> struct get_typeinfo<NAME> { \
    enum is_defined_enum{ is_defined = 1 }; \
    static const char* name() { return STR_NAME; } \
}; } }


/**
 *  @brief Same as MACE_REFLECT_TYPEINFO except works for templates with 1 paramater.
 */
#define MACE_REFLECT_TEMPLATE_TYPEINFO( NAME ) \
namespace mace { namespace reflect {\
template<> struct get_typeinfo<NAME<void_t> > { \
    enum is_defined_enum{ is_defined = 1 }; \
    static const char* name() { return BOOST_PP_STRINGIZE(NAME); } \
}; } }

/**
 *  @brief Same as MACE_REFLECT_TYPEINFO except works for templates with 2 paramaters.
 */
#define MACE_REFLECT_TEMPLATE2_TYPEINFO( NAME ) \
namespace mace { namespace reflect {\
template<> struct get_typeinfo<NAME<void_t,void_t> > { \
    enum is_defined_enum{ is_defined = 1 }; \
    static const char* name() { return BOOST_PP_STRINGIZE(NAME); } \
}; } }

} } // mace::reflect


// these macros specify namespace mace::reflect 
MACE_REFLECT_TYPEINFO( void )
MACE_REFLECT_TYPEINFO( bool )
MACE_REFLECT_TYPEINFO( uint8_t )
MACE_REFLECT_TYPEINFO( uint16_t )
MACE_REFLECT_TYPEINFO( uint32_t )
MACE_REFLECT_TYPEINFO( uint64_t )
MACE_REFLECT_TYPEINFO( int8_t )
MACE_REFLECT_TYPEINFO( int16_t )
MACE_REFLECT_TYPEINFO( int32_t )
MACE_REFLECT_TYPEINFO( int64_t )
MACE_REFLECT_TYPEINFO( double )
MACE_REFLECT_TYPEINFO( float )
MACE_REFLECT_TYPEINFO( void_t )
MACE_REFLECT_TYPEINFO( std::string )
MACE_REFLECT_TEMPLATE_TYPEINFO( std::vector )
MACE_REFLECT_TEMPLATE_TYPEINFO( std::set )
MACE_REFLECT_TEMPLATE_TYPEINFO( std::list )
MACE_REFLECT_TEMPLATE2_TYPEINFO( std::map )
MACE_REFLECT_TEMPLATE2_TYPEINFO( std::pair )

#include <mace/reflect/reflect_function_signature.hpp>

#endif
