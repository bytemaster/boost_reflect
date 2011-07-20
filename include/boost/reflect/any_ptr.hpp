#ifndef _BOOST_REFLECT_ANY_PTR_HPP
#define _BOOST_REFLECT_ANY_PTR_HPP
#include <boost/shared_ptr.hpp>
#include <boost/any.hpp>
#include <boost/reflect/vtable.hpp>
#include <boost/reflect/mirror_interface.hpp>

namespace boost { namespace reflect {

    /**
     *  @class any_ptr
     *  @brief Behaves like a smart pointer that can handle any type with the same interface.
     *
     *  If constructed from a shared pointer, then a copy of the shared pointer will go with
     *  every any_ptr.  If constructed from a regular pointer, then the pointer must be valid
     *  for the life of all copies of the any_ptr.
     *
     */
    template<typename InterfaceType, typename InterfaceDelegate = boost::reflect::mirror_interface>
    class any_ptr {
        public:
            typedef boost::reflect::vtable<InterfaceType,InterfaceDelegate> vtable_type;
            typedef InterfaceType                                           interface_type;
            any_ptr()
            :m_vtable(new vtable_type()) {}

            operator bool()const  { return m_vtable; }
            bool operator!()const { return !m_vtable; }

            template<typename T>
            any_ptr( T* v )
            :m_vtable(new vtable_type()) {
                typename InterfaceDelegate::template set_visitor<T> vi(v);
                reflector<interface_type>::visit( *m_vtable, vi );
            }
            template<typename T>
            any_ptr( const boost::shared_ptr<T>& v )
            :m_ptr(v),m_vtable(new vtable_type()) {
                typename InterfaceDelegate::template set_visitor<T> vi(v.get());
                reflector<interface_type>::visit( *m_vtable, vi );
            }

            any_ptr( const any_ptr& cp )
            :m_ptr(cp.m_ptr),m_vtable(cp.m_vtable)
            {
                
            }

            const vtable_type& operator*()const  { return *m_vtable;  } 
            vtable_type&       operator*()       { return *m_vtable;  } 

            const vtable_type* operator->()const { return m_vtable.get(); } 
            vtable_type*       operator->()      { return m_vtable.get(); } 
             
        protected:
            boost::any                          m_ptr;
            boost::shared_ptr<vtable_type>      m_vtable;
    };

} }

#endif
