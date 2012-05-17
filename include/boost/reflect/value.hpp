#ifndef _BOOST_REFLECT_VALUE_HPP_
#define _BOOST_REFLECT_VALUE_HPP_
#include <typeinfo>
#include <new>
#include <string.h>
#include <boost/throw_exception.hpp>
#include <boost/unordered_map.hpp>
#include <boost/atomic.hpp>
#include <boost/reflect/reflect.hpp>
#include <boost/cmt/log/log.hpp>
#include <boost/reflect/error.hpp>

#include <boost/reflect/value_ref.hpp>
#include <boost/reflect/value_cref.hpp>

namespace boost { namespace reflect {

/**
 *  value ref has reference semantics, therefore it may only be
 *  assigned to reference types at construction
*/
class value {
    
    public:
        value();
        ~value();

        template<typename T>
        value( const T& v );

        template<typename T>
        value( T&& v );

        value( const value& v );
        value( value&& v );

        value( const value_cref& );
        value( const value_ref& );

        value_cref operator[]( const std::string& field )const;
        value_ref operator[]( const std::string& field );

        const std::type_info& type()const;

        template<typename T>
        inline operator T&(){
            T* v = ptr<T>();
            if( !v )
                boost::throw_exception(bad_value_cast());
            return *v;
        }

        template<typename T>
        inline operator const T&()const {
            const T* v = const_ptr<T>();
            if( !v )
                boost::throw_exception(bad_value_cast());
            return *v;
        }

        template<typename T>
        inline const T* const_ptr()const; 

        template<typename T>
        inline T* ptr(); 

        template<typename T>
        value& operator=( const T& v );
        
        template<typename T>
        value& operator=( T&& v );
        
        template<typename T>
        value& operator=( value&& v );
        
        template<typename T>
        value& operator=( const value& v );

        template<typename T>
        value& operator=( const value_ref& v );

        template<typename T>
        value& operator=( const value_cref& v );

        template<typename T>
        bool operator == ( const T& t )const {
           const T* v = ptr<T>();
           return v && *v == t;
        }

    private:
        char held[3*sizeof(void*)];
};


} } // namespace boost

#endif
