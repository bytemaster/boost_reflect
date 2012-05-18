#ifndef _BOOST_REFLECT_DETAIL_VALUE_REF_IPP_
#define _BOOST_REFLECT_DETAIL_VALUE_REF_IPP_
#include <boost/reflect/value_ref.hpp>
#include <boost/reflect/detail/get_field.hpp>

namespace boost { namespace reflect {
     value_ref::value_ref( const value_ref& r ):value_base(r){}

     value_ref::value_ref( value_ref&& v ) {
        memcpy( held, v.held, sizeof(held) ); 
        new (v.held) detail::place_holder();
     }

     value_ref::operator value_cref()const {
        return get_holder()->cref();
     }

     template<typename T>
     value_ref::value_ref( T& v ) { 
      BOOST_STATIC_ASSERT( sizeof(detail::place_holder_reference<T>(v)) <= sizeof(held) );
      new (held) detail::place_holder_reference<T>(v); 
     }

     template<typename T>
     value_ref& value_ref::operator=( const T& r ) {
         get<T>() = r;
         return *this;
     }

     value_cref value_ref::operator[]( const std::string& field )const { 
       return get_holder()->get_field(field); 
     }
     value_ref  value_ref::operator[]( const std::string& field ) { 
       return get_holder()->get_field(field);            
     }


} } // namespace boost::reflect

#endif // _BOOST_REFLECT_DETAIL_VALUE_REF_IPP_
