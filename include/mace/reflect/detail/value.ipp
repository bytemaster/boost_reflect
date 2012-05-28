#ifndef _MACE_REFLECT_DETAIL_VALUE_IPP_
#define _MACE_REFLECT_DETAIL_VALUE_IPP_
#include <mace/reflect/value.hpp>

namespace mace { namespace reflect {

    value::value( const value_cref& c) {
       *this =  c.get_holder()->copy();
    }
    value::value( const value_ref& c) {
       *this =  c.get_holder()->copy();
    }

    template<typename T>
    value::value( const T& v ) {
      BOOST_STATIC_ASSERT( sizeof(held) >= sizeof(typename detail::select_holder<T>::type) );
      new(held) typename detail::select_holder<T>::type(v);
    }

    template<typename T>
    value::value( typename boost::remove_reference<T>::type&& v ) {
      BOOST_STATIC_ASSERT( sizeof(held) >= sizeof(typename detail::select_holder<T>::type) );
      new(held) typename detail::select_holder<T>::type(std::forward<T>(v));
    }

    value::value( value&& v ) {
      memcpy( held, v.held, sizeof(held) );
      new(v.held) detail::place_holder();
    }

    template<typename T>
    value& value::operator=( const T& v ) {
       if( ptr<T>() ) { *ptr<T>() = v; return *this; }
       get_holder()->~place_holder();

       new(held) typename detail::select_holder<T>::type(v);
       return *this;
    }
    
    template<typename T>
    value& value::operator=( T&& v ) {
       if( ptr<T>() ) { *ptr<T>() = std::forward<T>(v); }
       else {
           new(held)typename detail::select_holder<T>::type(std::forward<T>(v));
       }
        return *this;
     }
     
     template<typename T>
     value& value::operator=( value&& v ) {
        memcpy(held,v.held,sizeof(held));
        new (v.held) detail::place_holder();
        return *this;
     }
     
     template<typename T>
     value& value::operator=( const value& v ) {
       if( this != &v ) {
          get_holder()->~place_holder();
          v.get_holder()->clone( held );
       }
       return *this;
     }

     template<typename T>
     value& value::operator=( const value_ref& v ){
        return *this = v.get_holder()->copy();
     }

     template<typename T>
     value& value::operator=( const value_cref& v ) {
        return *this = v.get_holder()->copy();
     }

     value_cref value::operator[]( const std::string& field )const { 
       return get_holder()->get_field(field); 
     }
     value_ref  value::operator[]( const std::string& field ) { 
       return get_holder()->get_field(field);            
     }

} } // mace::reflect

#endif 
