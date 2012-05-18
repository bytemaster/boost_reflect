#ifndef _BOOST_REFLECT_VALUE_REF_HPP_
#define _BOOST_REFLECT_VALUE_REF_HPP_
#include <boost/reflect/value_cref.hpp>

namespace boost { namespace reflect {

/**
 *  value ref has reference semantics, therefore it may only be
 *  assigned to reference types at construction
*/
class value_ref : public value_base  {
    
    public:
        template<typename T>
        value_ref( T& v );
        value_ref( const value_ref& );
        value_ref( value_ref&& );

        operator value_cref()const;
        value_cref operator[]( const std::string& field )const;
        value_ref  operator[]( const std::string& field );

        template<typename T>
        value_ref& operator=( const T& r );

    private:
        friend class value;
        // references must be provided a value at construction
        value_ref(){}
        value_ref(const class value_cref& ){}
        value_ref(const class value& ){}

        // you cannot assign references after construction
        template<typename T>
        value_ref operator = (T){ return *this; }
        value_ref operator = ( const value_ref& v );

        // const ref types need not apply
        template<typename T>
        value_ref( const T& );
};


} } // namespace boost


#endif
