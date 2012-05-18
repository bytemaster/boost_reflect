#ifndef _BOOST_REFLECT_DETAIL_VALUE_CREF_IPP_
#define _BOOST_REFLECT_DETAIL_VALUE_CREF_IPP_
#include <boost/reflect/value_cref.hpp>
#include <boost/reflect/detail/place_holder.hpp>
#include <boost/reflect/detail/get_field.hpp>

namespace boost { namespace reflect {

    template<typename T>
    value_cref::value_cref( const T&& t ) {
      BOOST_STATIC_ASSERT( sizeof(held) >= sizeof( detail::place_holder_const_reference<T> ) );
      new (held) detail::place_holder_const_reference<T>( std::forward<const T>(t) );
    }

    template<typename T>
    value_cref::value_cref( const T& v ) { 
      BOOST_STATIC_ASSERT( sizeof(held) >= sizeof( detail::place_holder_const_reference<T> ) );
      new (held) detail::place_holder_const_reference<T>(v);
    }

    value_cref::value_cref( const value& v ) {
        v.get_holder()->cref().get_holder()->clone(held);
    }


    value_cref value_cref::operator[]( const std::string& field )const {
      return get_holder()->get_field( field );
    }

    value_cref::value_cref( const value_cref& c ):value_base(c){}

    value_cref::value_cref( value_cref&& t ) {
      memcpy( held, t.held, sizeof(held) );
      new (t.held) detail::place_holder(); 
    }

    template<typename T>
    inline const T& value_cref::get()const {
      const T* v = const_ptr<T>();
      if( v ) return *v;
      BOOST_THROW_EXCEPTION( bad_value_cast() );
    }

    template<typename T>
    inline const T* value_cref::const_ptr()const {
       if( get_holder()->type() == get_typename<T>() )
         return static_cast<const T*>( get_holder()->ptr() );
       return 0;
    }


} } // namespace boost::reflect

#endif // _BOOST_REFLECT_DETAIL_VALUE_CREF_IPP_
