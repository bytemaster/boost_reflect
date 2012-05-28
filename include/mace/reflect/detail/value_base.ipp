#ifndef _MACE_REFLECT_DETAIL_VALUE_BASE_IPP_
#define _MACE_REFLECT_DETAIL_VALUE_BASE_IPP_
#include <mace/reflect/value_base.hpp>
#include <mace/reflect/detail/get_field.hpp>

namespace mace { namespace reflect {

    value_base::value_base()  { new (held) detail::place_holder(); }
    value_base::~value_base() { get_holder()->~place_holder(); }
    value_base::value_base(const value_base& copy) 
    { copy.get_holder()->clone( held ); }

    bool           value_base::is_array()const {
      return get_holder()->is_array();
    }
    size_t         value_base::size()const {
      return get_holder()->size();
    }

    /**
     *  If a struct, iterates over fields
     *  If a map, over keys
     *  if an array, over indexes
     */
    iterator       value_base::begin() {
      return get_holder()->begin();
    }
    const_iterator value_base::begin()const {
      return get_holder()->begin();
    }
    iterator       value_base::end() {
      return iterator();
    }
    const_iterator value_base::end()const  {
      return const_iterator();
    }

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


      template<typename P1>
      value value_base::operator()(P1 p1)const {
        std::vector<value> params(1); params[0] = p1;
        return get_holder()->call( params );
      }
      template<typename P1>
      value value_base::operator()(P1 p1){
        std::vector<value> params(1); params[0] = p1;
        return get_holder()->call( params );
      }

      template<typename P1, typename P2>
      value value_base::operator()(P1 p1,P2 p2)const{
        std::vector<value> params(2); 
        params[0] = p1;
        params[1] = p2;
        return get_holder()->call( params );
      }

      template<typename P1,typename P2>
      value value_base::operator()(P1 p1,P2 p2){
        std::vector<value> params(2); 
        params[0] = p1;
        params[1] = p2;
        return get_holder()->call( params );
      }



} } // mace::reflect
#endif // _BOOST_REFLECT_DETAIL_VALUE_BASE_IPP_
