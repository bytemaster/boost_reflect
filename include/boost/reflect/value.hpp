#ifndef _BOOST_REFLECT_VALUE_HPP_
#define _BOOST_REFLECT_VALUE_HPP_
#include <boost/reflect/value_ref.hpp>

namespace boost { namespace reflect {

/**
 *  value ref has reference semantics, therefore it may only be
 *  assigned to reference types at construction
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
};


} } // namespace boost

#include <boost/reflect/detail/value_cref.ipp>
#include <boost/reflect/detail/value_ref.ipp>
#include <boost/reflect/detail/value.ipp>
#include <boost/reflect/detail/value_base.ipp>

#endif
