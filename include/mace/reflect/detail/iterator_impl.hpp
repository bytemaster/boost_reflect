#ifndef __MACE_REFLECT_DETAIL_ITERATOR_IMPL_HPP_
#define __MACE_REFLECT_DETAIL_ITERATOR_IMPL_HPP_

#include <mace/reflect/value_ref.hpp>
#include <vector>

namespace mace { namespace reflect { namespace detail {

    class const_iterator_impl_base {
      public:
        virtual ~const_iterator_impl_base(){}
        virtual std::string               key()const = 0;
        virtual value_cref                const_value()const = 0;
        virtual const_iterator_impl_base* const_clone()const = 0;

        virtual void next() = 0;
        virtual bool equals( const const_iterator_impl_base* other )const = 0;
        virtual const char* type()const = 0;
    };

    class iterator_impl_base : public const_iterator_impl_base {
      public:
        virtual value_ref           value()const = 0;
        virtual iterator_impl_base* clone()const = 0;
    };

    template<typename T>
    class const_vector_iterator_impl;

    template<typename T>
    class vector_iterator_impl : public iterator_impl_base {
      public:
        typename std::vector<T>::iterator itr;
        std::vector<T>&  vec;
        vector_iterator_impl( std::vector<T>& v, const typename std::vector<T>::iterator i ):vec(v),itr(i){}
    
        virtual std::string               key()const { return boost::lexical_cast<std::string>(itr-vec.begin()); }
        virtual value_ref                 value()const { return *itr; }
        virtual value_cref                const_value()const { return *itr; }
        virtual const_iterator_impl_base* const_clone()const { return new const_vector_iterator_impl<T>(vec,itr); }
        virtual iterator_impl_base*       clone()const { return new vector_iterator_impl(vec,itr); }
    
        virtual void next() { ++itr; }
        virtual bool equals( const const_iterator_impl_base* other )const{
            if( other == 0 )
               return itr == vec.end();
    
            if( type() == other->type() ) {
                return itr == static_cast<const vector_iterator_impl<T>*>(other)->itr;
            }
        }
        virtual const char* type()const  { return get_typename<vector_iterator_impl>(); }
    };

    template<typename T>
    class const_vector_iterator_impl : public const_iterator_impl_base {
      public:
        typename std::vector<T>::const_iterator itr;
        const std::vector<T>& vec;
        const_vector_iterator_impl( const std::vector<T>& v, const typename std::vector<T>::const_iterator& i ):vec(v),itr(i){}
    
        virtual std::string               key()const { return boost::lexical_cast<std::string>(itr-vec.begin()); }
        virtual value_cref                const_value()const { return *itr; }
        virtual const_iterator_impl_base* const_clone()const { return new const_vector_iterator_impl(vec,itr); }
    
        virtual void next() { ++itr; }
        virtual bool equals( const const_iterator_impl_base* other )const{
            if( other == 0 )
               return itr == vec.end();
    
            if( type() == other->type() ) {
                return itr == static_cast<const vector_iterator_impl<T>*>(other)->itr;
            }
        }
        virtual const char* type()const  { return get_typename<const_vector_iterator_impl>(); }
    };

    template<typename K, typename V>
    class const_map_iterator_impl;

    template<typename K,typename V>
    class map_iterator_impl : public iterator_impl_base {
      public:
        typename std::map<K,V>::iterator itr;
        std::map<K,V>&  vec;
        map_iterator_impl( std::map<K,V>& v, const typename std::map<K,V>::iterator i ):vec(v),itr(i){}
    
        virtual std::string               key()const { return boost::lexical_cast<std::string>(itr->first); }
        virtual value_ref                 value()const { return itr->second; }
        virtual value_cref                const_value()const { return itr->second; }
        virtual const_iterator_impl_base* const_clone()const { return new const_map_iterator_impl<K,V>(vec,itr); }
        virtual iterator_impl_base*       clone()const { return new map_iterator_impl(vec,itr); }
    
        virtual void next() { ++itr; }
        virtual bool equals( const const_iterator_impl_base* other )const{
            if( other == 0 )
               return itr == vec.end();
    
            if( type() == other->type() ) {
                return itr == static_cast<const map_iterator_impl<K,V>*>(other)->itr;
            }
        }
        virtual const char* type()const  { return get_typename<map_iterator_impl>(); }
    };

    template<typename K, typename V>
    class const_map_iterator_impl : public const_iterator_impl_base {
      public:
        typename std::map<K,V>::const_iterator itr;
        const std::map<K,V>& vec;
        const_map_iterator_impl( const std::map<K,V>& v, const typename std::map<K,V>::const_iterator& i ):vec(v),itr(i){}
    
        virtual std::string               key()const { return boost::lexical_cast<std::string>(itr->first); }
        virtual value_cref                const_value()const { return itr->second; }
        virtual const_iterator_impl_base* const_clone()const { return new const_map_iterator_impl(vec,itr); }
    
        virtual void next() { ++itr; }
        virtual bool equals( const const_iterator_impl_base* other )const{
            if( other == 0 )
               return itr == vec.end();
    
            if( type() == other->type() ) {
                return itr == static_cast<const map_iterator_impl<K,V>*>(other)->itr;
            }
        }
        virtual const char* type()const  { return get_typename<const_map_iterator_impl>(); }
    };

    
} } }



#endif // __BOOST_REFLECT_DETAIL_ITERATOR_IMPL_HPP_
