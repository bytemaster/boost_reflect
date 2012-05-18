#ifndef _BOOST_REFLECT_DETAIL_VALUE_BASE_IPP_
#define _BOOST_REFLECT_DETAIL_VALUE_BASE_IPP_
#include <boost/reflect/value_base.hpp>
#include <boost/reflect/detail/get_field.hpp>
#include <boost/cmt/log/log.hpp>
namespace boost { namespace reflect {

    value_base::value_base()  { new (held) detail::place_holder(); }
    value_base::~value_base() { get_holder()->~place_holder(); }
    value_base::value_base(const value_base& copy) 
    { copy.get_holder()->clone( held ); }

    inline const detail::place_holder* value_base::get_holder()const {
      return reinterpret_cast<detail::place_holder*>(const_cast<char*>(held));
    }
    inline detail::place_holder* value_base::get_holder(){
      return reinterpret_cast<detail::place_holder*>(held);
    }

    const char* value_base::type()const { return get_holder()->type(); }
  
    void value_base::visit( read_value_visitor&& v )const {
      get_holder()->visit( std::forward<read_value_visitor>(v) );
    }
    void value_base::visit( write_value_visitor&& v ) {
      get_holder()->visit( std::forward<write_value_visitor>(v) );
    }

    template<typename T>
    T value_base::as()const {
      T val;
      visit( get_visitor<T>(val) );
      return val;
    }

    template<typename T>
    void value_base::set_as( const T&& v ) {
      visit( set_visitor<T>( std::forward<const T>(v) ) );
    }

    bool value_base::operator!()const {
      return get_holder()->type() == 0;
    }

    template<typename T>
    inline const T* value_base::ptr()const {
       if( get_holder()->type() == get_typename<T>() )
         return static_cast<const T*>( get_holder()->ptr() );
       return 0;
    }
    template<typename T>
    inline T* value_base::ptr(){
       if( get_holder()->type() == get_typename<T>() )
         return static_cast<T*>( get_holder()->ptr() );
       return 0;
    }


    bool value_base::has_field( const std::string& field )const{
      return get_holder()->has_field(field); 
    }

    template<typename T>
    inline const T& value_base::get()const {
      const T* v = ptr<T>();
      if( v ) return *v;
      BOOST_THROW_EXCEPTION( bad_value_cast() );
    }

    template<typename T>
    inline T& value_base::get() {
      T* v = ptr<T>();
      if( v ) return *v;
      BOOST_THROW_EXCEPTION( bad_value_cast() );
    }


} } // boost::reflect
#endif // _BOOST_REFLECT_DETAIL_VALUE_BASE_IPP_
