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
        void start( T& inter )
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

} } // namespace boost::idl

#ifdef DOXYGEN
    
    /**
     *  This is an example of what the BOOST_IDL_INTERFACE( ) macro will generate.  
     *
     *  InterfaceDelegate is a class that defines how the method function objects are
     *  derived from the interface definition.  For example, an interface might transform 
     *  synchronous return values into future<return_type>.  Other delegates may offer
     *  other operations on a per-method basis such as specifying parameters to an RPC
     *  layer.
     *
     *
     */
    template<typename InterfaceDelegate = boost::idl::mirror_interface >
    class InterfaceName : public BaseInterface<InterfaceDelegate>, ... 
    {
        public:
        struct __idl__membername : public typename InterfaceDelegate::calculate_type<&idl_definition::InterfaceName::membername>::type
        {
            template<typename T> 
            static typename boost::idl::member_from_signature<&T::membername>::type get_member_on_type() {
               return &T::membername; 
            }
        } membername;

        /**
         *  This is a free function that is given friend access to start visitors.  The purpose of this
         *  function is to prevent 'poluting' the interface with idl methods.  
         */
        template<typename Vi> 
        friend inline void visit( NAME& i, Vi& v ) 
        { 
            i.__idl__visit(v); 
        } 

        protected;
        /**
         *  This method will pass the visitor over all members.
         */
        template<typename T> 
        void __idl__visit( T& t ) 
        { 
            BOOST_PP_LIST_FOR_EACH( VISIT_BASE, t, INHERITS ) 
            BOOST_PP_SEQ_FOR_EACH( VISIT_MEMBER, NAME, MEMBERS ) 
        } 
    };

#endif // DOXYGEN


            //static BOOST_TYPEOF(&T::area) get_member_on_type() { return &T::elem; }  \

#define PUBLIC_BASE( r, data, elem )   public elem<data>,
#define BASE_INIT( r, data, elem )     elem<InterfaceDelegate>::__idl__init(data);
//#define INIT_MEMBER( r, data, elem )   elem = boost::idl::make_delegate( v, &T::elem );
#define INIT_MEMBER( r, data, elem )   elem.set_delegate( v, &T::elem );
#define DEFINE_MEMBER( r, data, elem ) \
        struct BOOST_PP_CAT( __idl__, elem) : public InterfaceDelegate::template calculate_type<BOOST_TYPEOF(&idl_definition::data::elem)>::type  \
        {  \
            typedef typename InterfaceDelegate::template calculate_type<BOOST_TYPEOF(&idl_definition::data::elem)>::type base_type;  \
            using base_type::operator=;\
            template<typename T> \
            static typename boost::idl::detail::member_from_signature<T,typename base_type::signature,base_type::is_const>::type get_member_on_type() { return &T::elem; }  \
        } elem;

#define VISIT_BASE( r, data, elem ) elem<InterfaceDelegate>::__idl__visit(data);
#define VISIT_MEMBER( r, data, elem ) t.template accept<data>( elem, BOOST_PP_STRINGIZE(elem) );

#define BOOST_IDL_BASE BOOST_PP_NIL
#define BOOST_IDL_INTERFACE( NAME, INHERITS, MEMBERS ) \
template<typename InterfaceDelegate = boost::idl::mirror_interface > \
class NAME :BOOST_PP_LIST_FOR_EACH( PUBLIC_BASE, InterfaceDelegate, INHERITS )  virtual protected boost::any \
{ \
    public: \
        NAME(){} \
        template<typename T> \
        NAME( T* v )  \
        :boost::any(v) \
        { \
            boost::idl::set_delegate_visitor<T> sd(v);\
            sd.start(*this);\
        }\
        template<typename T> \
        NAME( boost::shared_ptr<T> v )  \
        :boost::any(v) \
        { \
            boost::idl::set_delegate_visitor<T> sd(v.get());\
            sd.start(*this);\
        }\
        template<typename T> \
        NAME( const T& v )  \
        :boost::any(v) \
        { \
            boost::idl::set_delegate_visitor<T> sd(boost::any_cast<T>( (boost::any*)(this) ) ); \
            sd.start(*this);\
        }\
        BOOST_PP_SEQ_FOR_EACH( DEFINE_MEMBER, NAME, MEMBERS ) \
        \
        template<typename T> \
        NAME& operator = ( T* v ) { \
            boost::any* a = this; \
            *a = v; \
            boost::idl::set_delegate_visitor<T> sd(v);\
            sd.start(*this);\
            return *this; \
        } \
\
        template<typename Vi> \
        friend inline void visit( NAME& i, Vi& v ) \
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
BOOST_IDL_TYPESTRING_ALIAS(BOOST_STRINGIZE(NAME), NAME<>) 

#endif
