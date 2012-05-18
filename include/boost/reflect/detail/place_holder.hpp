#ifndef _BOOST_REFLECT_DETAIL_PLACE_HOLDER_HPP_
#define _BOOST_REFLECT_DETAIL_PLACE_HOLDER_HPP_
#include <boost/reflect/detail/get_field.hpp>
#include <boost/reflect/value_visitor.hpp>

namespace boost { namespace reflect { namespace detail {

  struct place_holder {
      virtual ~place_holder(){}
      virtual const char*   type()const { return 0; }
      virtual void*         ptr()       { return 0; }
      virtual const void*   ptr()const  { return 0; }
      virtual place_holder* clone( char* place = 0 )const {
        if( place ) return new(place)place_holder();
        return new place_holder();
      }

      virtual void visit( read_value_visitor&& v )const { v(); }
      virtual void visit( write_value_visitor&& v )     { v(); }

      virtual value      copy()const { BOOST_THROW_EXCEPTION(bad_value_cast()); }
      virtual value_cref cref()const { BOOST_THROW_EXCEPTION(bad_value_cast()); }
      virtual bool       has_field( const std::string& f )const { return false; }
      virtual value_ref  get_field( const std::string& f )      { BOOST_THROW_EXCEPTION(bad_value_cast()); }
      virtual value_cref get_field( const std::string& f )const { BOOST_THROW_EXCEPTION(bad_value_cast()); }
  };

  template<typename T>
  struct place_holder_reference : place_holder {
      place_holder_reference( T& v ):val(v){}

      virtual const char*   type()const { return get_typename<T>(); }
      virtual void*         ptr()       { return &val;              }
      virtual const void*   ptr()const  { return &val;              }
      virtual place_holder* clone( char* place = 0 )const {
        if( place ) return new (place) place_holder_reference(val);
        return new place_holder_reference(val);
      }

      virtual void visit( read_value_visitor&& v )const { v(val); }
      virtual void visit( write_value_visitor&& v )     { v(val); }

      virtual value_cref cref()const { return val;  }
      virtual value      copy()const { return val; }
      virtual bool       has_field( const std::string& f )const { return reflect::has_field<T>(f); }
      virtual value_ref  get_field( const std::string& f ) {
         return reflect::get_field(f,val); 
      }
      virtual value_cref get_field( const std::string& f )const{
         return reflect::get_field(f,val); 
      }
      T& val;
  };

  template<typename T>
  struct place_holder_const_reference : place_holder {
      place_holder_const_reference( const T& v ):val(v){}

      virtual const char*   type()const { return get_typename<T>(); }
      virtual void*         ptr()       { return 0;                 }
      virtual const void*   ptr()const  { return &val;              }
      virtual place_holder* clone( char* place = 0 )const {
        if( place ) return new (place) place_holder_const_reference(val);
        return new place_holder_const_reference(val);
      }

      virtual void visit( read_value_visitor&& v )const { v(val); }
      virtual void visit( write_value_visitor&& v )     { v();    }

      virtual value_cref cref()const { return val; }
      virtual value      copy()const { return val; }
      virtual bool       has_field( const std::string& f )const { return reflect::has_field<T>(f); }
      virtual value_ref  get_field( const std::string& f ) {
         BOOST_THROW_EXCEPTION(bad_value_cast());
      }
      virtual value_cref get_field( const std::string& f )const  {
         return reflect::get_field(f,val); 
      }

      const T& val;
  };

  template<typename T,bool S = sizeof(T) <= sizeof(T*) >
  struct place_holder_value : place_holder {
      place_holder_value( const T& v ):val(new T(v)){}
      place_holder_value( T&& v ):val(new T(v)){}
      ~place_holder_value() { delete val; }

      virtual const char*   type()const { return get_typename<T>(); }
      virtual void*         ptr()       { return val;               }
      virtual const void*   ptr()const  { return val;               }
      virtual place_holder* clone( char* place = 0 )const {
        if( place ) return new (place) place_holder_value(*val);
        return new place_holder_value(*val);
      }

      virtual void visit( read_value_visitor&& v )const { v(*val); }
      virtual void visit( write_value_visitor&& v )     { v(*val); }

      virtual value_cref cref()const { return *val;  }
      virtual value      copy()const { return *val; }
      virtual bool       has_field( const std::string& f )const { return reflect::has_field<T>(f); }
      virtual value_ref  get_field( const std::string& f ){
         return reflect::get_field(f,*val); 
      }
      virtual value_cref get_field( const std::string& f )const {
         return reflect::get_field(f,*val); 
      }
      T* val;
  };

  template<typename T>
  struct place_holder_value<T,true> : place_holder {
    BOOST_STATIC_ASSERT( sizeof(T) <= sizeof(T*) );
      place_holder_value( T&& v ):val(std::forward<T>(v)){}
      place_holder_value( const T& v ):val(v){}

      virtual const char*   type()const { return get_typename<T>(); }
      virtual void*         ptr()       { return &val;              }
      virtual const void*   ptr()const  { return &val;              }
      virtual place_holder* clone( char* place = 0 )const {
        if( place ) return new (place) place_holder_value(val);
        return new place_holder_value(val);
      }

      virtual void visit( read_value_visitor&& v )const { v(val); }
      virtual void visit( write_value_visitor&& v )     { v(val); }

      virtual value      copy()const { return val; }
      virtual bool       has_field( const std::string& f )const { return reflect::has_field<T>(f); }
      virtual value_ref  get_field( const std::string& f ){
         return reflect::get_field(f,val); 
      }
      virtual value_cref get_field( const std::string& f )const {
         return reflect::get_field(f,val); 
      }

      T val;
  };

} } } // boost::reflect::detail

#endif
