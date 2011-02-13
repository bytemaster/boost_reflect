/**
 *  @file idl.hpp
 *
 *  This class defines the macros and types used to implement
 *  an IDL interface.
 *
 */
#ifndef _BOOST_IDL_HPP_
#define _BOOST_IDL_HPP_
#include <boost/preprocessor/list/for_each.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/idl/get_typestring.hpp>
#include <boost/any.hpp>
#include <boost/idl/mirror_interface.hpp>

namespace boost { namespace idl {

    template<typename VisitorType>
    struct visitor 
    {
        template<typename T>
        void start_visit( T& inter )
        {
            visit( inter, *((VisitorType*)this) );
        }
    };

    template<typename T>
    class set_delegate_visitor : public boost::idl::visitor<set_delegate_visitor<T> >
    {
        public:
           set_delegate_visitor( T* self = 0)
           :m_self(self){}

           template<typename InterfaceName, typename M>
           bool accept( M& m, const char* name )
           {
                m.set_delegate( m_self, M::template get_member_on_type<T>() );
                return true;
           }
       private:
           T* m_self;
    };

    template<typename T, typename InterfaceDelegate = boost::idl::mirror_interface>
    class any {};

} } // namespace boost::idl

#define PUBLIC_BASE( r, data, elem )   public boost::idl::any<elem,data>,
#define BASE_INIT( r, data, elem )     elem<InterfaceDelegate>::__idl__init(data);
//#define INIT_MEMBER( r, data, elem )   elem = boost::idl::make_delegate( v, &T::elem );
#define INIT_MEMBER( r, data, elem )   elem.set_delegate( v, &T::elem );
#define DEFINE_MEMBER( r, data, elem ) \
        struct BOOST_PP_CAT( __idl__, elem) : public InterfaceDelegate::template calculate_type<BOOST_TYPEOF(&idl_definition_class::elem)>::type  \
        {  \
            typedef typename InterfaceDelegate::template calculate_type<BOOST_TYPEOF(&idl_definition_class::elem)>::type base_type;  \
            using base_type::operator=;\
            template<typename T> \
            static typename boost::idl::detail::member_from_signature<T,typename base_type::signature,base_type::is_const>::type get_member_on_type() { return &T::elem; }  \
        } elem;

#define VISIT_BASE( r, data, elem ) any<elem,InterfaceDelegate>::__idl__visit(data);
#define VISIT_MEMBER( r, data, elem ) t.template accept<data>( elem, BOOST_PP_STRINGIZE(elem) );

#define BOOST_IDL_BASE BOOST_PP_NIL
#define BOOST_IDL_INTERFACE( NAME, INHERITS, MEMBERS ) \
namespace boost { namespace idl { \
template<typename InterfaceDelegate > \
class any<NAME,InterfaceDelegate> :BOOST_PP_LIST_FOR_EACH( PUBLIC_BASE, InterfaceDelegate, INHERITS )  virtual protected boost::any \
{ \
    public: \
        typedef NAME idl_definition_class; \
        any(){} \
        template<typename T> \
        any( T* v )  \
        :boost::any(v) \
        { \
            boost::idl::set_delegate_visitor<T> sd(v);\
            sd.start_visit(*this);\
        }\
        template<typename T> \
        any( boost::shared_ptr<T> v )  \
        :boost::any(v) \
        { \
            boost::idl::set_delegate_visitor<T> sd(v.get());\
            sd.start_visit(*this);\
        }\
        template<typename T> \
        any( T v )  \
        :boost::any(v) \
        { \
            boost::idl::set_delegate_visitor<T> sd(boost::any_cast<T>( (boost::any*)(this) ) ); \
            sd.start_visit(*this);\
        }\
        BOOST_PP_SEQ_FOR_EACH( DEFINE_MEMBER, NAME, MEMBERS ) \
        \
        template<typename T> \
        any& operator = ( T* v ) { \
            boost::any* a = this; \
            *a = v; \
            boost::idl::set_delegate_visitor<T> sd(v);\
            sd.start_visit(*this);\
            return *this; \
        } \
\
        template<typename Vi> \
        friend inline void visit( any& i, Vi& v ) \
        { \
            i.__idl__visit(v); \
        } \
    protected: \
\
        template<typename T> \
        void __idl__visit( T& t ) \
        { \
            BOOST_PP_LIST_FOR_EACH( VISIT_BASE, t, INHERITS ) \
            BOOST_PP_SEQ_FOR_EACH( VISIT_MEMBER, NAME, MEMBERS ) \
        } \
}; \
} } \
BOOST_IDL_TYPESTRING(NAME) 

#endif
