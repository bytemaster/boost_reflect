
#ifndef _BOOST_REFLECT_VALUE_CREF_HPP_
#define _BOOST_REFLECT_VALUE_CREF_HPP_

#include <typeinfo>
#include <new>
#include <string.h>
#include <boost/throw_exception.hpp>
#include <boost/unordered_map.hpp>
#include <boost/atomic.hpp>
#include <boost/reflect/reflect.hpp>
#include <boost/reflect/error.hpp>

namespace boost { namespace reflect {

/**
 *  value ref has reference semantics, therefore it may only be
 *  assigned to reference types at construction
*/
class value_cref {
    public:
      friend class value;
        template<typename T>
        explicit value_cref( const T&& t ); 

        explicit value_cref( value_cref&& t ); 
        
        template<typename T>
        explicit value_cref( const T& v );

        value_cref();
        ~value_cref(); 

        bool operator!()const;
        
        /**
         *  Return a field on this type as defined by BOOST_REFLECT()
         *  macro.
         */
        value_cref operator[]( const std::string& field )const;

        const std::type_info& type()const;

        /**
         *  Attempt cast or throw bad_value_cast on failure.
         */
        template<typename T>
        inline const T& get()const; 

        /** Attempt to cast to T* and return
         * NULL on failure. */
        template<typename T>
        inline const T* ptr()const;

        /** Attempt cast to T then compare with t
         *  @return false if cast or compare failed.  */
        template<typename T>
        bool operator == ( const T& t )const; 

        value_cref( const value_cref& c ); 

    private:
        template<typename T>
        value_cref operator = (T){ return *this; }

        char held[2*sizeof(void*)];
};


} } // namespace boost


#endif
