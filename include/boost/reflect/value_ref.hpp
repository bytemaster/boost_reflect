#ifndef _BOOST_REFLECT_VALUE_REF_HPP_
#define _BOOST_REFLECT_VALUE_REF_HPP_
#include <typeinfo>
#include <new>
#include <string.h>
#include <boost/reflect/value_cref.hpp>

namespace boost { namespace reflect {

/**
 *  value ref has reference semantics, therefore it may only be
 *  assigned to reference types at construction
*/
class value_ref {
    
    private:

        // const ref types need not apply
        template<typename T>
        value_ref( const T& );
        
        value_ref operator = ( const value_ref& v );
        friend class value;
    
    public:
        // you cannot copy references... 
        value_ref( const value_ref& );
        value_ref( value_ref&& );
        operator value_cref()const;

        value_ref();
        ~value_ref();

        bool operator!()const;

        template<typename T>
        explicit value_ref( T& v );

        value_ref operator[]( const std::string& field )const;

        const std::type_info& type()const;

        template<typename T>
        inline T& get()const;
        
        template<typename T>
        inline T* ptr()const;

        template<typename T>
        T& operator=( const T& r );

        template<typename T>
        bool operator == ( const T& t )const;

    private:
        char held[3*sizeof(void*)];
};


} } // namespace boost


#endif
