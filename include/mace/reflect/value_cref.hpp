#ifndef _MACE_REFLECT_VALUE_CREF_HPP_
#define _MACE_REFLECT_VALUE_CREF_HPP_
#include <mace/reflect/value_base.hpp>

namespace mace { namespace reflect {

/**
 *  value ref has reference semantics, therefore it may only be
 *  assigned to reference types at construction
*/
class value_cref : public value_base {
    public:
        template<typename T> value_cref( const T& v );
        template<typename T> value_cref( const T&& t ); 
        value_cref( const struct value& t ); 

        value_cref( const value_cref& c ); 
        value_cref( value_cref&& t ); 
        
        /**
         *  Return a field on this type as defined by BOOST_REFLECT()
         *  macro.
         */
        value_cref operator[]( const std::string& field )const;

        template<typename T>
        inline const T& get()const;

        template<typename T>
        inline const T* const_ptr()const; 
    private:
        friend class value;

        template<typename T>
        inline T& get();

        template<typename T>
        inline T* ptr(); 

        using value_base::get;
        using value_base::ptr;
        using value_base::set_as;

        value_cref(){};

        /** Cannot modify const ref */
        template<typename T>
        void set_as(const T&&); 

        template<typename T>
        value_cref operator = (T){ return *this; }
        value_cref operator = ( const value_cref& v );
};


} } // namespace mace::reflect


#endif
