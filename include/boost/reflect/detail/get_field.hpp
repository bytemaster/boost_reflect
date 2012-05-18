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
value_cref get_field( const std::string& n, const T& v  );

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
bool has_field( const std::string& n ) {
  field_map_type::const_iterator itr = get_field_map<T>().find(n);
  return itr != get_field_map<T>().end();
}
template<typename T>
value_ref get_field( const std::string& n, T& v  ) {
  field_map_type::const_iterator itr = get_field_map<T>().find(n);
  if( itr != get_field_map<T>().end() )
    return (*itr->second)(&v);
  BOOST_THROW_EXCEPTION( unknown_field() << err_msg(n) );
}
template<typename T>
value_cref get_field( const std::string& n, const T& v  ) {
  field_map_type::const_iterator itr = get_field_map<T>().find(n);
  if( itr != get_field_map<T>().end() )
    return (*itr->second)(&v);
  BOOST_THROW_EXCEPTION( unknown_field() << err_msg(n) );
}

} } // boost::reflect

#endif
