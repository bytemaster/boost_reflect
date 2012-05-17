#ifndef _BOOST_REFLECT_DETAIL_GET_FIELD_HPP
#define _BOOST_REFLECT_DETAIL_GET_FIELD_HPP
#include <boost/throw_exception.hpp>
#include <boost/unordered_map.hpp>
#include <boost/atomic.hpp>
#include <boost/reflect/reflect.hpp>
#include <boost/reflect/error.hpp>
#include <boost/reflect/value_ref.hpp>
#include <boost/reflect/value_cref.hpp>

namespace boost { namespace reflect { 

namespace detail { struct get_field_method; }

typedef boost::unordered_map<std::string,detail::get_field_method*> field_map_type;

template<typename T>
value_ref get_field( const std::string& n, T& v  );

template<typename T>
value_cref get_const_field( const std::string& n, const T& v  );

namespace detail {

      struct get_field_method {
        public:
            get_field_method(  const char* n ):m_name(n){}
            virtual value_ref  operator()( void* f )const = 0;
            virtual value_cref operator()( const void* f )const = 0;
            const char* name()const { return m_name; }
        private:
            const char* m_name;
      };

      template<typename T,typename C, T(C::*p)>
      struct get_field_method_impl : get_field_method {
        get_field_method_impl( const char* n ):get_field_method(n){}
        virtual value_ref operator()( void* f )const {
           return value_ref((((C*)f)->*p));
        }
        virtual value_cref  operator()( const void* f )const {
           return value_cref(((const C*)f)->*p);
        }
      };

      class get_field_visitor {
        public:
          get_field_visitor( field_map_type& _field_map )
          :field_map(_field_map){}
     
          template<typename T,typename C, T(C::*p)>
          inline void operator()( const char* name )const {
            static get_field_method_impl<T,C,p> gfm(name);
            field_map[name] = &gfm;
          }

          //TODO: visit member methods
          
          field_map_type& field_map;
      };
      
      /**
       *  Atomicly creates a field map, deletes old one if there
       *  was a race condition on creating it. 
       */
      template<typename T>
      static field_map_type* create_field_map() {
        static boost::atomic<field_map_type*> fm(0);
        field_map_type* n = new field_map_type();
        boost::reflect::reflector<T>::visit( get_field_visitor(*n) );
        delete fm.exchange(n,boost::memory_order_consume);
        return fm;
      }

        /**
         *  Base class for all place_holder types.
         */
        struct place_holder {
            virtual ~place_holder(){}
            virtual void* clone( char* l )const { return new(l)place_holder(); }
            virtual const std::type_info & type() const { 
                return typeid(place_holder); 
            }
            virtual value_ref get_field( const std::string& f )const {
                boost::throw_exception(bad_value_cast());
            }
            virtual value_cref get_const_field( const std::string& f )const {
                boost::throw_exception(bad_value_cast());
            }
            virtual value_cref get_cref()const { return value_cref(); }
        };

        /**
         *  Place holders have different implementations based upon
         *  whether they are storing:
         *  
         *  1) const lvalue reference
         *  2) const rvalue reference (temporary object required )
         *  3) lvalue reference
         *  4) small value <= sizeof(void*) stored internally 
         *  5) large value > sizeof(void*)  allocated on heap
         *
         *  Regard
         */
        template<typename T>
        struct place_holder_const : virtual public place_holder {
            value_cref get_const_field( const std::string& f )const  {
             // slog( "place_holder_ref::get_const_field %1%", f );
              return reflect::get_const_field(f,cref()); 
            }
        
            virtual const T& cref()const  = 0;
            virtual value_cref get_cref()const { return value_cref(cref()); }

            virtual const std::type_info & type() const {
                return typeid(const T&);
            }
        };
        template<typename T>
        struct place_holder_ref : virtual public place_holder {
            virtual T& ref()const  = 0;
            virtual value_cref get_cref()const { return value_cref(ref()); }
            virtual const std::type_info & type() const {
                return typeid(T&);
            }
            virtual value_ref get_field( const std::string& f )const {
              //slog( "place_holder_ref::get_field %1%", f );
              return reflect::get_field(f,ref()); 
            }
        };

        template<typename T>
        struct place_holder_const_lvalue : public place_holder_const<T> {
            explicit place_holder_const_lvalue( const T& v )
            :m_cref(v){}

            virtual const T& cref()const { return m_cref; }
            virtual void* clone( char* l )const { return new(l)place_holder_const_lvalue(m_cref); }

            private:
                const T& m_cref;
        };


        template<typename T, bool SizeLessThanOrEqualToVoidPtr = false>
        struct place_holder_const_rvalue_impl : public place_holder_const<T> {
            place_holder_const_rvalue_impl( const T&& v )
            :m_rval( new T( std::forward<const T>(v) ) ){}

            place_holder_const_rvalue_impl( const place_holder_const_rvalue_impl& v )
            :m_rval( new T( *v.m_rval) ){}

            ~place_holder_const_rvalue_impl() { delete m_rval; }

            virtual const T& cref()const { return *m_rval; }
            virtual void* clone( char* l )const { return new(l)place_holder_const_rvalue_impl(*this); }
            private: 
                const T* m_rval;
        };

        template<typename T>
        struct place_holder_const_rvalue_impl<T,true> : public place_holder_const<T> {
            place_holder_const_rvalue_impl( const T&& v )
            :m_rval( std::forward<const T>(v) ){}
            virtual const T& cref()const { return m_rval; }
            virtual void* clone( char* l )const { return new(l)place_holder_const_rvalue_impl(T(m_rval)); }
            private: 
                const T m_rval;
        };

        template<typename T>
        struct place_holder_const_rvalue : public place_holder_const_rvalue_impl<T,sizeof(T)<=sizeof(void*)> {
            place_holder_const_rvalue( const T&& v )
            :place_holder_const_rvalue_impl<T,sizeof(T)<=sizeof(void*)>( std::forward<const T>(v) ){}
        };

        template<typename T>
        struct place_holder_lvalue : public place_holder_const<T>, public place_holder_ref<T> {
            place_holder_lvalue( T& v )
            :m_ref(v){ }

            virtual const T& cref()const { return m_ref; }
            virtual T&        ref()const { return m_ref; }
            virtual void* clone( char* l )const { return new(l)place_holder_lvalue(m_ref); }

            virtual value_cref get_cref()const { return value_cref(cref()); }
            virtual const std::type_info & type() const {
                return typeid(T&);
            }

            T& m_ref;
        };


        template<typename T, bool SizeLessThanOrEqualToVoidPtr = false>
        struct place_holder_value_impl : public place_holder_const<T>, public place_holder_ref<T> {
            place_holder_value_impl( T&& v )
            :m_rval( new T( std::forward<T>(v) ) ){}

            place_holder_value_impl( const T& v )
            :m_rval( new T(v) ){}

            place_holder_value_impl( const place_holder_value_impl& v )
            :m_rval( new T( *v.m_rval) ){}

            ~place_holder_value_impl() { delete m_rval; }

            virtual const T& cref()const { return *m_rval; }
            virtual T& ref()const        { return *m_rval; }
            virtual void* clone( char* l )const { return new(l)place_holder_value_impl(*this); }

            virtual value_cref get_cref()const { return cref(); }
            virtual const std::type_info & type() const {
                return typeid(T);
            }
            private: 
                T* m_rval;
        };


        template<typename T>
        struct place_holder_value_impl<T,true> : public place_holder_const<T>, public place_holder_ref<T> {
            place_holder_value_impl( const T& v )
            :m_rval( v ){}
            place_holder_value_impl( T&& v )
            :m_rval( std::forward<T>(v) ){}
            virtual const T& cref()const { return m_rval; }
            virtual T&       ref()const  { return m_rval; }
            virtual void* clone( char* l )const { return new(l)place_holder_value_impl(m_rval); }
            virtual value_cref get_cref()const { return cref(); }
            virtual const std::type_info & type() const {
                return typeid(T);
            }
            private: 
                mutable T m_rval;
        };

        template<typename T>
        struct place_holder_value : public place_holder_value_impl<T,sizeof(T)<=sizeof(void*)> {
            place_holder_value( const T&& v )
            :place_holder_value_impl<T,sizeof(T)<=sizeof(void*)>( std::forward<const T>(v) ){}
        };



 } // namespace detail


 /**
  *  returns the singlton field map for T
  */
template<typename T>
const field_map_type& get_field_map() {
    static field_map_type* fm = detail::create_field_map<T>();
    return *fm;
}

template<typename T>
value_ref get_field( const std::string& n, T& v  ) {
    field_map_type::const_iterator itr = get_field_map<T>().find(n);
    if( itr != get_field_map<T>().end() )
      return (*itr->second)(&v);
    return value_ref();
}
template<typename T>
value_cref get_const_field( const std::string& n, const T& v  ) {
    field_map_type::const_iterator itr = get_field_map<T>().find(n);
    if( itr != get_field_map<T>().end() )
      return (*itr->second)(&v);
    return value_cref();
}

} } // boost::reflect

#endif
