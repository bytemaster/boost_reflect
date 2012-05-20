#ifndef _BOOST_REFLECT_DETAIL_PLACE_HOLDER_HPP_
#define _BOOST_REFLECT_DETAIL_PLACE_HOLDER_HPP_
#include <boost/reflect/detail/get_field.hpp>
#include <boost/reflect/value_visitor.hpp>
#include <boost/reflect/detail/iterator_impl.hpp>
#include <map>
#include <boost/fusion/include/is_sequence.hpp>
#include <boost/fusion/include/deduce.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/mpl/if.hpp>

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


  template<typename T, bool StackAllocate = true >
  struct place_holder_impl : place_holder {
      place_holder_impl( const T& v ):val(new T(v)){}
      place_holder_impl( T&& v ):val(new T(v)){}
      ~place_holder_impl() { delete val; }

      virtual size_t        size()const { return get_field_map<T>().size(); }
      virtual const char*   type()const { return get_typename<T>(); }
      virtual void*         ptr()       { return val;               }
      virtual const void*   ptr()const  { return val;               }
      virtual place_holder* clone( char* place = 0 )const {
        if( place ) return new (place) place_holder_impl(*val);
        return new place_holder_impl(*val);
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
  struct place_holder_impl<T,false> : place_holder {
      place_holder_impl( T&& v ):val(std::forward<T>(v)){}
      place_holder_impl( const T& v ):val(v){}

      virtual size_t        size()const { return get_field_map<T>().size(); }
      virtual const char*   type()const { return get_typename<T>(); }
      virtual void*         ptr()       { return &val;              }
      virtual const void*   ptr()const  { return &val;              }
      virtual place_holder* clone( char* place = 0 )const {
        if( place ) return new (place) place_holder_impl(val);
        return new place_holder_impl(val);
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
  struct place_holder_impl<T&> : place_holder {
      place_holder_impl( T& v ):val(v){}

      virtual size_t        size()const { return get_field_map<T>().size(); }
      virtual const char*   type()const { return get_typename<T>(); }
      virtual void*         ptr()       { return &val;              }
      virtual const void*   ptr()const  { return &val;              }
      virtual place_holder* clone( char* place = 0 )const {
        if( place ) return new (place) place_holder_impl(val);
        return new place_holder_impl(val);
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

  template<typename T>
  struct place_holder_impl<const T&> : place_holder {
      place_holder_impl( const T& v ):val(v){}

      virtual size_t        size()const { return get_field_map<T>().size(); }
      virtual const char*   type()const { return get_typename<T>(); }
      virtual void*         ptr()       { return 0;                 }
      virtual const void*   ptr()const  { return &val;              }
      virtual place_holder* clone( char* place = 0 )const {
        if( place ) return new (place) place_holder_impl(val);
        return new place_holder_impl(val);
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


  template<typename T, bool StackAllocate = true> 
  struct select_place_holder { typedef place_holder_impl<T,StackAllocate> type; };

  template<typename T>
  struct select_place_holder<std::vector<T>,false > : public place_holder_array<std::vector<T>, place_holder_impl<std::vector<T>,false > > {
    select_place_holder( const std::vector<T>& v ):place_holder_array<std::vector<T>, place_holder_impl<std::vector<T>,false > >(v){}
    select_place_holder( std::vector<T>&& v ):place_holder_array<std::vector<T>, place_holder_impl<std::vector<T>,false > >(std::forward<std::vector<T> >(v)){}

    typedef select_place_holder type;
  };
  template<typename T>
  struct select_place_holder<std::vector<T>& > : public place_holder_array<std::vector<T>, place_holder_impl<std::vector<T>& > > {
    select_place_holder(  std::vector<T>& v ):place_holder_array<std::vector<T>, place_holder_impl<std::vector<T>& > >(v){};
    typedef select_place_holder type;
  };
  template<typename T>
  struct select_place_holder<const std::vector<T>& > : public  place_holder_const_array<std::vector<T>, place_holder_impl<const std::vector<T>& > > {
    select_place_holder( const std::vector<T>& v ):place_holder_const_array<std::vector<T>, place_holder_impl<const std::vector<T>& > >(v){};
    typedef select_place_holder type;
  };

  template<typename K,typename V>
  struct select_place_holder<std::map<K,V>,false > : public place_holder_array<std::map<K,V>, place_holder_impl<std::map<K,V>,false > > {
    select_place_holder( const std::map<K,V>& v ):place_holder_map<K,V, place_holder_impl<std::map<K,V>,false > >(v){}
    select_place_holder( std::map<K,V>&& v ):place_holder_map<K,V, place_holder_impl<std::map<K,V>,false > >(std::forward<std::map<K,V> >(v)){}
    typedef select_place_holder type;
  };
  template<typename K,typename V>
  struct select_place_holder<std::map<K,V>& > : public place_holder_map<K,V, place_holder_impl<std::map<K,V>& > > {
    select_place_holder(  std::map<K,V>& v ):place_holder_map<K,V, place_holder_impl<std::map<K,V>& > >(v){};
    typedef select_place_holder type;
  };
  template<typename K,typename V>
  struct select_place_holder<const std::map<K,V>& > : public  place_holder_const_map<K,V, place_holder_impl<const std::map<K,V>& > > {
    select_place_holder( const std::map<K,V>& v ):place_holder_const_map<K,V, place_holder_impl<const std::map<K,V>& > >(v){};
    typedef select_place_holder type;
  };

  
  template<typename Seq>
  class sequence_iterator_impl {};

  template<typename Seq>
  class sequence_iterator_impl<Seq&> : public iterator_impl_base {
    public:
      Seq&  seq;
      int  idx;

      sequence_iterator_impl( int i, Seq& s ):seq(s),idx(i){}
  
      virtual std::string               key()const { return boost::lexical_cast<std::string>(idx); }
      virtual value_ref                 value()const { return get_value_ref( seq, idx ); }
      virtual value_cref                const_value()const { return get_value_ref(seq,idx); }
      virtual const_iterator_impl_base* const_clone()const { return new sequence_iterator_impl<const Seq&>(idx,seq); }
      virtual iterator_impl_base*       clone()const { return new sequence_iterator_impl<Seq&>(idx,seq); }
  
      virtual void next() { ++idx; }
      virtual bool equals( const const_iterator_impl_base* other )const{
          if( other == 0 )
             return idx == boost::fusion::result_of::size<Seq>::value;
  
          if( type() == other->type() ) {
              return idx == static_cast<const sequence_iterator_impl<const Seq&>*>(other)->idx;
          }
      }
      virtual const char* type()const  { return get_typename<sequence_iterator_impl>(); }
  };

  template<typename Seq>
  class sequence_iterator_impl<const Seq&> : public const_iterator_impl_base {
    public:
      const Seq& seq;
      int        idx;

      sequence_iterator_impl( int i, const Seq& s ):seq(s),idx(i){}
  
      virtual std::string               key()const { return boost::lexical_cast<std::string>(idx); }
      virtual value_cref                const_value()const { return get_value_ref(seq,idx); }
      virtual const_iterator_impl_base* const_clone()const { return new sequence_iterator_impl<const Seq&>(idx,seq); }
  
      virtual void next() { ++idx; }
      virtual bool equals( const const_iterator_impl_base* other )const{
          if( other == 0 )
             return idx == boost::fusion::result_of::size<Seq>::value;
  
          if( type() == other->type() ) {
              return idx == static_cast<const sequence_iterator_impl<const Seq&>*>(other)->idx;
          }
      }
      virtual const char* type()const  { return get_typename<sequence_iterator_impl>(); }
  };

  template<typename Seq>
  struct get_value_ref_visitor {
    mutable int                 idx;
    int                         target;
    mutable char                ref[sizeof(value_ref)];

    value_ref result()const {
      if( *((int64_t*)ref) == 0 ) 
        BOOST_THROW_EXCEPTION( std::out_of_range( boost::lexical_cast<std::string>(target) ) ); 
      return *static_cast<value_ref*>((void*)ref); 
    }
    get_value_ref_visitor(int t=0):idx(0),target(t){}

    template<typename T>
    inline void operator()( T& v )const {
      if( idx == target ) new(ref)value_ref(v);
      ++idx;
    }
  };

  template<typename Seq>
  struct get_value_ref_visitor<const Seq&> {
    mutable int                 idx;
    int                         target;
    mutable char                ref[sizeof(value_cref)];
    value_cref&  result()const {
      if( *((int64_t*)ref) == 0 ) 
        BOOST_THROW_EXCEPTION( std::out_of_range( boost::lexical_cast<std::string>(target) ) ); 
      return *static_cast<value_cref*>((void*)ref); 
    }
    get_value_ref_visitor(int t=0):idx(0),target(t){memset(ref,0,sizeof(ref));}

    template<typename T>
    inline void operator()( const T& v )const {
      if( idx == target ) new(ref)value_cref(v);
      ++idx;
    }
  };

  template<typename Seq>
  value_cref get_value_ref( const Seq& s, int idx ) {
    get_value_ref_visitor<const Seq&> get_idx(idx);
    boost::fusion::for_each( s, get_idx );
    return get_idx.result();
  }
  template<typename Seq>
  value_ref get_value_ref( Seq& s, int idx ) {
    get_value_ref_visitor<Seq&> get_idx(idx);
    boost::fusion::for_each( s, get_idx );
    return get_idx.result();
  }


  template<typename Seq, typename Base> 
  struct place_holder_sequence : public Base {
      place_holder_sequence(){}
      place_holder_sequence( Seq v ):Base(v){}
      virtual bool   is_array()const { return true;       }
      virtual size_t size()const     { return boost::fusion::result_of::size<typename boost::fusion::traits::deduce<Seq>::type>::value; }
      virtual bool   has_field( const std::string& f )const { 
        try {
            uint64_t idx = boost::lexical_cast<uint64_t>(f);
            return idx < size();
        } catch ( ... ) { return false; } // bad lexical cast
      }
      virtual value_ref  get_field( const std::string& f ) {
            uint64_t idx = boost::lexical_cast<uint64_t>(f);
            return get_value_ref( this->get_val(), idx );
      }
      virtual value_cref get_field( const std::string& f )const{
            uint64_t idx = boost::lexical_cast<uint64_t>(f);
            return get_value_ref( this->get_val(), idx );
      }
      virtual value_ref  get_field( uint64_t idx )      { return get_value_ref( this->get_val(), idx ); }
      virtual value_cref get_field( uint64_t idx )const { return get_value_ref( this->get_val(), idx ); }
      virtual iterator_impl_base*       begin()      { return new sequence_iterator_impl<Seq&>( 0,this->get_val()); }
      virtual const_iterator_impl_base* begin()const { return new sequence_iterator_impl<const Seq&>( 0,this->get_val());}
  };
  template<typename Seq, typename Base> 
  struct place_holder_sequence<const Seq&,Base> : public Base {
      place_holder_sequence(){}
      place_holder_sequence( const Seq& v ):Base(v){}
      virtual bool   is_array()const { return true;       }
      virtual size_t size()const     { return boost::fusion::result_of::size<typename boost::fusion::traits::deduce<Seq>::type>::value; }
      virtual bool   has_field( const std::string& f )const { 
        try {
            uint64_t idx = boost::lexical_cast<uint64_t>(f);
            return idx < size();
        } catch ( ... ) { return false; } // bad lexical cast
      }
      virtual value_cref get_field( const std::string& f )const{
            uint64_t idx = boost::lexical_cast<uint64_t>(f);
            return get_value_ref( this->get_val(), idx );
      }
      virtual value_cref get_field( uint64_t idx )const { return get_value_ref( this->get_val(), idx ); }
      virtual iterator_impl_base*       begin()      { return 0; }
      virtual const_iterator_impl_base* begin()const { return new sequence_iterator_impl<const Seq&>( 0,this->get_val());}
  };

  template<typename Seq, bool StackAllocate = true> 
  struct select_sequence_holder: public place_holder_sequence<Seq, place_holder_impl<Seq,StackAllocate > > {
    select_sequence_holder( const Seq& v ):place_holder_sequence<Seq, place_holder_impl<Seq,StackAllocate > >(v){}
    select_sequence_holder( Seq&& v ):place_holder_sequence<Seq, place_holder_impl<Seq,StackAllocate > >(std::forward<Seq >(v)){}
    typedef select_sequence_holder type;
  };
  template<typename Seq>
  struct select_sequence_holder<Seq& > : public place_holder_sequence<Seq, place_holder_impl<Seq& > > {
    select_sequence_holder(  Seq& v ):place_holder_sequence<Seq, place_holder_impl<Seq& > >(v){};
    typedef select_sequence_holder type;
  };
  template<typename Seq>
  struct select_sequence_holder<const Seq& > : public  place_holder_sequence<const Seq&, place_holder_impl<const Seq& > > {
    select_sequence_holder( const Seq& v ):place_holder_sequence<const Seq&, place_holder_impl<const Seq& > >(v){};
    typedef select_sequence_holder type;
  };


  template<typename T>
  struct select_holder {
    typedef typename boost::mpl::if_c<
        boost::fusion::traits::is_sequence<typename boost::fusion::traits::deduce<T>::type >::value,
        typename select_sequence_holder<T, sizeof(typename select_sequence_holder<T,true>::type ) <= 3*sizeof(void*)>::type,
        typename select_place_holder   <T, sizeof(typename select_place_holder   <T,true>::type ) <= 3*sizeof(void*)>::type 
        >::type type;
  };


} } } // boost::reflect::detail

#endif
