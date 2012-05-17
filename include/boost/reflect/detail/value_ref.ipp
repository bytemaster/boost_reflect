#ifndef _BOOST_REFLECT_DETAIL_VALUE_REF_IPP_
#define _BOOST_REFLECT_DETAIL_VALUE_REF_IPP_
#include <boost/reflect/value_ref.hpp>
#include <boost/reflect/detail/get_field.hpp>

namespace boost { namespace reflect {
    // intentionally not implemented...
    //value_ref::value_ref( const value_ref& ){};

    // const ref types need not apply
    //template<typename T>
    //value_ref::value_ref( const T& ){};

     value_ref::value_ref()  { new (held) detail::place_holder();              }
     value_ref::~value_ref() { ((detail::place_holder*)held)->~place_holder(); }

     value_ref::value_ref( const value_ref& r ) {
        slog( "clone\n" );
        ((detail::place_holder*)r.held)->clone(held);
     }
      value_ref::value_ref( value_ref&& v ) {
        slog("rval" );
        memcpy( held, v.held, sizeof(held) ); 
        new (v.held) detail::place_holder();
     }

     bool value_ref::operator!()const {
      return ((detail::place_holder*)held)->get_cref() == typeid(detail::place_holder);
     }

     value_ref::operator value_cref()const {
        slog( "get_cref" );
        return ((detail::place_holder*)held)->get_cref();
     }
     template<typename T>
     value_ref::value_ref( T& v ) { 
      BOOST_STATIC_ASSERT( sizeof(detail::place_holder_lvalue<T>) <= sizeof(held) );
      new (held) detail::place_holder_lvalue<T>(v); 
     }

     value_ref value_ref::operator[]( const std::string& field )const {
       return ((detail::place_holder*)held)->get_field( field );
     }

     const std::type_info& value_ref::type()const { 
       return ((detail::place_holder*)held)->type(); 
     }

     template<typename T>
     inline T& value_ref::get()const {

        slog( "%1%", get_typename<T>() );
         detail::place_holder_ref<T>* p = 
             dynamic_cast<detail::place_holder_ref<T>*>((detail::place_holder*)held);
         if( p ) return p->ref();
         boost::throw_exception(bad_value_cast());
     }

     template<typename T>
     inline T* value_ref::ptr()const {
         detail::place_holder_ref<T>* p = 
             dynamic_cast<detail::place_holder_ref<T>*>((detail::place_holder*)held);
         if( p ) 
             return &p->ref();
         return 0;
     }

     template<typename T>
     T& value_ref::operator=( const T& r ) {
         return get<T>() = r;
     }

     template<typename T>
     bool value_ref::operator == ( const T& t )const {
        slog( "%1%", get_typename<T>() );
        const T* v = ptr<T>();
        return v && *v == t;
     }


} } // namespace boost::reflect

#endif // _BOOST_REFLECT_DETAIL_VALUE_REF_IPP_
