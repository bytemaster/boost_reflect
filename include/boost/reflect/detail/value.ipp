#ifndef _BOOST_REFLECT_DETAIL_VALUE_IPP_
#define _BOOST_REFLECT_DETAIL_VALUE_IPP_
#include <boost/reflect/value.hpp>

namespace boost { namespace reflect {

    value::value() { new (held) detail::place_holder(); }

    value::~value() { ((detail::place_holder*)held)->~place_holder(); }
    value::value( const value_cref& c) {
        ((detail::place_holder*)c.held)->get_cref();
    }
    value::value( const value_ref& c) {
        ((detail::place_holder*)c.held)->get_cref();
    }

    template<typename T>
    value::value( const T& v ) {
      new(held) detail::place_holder_value<T>(v);
    }

    template<typename T>
    value::value( T&& v ) {
      BOOST_STATIC_ASSERT( sizeof(held) >= sizeof(detail::place_holder_value<T>) );
      new(held) detail::place_holder_value<T>(std::forward<T>(v));
    }

    value::value( const value& v ) {
      ((detail::place_holder*)v.held)->clone( held );
    }

    value::value( value&& v ) {
      memcpy( held, v.held, sizeof(held) );
      new(v.held) detail::place_holder();
    }

    value_cref value::operator[]( const std::string& field )const {
      return ((detail::place_holder*)held)->get_const_field( field );
    }
    value_ref value::operator[]( const std::string& field ){
      return ((detail::place_holder*)held)->get_field( field );
    }

     const std::type_info& value::type()const {
        ((detail::place_holder*)held)->type();
     }

     template<typename T>
     inline const T* value::const_ptr()const {
        detail::place_holder_value<T>* p = 
            dynamic_cast<detail::place_holder_value<T>*>((detail::place_holder*)held);
        if( p ) 
            return &p->cref();
        return 0;
     }
     template<typename T>
     inline T* value::ptr(){
        detail::place_holder_value<T>* p = 
            dynamic_cast<detail::place_holder_value<T>*>((detail::place_holder*)held);
        if( p ) 
            return &p->ref();
        return 0;
     }

     template<typename T>
     value& value::operator=( const T& v ) {
        if( ptr<T>() ) { *ptr<T>() = v; return *this; }

        ((detail::place_holder*)held)->~place_holder();
        new(held) detail::place_holder_value<T>(v);
        return *this;
     }
     
     template<typename T>
     value& value::operator=( T&& v ) {
        if( ptr<T>() ) { *ptr<T>() = std::forward<T>(v); }
        else {
          ((detail::place_holder*)held)->~place_holder();
          new(held) detail::place_holder_value<T>(std::forward<T>(v) );
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
          ((detail::place_holder*)held)->~place_holder();
          ((detail::place_holder*)v.held)->clone( held );
       }
       return *this;
     }

     template<typename T>
     value& value::operator=( const value_ref& v ){
        ((detail::place_holder*)held)->~place_holder();
        ((detail::place_holder*)v.held)->get_cref();
        return *this;
     }

     template<typename T>
     value& value::operator=( const value_cref& v ) {
        ((detail::place_holder*)held)->~place_holder();
        ((detail::place_holder*)v.held)->get_cref();
        return *this;
     }


} } // boost::reflect

#endif 
