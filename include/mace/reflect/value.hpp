#ifndef _MACE_REFLECT_VALUE_HPP_
#define _MACE_REFLECT_VALUE_HPP_
#include <mace/reflect/value_ref.hpp>

namespace mace { namespace reflect {

/**
 *  A value may hold any type and provides polymorphic
 *  access to its members by name.  In general a value
 *  may hold a struct, array, map, number, string, null,
 *  bool, or function.  
 *
 *  @code
 *    struct test {
 *      int num;
 *      std::string str;
 *      int print( std::string& );
 *    };
 *    value v(test());
 *    v["num"].as<int>();
 *    v["num"].as<std::string>();
 *    v["str"].as<std::string>();
 *    v["str"].as<int>();
 *    v["print"]( "hello world" );
 *
 *  @endcode
 *
 *  Given a value you can iterate over its members and 
 *  perform actions.
 *  
*/
class value : public value_base {
  public:
    value(){}

    template<typename T>
    value( const T& v );

    value( value&& v );

    template<typename T>
    value( typename boost::remove_reference<T>::type&& v );

    value( const value_cref& );
    value( const value_ref& );

    template<typename T>
    value& operator=( const T& v );
    
    template<typename T>
    value& operator=( T&& v );
    
    template<typename T>
    value& operator=( value&& v );
    
    template<typename T>
    value& operator=( const value& v );

    template<typename T>
    value& operator=( const value_ref& v );

    template<typename T>
    value& operator=( const value_cref& v );

    value_cref operator[]( const std::string& field )const;
    value_ref  operator[]( const std::string& field );
    value_cref operator[]( uint64_t idx )const;
    value_ref  operator[]( uint64_t idx );
};


} } // namespace mace::reflect

#include <mace/reflect/detail/value_cref.ipp>
#include <mace/reflect/detail/value_ref.ipp>
#include <mace/reflect/detail/value.ipp>
#include <mace/reflect/detail/value_base.ipp>
#include <mace/reflect/detail/iterator.ipp>

#endif
