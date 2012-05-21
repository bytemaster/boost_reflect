#ifndef _BOOST_REFLECT_DETAIL_GET_FIELD_HPP
#define _BOOST_REFLECT_DETAIL_GET_FIELD_HPP
#include <boost/throw_exception.hpp>
#include <boost/unordered_map.hpp>
#include <boost/atomic.hpp>
#include <boost/reflect/reflect.hpp>
#include <boost/reflect/error.hpp>
#include <boost/reflect/value_ref.hpp>
#include <boost/reflect/value_cref.hpp>
#include <boost/reflect/iterator.hpp>
#include <boost/reflect/detail/iterator_impl.hpp>
#include <boost/function_types/components.hpp>

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

      template<typename P, P p >
      struct get_field_method_impl : get_field_method {
        get_field_method_impl( const char* n ):get_field_method(n){}
        virtual value_ref operator()( void* f )const {
            typedef typename mpl::at_c< function_types::components<P,mpl::identity<mpl::_> >,1 >::type C;
           return value_ref((((C*)f)->*p));
        }
        virtual value_cref  operator()( const void* f )const {
            typedef typename mpl::at_c< function_types::components<P,mpl::identity<mpl::_> >,1 >::type C;
            return value_cref((((const C*)f)->*p));
        }
      };

      class get_field_visitor {
        public:
          get_field_visitor( field_map_type& _field_map )
          :field_map(_field_map){}

          template<typename MemberPointerType,  MemberPointerType p >
          inline void operator()( const char* name )const {
            static get_field_method_impl<MemberPointerType,p> gfm(name);
            field_map[name] = &gfm;
          }
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
  BOOST_THROW_EXCEPTION( unknown_field() << err_msg(n+": '"+n+"'") );
}
template<typename T>
value_cref get_field( const std::string& n, const T& v  ) {
  field_map_type::const_iterator itr = get_field_map<T>().find(n);
  if( itr != get_field_map<T>().end() )
    return (*itr->second)(&v);
  BOOST_THROW_EXCEPTION( unknown_field() << err_msg(n+": '"+n+"'") );
}

template<typename T>
class iterator_impl;

template<typename T>
class const_iterator_impl : public detail::const_iterator_impl_base {
  public:
    const T&                       val;
    field_map_type::const_iterator itr;

    const_iterator_impl( const T& _val, const field_map_type::const_iterator& i )
    :val(_val),itr(i){}

    const_iterator_impl( const const_iterator_impl& c )
    :val(c.val),itr(c.itr){}

    virtual std::string               key()const         { return itr->first;                        }
    virtual value_ref                 value()const       { BOOST_THROW_EXCEPTION( bad_value_cast() );}
    virtual value_cref                const_value()const { return (*itr->second)(&val);              }
    virtual const_iterator_impl_base* const_clone()const { return new const_iterator_impl(val,itr);  }
    virtual const char*               type()const        { return get_typename<iterator_impl<T> >(); }

    virtual void next() { ++itr; }
    virtual bool equals( const const_iterator_impl_base* other )const {
      if( other == 0 )
         return itr == get_field_map<T>().end();

      if( type() == other->type() ) {
          return itr == static_cast<const const_iterator_impl<T>*>(other)->itr;
                &val == &static_cast<const const_iterator_impl<T>*>(other)->val;
      }
    }
};
template<typename T>
class iterator_impl : public detail::iterator_impl_base {
  public:
    T&                             val;
    field_map_type::const_iterator itr;

    iterator_impl( T& _val, const field_map_type::const_iterator& i )
    :val(_val),itr(i){}

    iterator_impl( const iterator_impl& c )
    :val(c.val),itr(c.itr){}

    virtual std::string          key()const { return itr->first; }
    virtual value_ref            value()const       { return (*itr->second)(&val);      }
    virtual value_cref           const_value()const { return (*itr->second)(&val);      }
    virtual iterator_impl_base* clone()const { return new iterator_impl(val,itr);       }
    virtual const_iterator_impl_base* const_clone()const { return new const_iterator_impl<T>(val,itr);  }
    virtual const char*          type()const { return get_typename<iterator_impl<T> >(); }

    virtual void next() { ++itr; }
    virtual bool equals( const const_iterator_impl_base* other )const {
      if( other == 0 )
         return itr == get_field_map<T>().end();

      if( type() == other->type() ) {
          return itr == static_cast<const iterator_impl<T>*>(other)->itr;
                &val == &static_cast<const iterator_impl<T>*>(other)->val;
      }
    }
};


} } // boost::reflect

#endif
