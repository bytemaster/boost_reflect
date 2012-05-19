#ifndef _BOOST_REFLECT_DETAIL_PLACE_HOLDER_HPP_
#define _BOOST_REFLECT_DETAIL_PLACE_HOLDER_HPP_
#include <boost/reflect/detail/get_field.hpp>
#include <boost/reflect/value_visitor.hpp>
#include <boost/reflect/detail/iterator_impl.hpp>
#include <map>
#include <boost/fusion/include/is_sequence.hpp>

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

      virtual iterator_impl_base*       begin()      { return 0; }
      virtual const_iterator_impl_base* begin()const { return 0; }
      virtual size_t                    size()const         { return 0; }
      virtual bool                      is_array()const     { return 0; }

      virtual value      copy()const { BOOST_THROW_EXCEPTION(bad_value_cast()); }
      virtual value_cref cref()const { BOOST_THROW_EXCEPTION(bad_value_cast()); }
      virtual bool       has_field( const std::string& f )const { return false; }
      virtual value_ref  get_field( const std::string& f )      { BOOST_THROW_EXCEPTION(bad_value_cast()); }
      virtual value_cref get_field( const std::string& f )const { BOOST_THROW_EXCEPTION(bad_value_cast()); }
      virtual value_ref  get_field( uint64_t idx )      { BOOST_THROW_EXCEPTION(bad_value_cast()); }
      virtual value_cref get_field( uint64_t idx )const { BOOST_THROW_EXCEPTION(bad_value_cast()); }
  };


  template<typename T>
  struct place_holder_reference : place_holder {
      place_holder_reference( T& v ):val(v){}

      virtual size_t        size()const { return get_field_map<T>().size(); }
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

      virtual iterator_impl_base*       begin()      { return new iterator_impl<T>(val, get_field_map<T>().begin()); }
      virtual const_iterator_impl_base* begin()const { return new const_iterator_impl<T>(val, get_field_map<T>().begin()); }

      const T& get_val()const { return val; }
      T&       get_val()      { return val; }
      T& val;
  };

  template<typename T, class Base >
  struct place_holder_array: public Base {
      place_holder_array(){}
      place_holder_array( T& v ):Base(v){}
      place_holder_array( const T& v ):Base(v){}
      place_holder_array( T&& v ):Base(std::forward<T>(v)){}
      virtual bool   is_array()const { return true;       }
      virtual size_t size()const     { return this->get_val().size(); }
      virtual bool   has_field( const std::string& f )const { 
        try {
            uint64_t idx = boost::lexical_cast<uint64_t>(f);
            return idx < size();
        } catch ( ... ) { return false; } // bad lexical cast
      }
      virtual value_ref  get_field( const std::string& f ) {
            uint64_t idx = boost::lexical_cast<uint64_t>(f);
            return this->get_val()[idx];
      }
      virtual value_cref get_field( const std::string& f )const{
            uint64_t idx = boost::lexical_cast<uint64_t>(f);
            return this->get_val()[idx];
      }
      virtual value_ref  get_field( uint64_t idx )      { return this->get_val()[idx]; }
      virtual value_cref get_field( uint64_t idx )const { return this->get_val()[idx]; }
      virtual iterator_impl_base*       begin()      { return new vector_iterator_impl<typename T::value_type>( this->get_val(), this->get_val().begin() ); }
      virtual const_iterator_impl_base* begin()const { return new const_vector_iterator_impl<typename T::value_type>( this->get_val(), this->get_val().begin() ); }
  };


  template<typename T, class Base >
  struct place_holder_const_array: public Base {
      place_holder_const_array( const T& v ):Base(v){}
      virtual bool   is_array()const { return true;       }
      virtual size_t size()const     { return this->get_val().size(); }
      virtual bool   has_field( const std::string& f )const { 
        try {
            uint64_t idx = boost::lexical_cast<uint64_t>(f);
            return idx < size();
        } catch ( ... ) { return false; } // bad lexical cast
      }
      virtual value_cref get_field( const std::string& f )const{
            uint64_t idx = boost::lexical_cast<uint64_t>(f);
            return this->get_val()[idx];
      }
      virtual value_cref get_field( uint64_t idx )const { return this->get_val()[idx]; }

      virtual iterator_impl_base*       begin()      { return 0; }
      virtual const_iterator_impl_base* begin()const { return new const_vector_iterator_impl<typename T::value_type>( this->get_val(), this->get_val().begin() ); }
  };



  template<typename K, typename V, class Base>
  struct place_holder_map: public Base {
      place_holder_map( std::map<K,V>& v ):Base(v){}
      place_holder_map( const std::map<K,V>& v ):Base(v){}
      place_holder_map( std::map<K,V>&& v ):Base(std::forward<std::map<K,V> >(v) ){}
      virtual bool   is_array()const { return false;       }
      virtual size_t size()const     { return this->get_val().size(); }
      virtual bool   has_field( const std::string& f )const { 
        try {
            return this->get_val().find(boost::lexical_cast<K>(f)) != this->get_val().end();
        } catch ( ... ) { return false; }
      }
      virtual value_ref  get_field( const std::string& f ) {
         return this->get_val()[boost::lexical_cast<K>(f)];
      }
      virtual value_cref get_field( const std::string& f )const{
         typename std::map<K,V>::const_iterator itr = this->get_val().find(boost::lexical_cast<K>(f));
         if( itr != this->get_val().end() )
           return itr->second;
         BOOST_THROW_EXCEPTION( bad_value_cast() );
      }
      virtual iterator_impl_base*       begin()      { return new map_iterator_impl<K,V>( this->get_val(), this->get_val().begin() ); }
      virtual const_iterator_impl_base* begin()const { return new const_map_iterator_impl<K,V>( this->get_val(), this->get_val().begin() ); }
  };
  template<typename K, typename V, class Base>
  struct place_holder_const_map: public Base {
      place_holder_const_map( const std::map<K,V>& v ):Base(v){}
      virtual bool   is_array()const { return false;       }
      virtual size_t size()const     { return this->get_val().size(); }
      virtual bool   has_field( const std::string& f )const { 
        try {
            return this->get_val().find(boost::lexical_cast<K>(f)) != this->get_val().end();
        } catch ( ... ) { return false; }
      }
      virtual value_ref  get_field( const std::string& f ) {
         BOOST_THROW_EXCEPTION( bad_value_cast() );
      }
      virtual value_cref get_field( const std::string& f )const{
         typename std::map<K,V>::const_iterator itr = this->get_val().find(boost::lexical_cast<K>(f));
         if( itr != this->get_val().end() )
           return itr->second;
         BOOST_THROW_EXCEPTION( bad_value_cast() );
      }
      virtual iterator_impl_base*       begin()      { return 0; }
      virtual const_iterator_impl_base* begin()const { return new const_map_iterator_impl<K,V>( this->get_val(), this->get_val().begin() ); }
  };

  template<typename T>
  struct place_holder_const_reference : place_holder {
      place_holder_const_reference( const T& v ):val(v){}

      virtual size_t        size()const { return get_field_map<T>().size(); }
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
      virtual iterator_impl_base*       begin()      { BOOST_THROW_EXCEPTION( bad_value_cast() ); return 0; }
      virtual const_iterator_impl_base* begin()const { return new const_iterator_impl<T>(val, get_field_map<T>().begin()); }

      const T& get_val()const { return val; }

      const T& val;
  };

  template<typename T,bool S = sizeof(T) <= sizeof(T*) >
  struct place_holder_value : place_holder {
      place_holder_value( const T& v ):val(new T(v)){}
      place_holder_value( T&& v ):val(new T(v)){}
      ~place_holder_value() { delete val; }

      virtual size_t        size()const { return get_field_map<T>().size(); }
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
      const T& get_val()const { return *val; }
      T&       get_val()      { return *val; }

      virtual iterator_impl_base*       begin()      { return new iterator_impl<T>(get_val(), get_field_map<T>().begin()); }
      virtual const_iterator_impl_base* begin()const { return new const_iterator_impl<T>(get_val(), get_field_map<T>().begin()); }

      T* val;
  };

  template<typename T>
  struct place_holder_value<T,true> : place_holder {
    BOOST_STATIC_ASSERT( sizeof(T) <= sizeof(T*) );
      place_holder_value( T&& v ):val(std::forward<T>(v)){}
      place_holder_value( const T& v ):val(v){}

      virtual size_t        size()const { return get_field_map<T>().size(); }
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

      virtual iterator_impl_base*       begin()      { return new iterator_impl<T>(val, get_field_map<T>().begin()); }
      virtual const_iterator_impl_base* begin()const { return new const_iterator_impl<T>(val, get_field_map<T>().begin()); }

      const T& get_val()const { return val; }
      T&       get_val()      { return val; }
      T val;
  };

  template<typename T> 
  struct select_holder { typedef place_holder_value<T> type; };
  template<typename T> 
  struct select_holder<T&> { typedef place_holder_reference<T> type; };
  template<typename T> 
  struct select_holder<const T&> { typedef place_holder_const_reference<T> type; };

  template<typename T> 
  struct select_holder<std::vector<T> > { typedef place_holder_array<std::vector<T>, place_holder_value<std::vector<T> > > type; };
  template<typename T> 
  struct select_holder<const std::vector<T>& > { typedef place_holder_const_array<std::vector<T>, place_holder_const_reference<std::vector<T> > > type; };
  template<typename T> 
  struct select_holder<std::vector<T>& > { typedef place_holder_array<std::vector<T>, place_holder_reference<std::vector<T> > > type; };

  
  template<typename K, typename V> 
  struct select_holder<std::map<K,V> > { typedef place_holder_map<K,V, place_holder_value<std::map<K,V> > > type; };
  template<typename K, typename V> 
  struct select_holder<const std::map<K,V>& > { typedef place_holder_const_map<K,V, place_holder_const_reference<std::map<K,V> > > type; };
  template<typename K, typename V> 
  struct select_holder<std::map<K,V>& > { typedef place_holder_map<K,V, place_holder_reference<std::map<K,V> > > type; };
  



} } } // boost::reflect::detail

#endif
