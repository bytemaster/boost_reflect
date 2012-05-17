#ifndef _BOOST_REFLECT_DETAIL_VALUE_CREF_IPP_
#define _BOOST_REFLECT_DETAIL_VALUE_CREF_IPP_
#include <boost/reflect/value_cref.hpp>
#include <boost/reflect/detail/get_field.hpp>

namespace boost { namespace reflect {

    template<typename T>
    value_cref::value_cref( const T&& t ) {
      BOOST_STATIC_ASSERT( sizeof(held) >= sizeof( detail::place_holder_const_rvalue<T> ) );
      new (held) detail::place_holder_const_rvalue<T>( std::forward<const T>(t) );
    }

    template<typename T>
    value_cref::value_cref( const T& v ) { 
      BOOST_STATIC_ASSERT( sizeof(held) >= sizeof( detail::place_holder_const_lvalue<T> ) );
      new (held) detail::place_holder_const_lvalue<T>(v);
    }

    value_cref::value_cref() { new (held) detail::place_holder(); }
    value_cref::~value_cref(){ ((detail::place_holder*)held)->~place_holder(); } 

    value_cref value_cref::operator[]( const std::string& field )const {
      return ((detail::place_holder*)held)->get_const_field( field );
    }

    value_cref::value_cref( const value_cref& c ){
      ((detail::place_holder*)c.held)->clone( held );
    } 

    value_cref::value_cref( value_cref&& t ) {
      memcpy( held, t.held, sizeof(held) );
      new (t.held) detail::place_holder(); 
    }
    bool value_cref::operator!()const {
      return ((detail::place_holder*)held)->type() == typeid(detail::place_holder);
    }

    const std::type_info& value_cref::type()const { 
      return ((detail::place_holder*)held)->type(); 
    }

    template<typename T>
    inline const T& value_cref::get()const {
        detail::place_holder_const<T>* p = 
            dynamic_cast<detail::place_holder_const<T>*>((detail::place_holder*)held);
        if( p ) return p->cref();
        boost::throw_exception(bad_value_cast());
    }


    template<typename T>
    inline const T* value_cref::ptr()const {
        detail::place_holder_const<T>* p = 
            dynamic_cast<detail::place_holder_const<T>*>((detail::place_holder*)held);
        if( p ) 
            return &p->cref();
        return 0;
    }

    template<typename T>
    bool value_cref::operator == ( const T& t )const {
       const T* v = ptr<T>();
       return v && *v == t;
    }

} } // namespace boost::reflect

#endif // _BOOST_REFLECT_DETAIL_VALUE_CREF_IPP_
