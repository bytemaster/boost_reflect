/**
 *  @file reflect.hpp
 *
 *  This class defines the macros and types used to implement
 *  an REFLECT interface.
 *
 */
#ifndef _BOOST_REFLECT_ANY_HPP_
#define _BOOST_REFLECT_ANY_HPP_
#include <boost/preprocessor/list/for_each.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/reflect/reflect.hpp>
#include <boost/any.hpp>
#include <boost/reflect/mirror_interface.hpp>

namespace boost { namespace reflect {

    template<typename Any, typename Vi> 
    inline void visit( Any& i, Vi& v ) 
    { 
        i.__reflect__visit(v); 
    } 

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
    class set_delegate_visitor : public boost::reflect::visitor<set_delegate_visitor<T> >
    {
        public:
           set_delegate_visitor( T* self = 0)
           :m_self(self){}

           template<typename InterfaceName, typename M>
           bool accept( M& m, const char* name ) {
                m.set_delegate( m_self, M::template get_member_on_type<T>() );
                return true;
           }
       private:
           T* m_self;
    };
    template<typename T>
    class set_forward_delegate_visitor : public boost::reflect::visitor<set_forward_delegate_visitor<T> >
    {
        public:
           set_forward_delegate_visitor( T* self = 0)
           :m_self(self){}

           template<typename InterfaceName, typename M>
           bool accept( M& m, const char* name ) {
                m.get_member_on_type( m_self, m.m_delegate );
                return true;
           }
       private:
           T* m_self;
    };

    template<typename T, typename InterfaceDelegate = boost::reflect::mirror_interface>
    class any {};

    class any_holder_base {
        virtual ~any_holder_base(){};
        virtual any_holder_base* clone()const = 0;
        virtual void apply( dynamic_any& da );
    };

    template<typename Interface, typename T>
    class any_holder_impl {
        public:
           any_holder_impl( const T& v )
           :m_value(v){}

           any_holder_base* clone() const { 
                return new any_holder_impl(*this); 
           } 
            
           void apply( dynamic_any& i ) {
              any_holder_impl 
           }
        private:
            T m_value;
    };

    class any_holder {
        public:
            
        
    };



} } // namespace boost::reflect

#define PUBLIC_BASE( r, data, elem )   public boost::reflect::any<elem,data>,
#define BASE_INIT( r, data, elem )     elem<InterfaceDelegate>::__reflect__init(data);
#define INIT_MEMBER( r, data, elem )   elem.set_delegate( v, &T::elem );
#define DEFINE_MEMBER( r, data, elem ) \
        struct BOOST_PP_CAT( __reflect__, elem) : public InterfaceDelegate::template calculate_type<BOOST_TYPEOF(&reflect_definition_class::elem)>::type  \
        {  \
            typedef typename InterfaceDelegate::template calculate_type<BOOST_TYPEOF(&reflect_definition_class::elem)>::type base_type;  \
            using base_type::operator=;\
            template<typename Type, typename AssignType> \
            static void get_member_on_type( Type* t, AssignType& v ) {  v = t->elem; } \
            template<typename T> \
            static typename boost::reflect::detail::member_from_signature<T,typename base_type::signature,base_type::is_const>::type get_member_on_type()\
                { return &T::elem; }  \
        } elem;

#define VISIT_BASE( r, data, elem ) any<elem,InterfaceDelegate>::__reflect__visit(data);
#define VISIT_MEMBER( r, data, elem ) t.template accept<data>( elem, BOOST_PP_STRINGIZE(elem) );

#define BOOST_REFLECT_BASE BOOST_PP_NIL
#define BOOST_REFLECT_ANY( NAME, INHERITS, MEMBERS ) \
BOOST_REFLECT( NAME, INHERITS, MEMBERS ) \
namespace boost { namespace reflect { \
template<typename InterfaceDelegate > \
class any<NAME,InterfaceDelegate> :BOOST_PP_SEQ_FOR_EACH( PUBLIC_BASE, InterfaceDelegate, INHERITS )  virtual protected boost::any \
{ \
    public: \
        typedef NAME reflect_definition_class; \
        any(){} \
        any( const any& c ) { \
            *this = c; \
        }\
        template<typename T> \
        any( T* v )  \
        :boost::any(v) \
        { \
            boost::reflect::set_delegate_visitor<T> sd(v);\
            sd.start_visit(*this);\
        }\
        template<typename T> \
        any( const boost::shared_ptr<T>& v )  \
        :boost::any(v) \
        { \
            boost::reflect::set_delegate_visitor<T> sd(v.get());\
            sd.start_visit(*this);\
        }\
        template<typename T> \
        any( T v )  \
        :boost::any(v) \
        { \
            boost::reflect::set_delegate_visitor<T> sd(boost::any_cast<T>( (boost::any*)(this) ) ); \
            sd.start_visit(*this);\
        }\
        BOOST_PP_SEQ_FOR_EACH( DEFINE_MEMBER, NAME, MEMBERS ) \
        \
        template<typename T> \
        any& operator = ( T* v ) { \
            boost::any* a = this; \
            *a = v; \
            boost::reflect::set_delegate_visitor<T> sd(v);\
            sd.start_visit(*this);\
            return *this; \
        } \
\
        template<typename Any, typename Vi> \
        friend inline void visit( Any& i, Vi& v ); \
\
    protected: \
\
        template<typename T> \
        void __reflect__visit( T& t ) \
        { \
            BOOST_PP_SEQ_FOR_EACH( VISIT_BASE, t, INHERITS ) \
            BOOST_PP_SEQ_FOR_EACH( VISIT_MEMBER, NAME, MEMBERS ) \
        } \
}; \
} } 


#endif
