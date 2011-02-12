#ifndef BOOST_PP_IS_ITERATING
    //#ifndef _BOOST_IDL_METHOD_HPP_
    //#define _BOOST_IDL_METHOD_HPP_
    #include <boost/idl/fast_delegate.hpp>
    #include <boost/typeof/typeof.hpp>
    #include <boost/function.hpp>
    #include <boost/bind.hpp>
    #include <boost/type_traits/function_traits.hpp>
    #include <boost/preprocessor/repetition.hpp>
    #include <boost/preprocessor/seq/for_each.hpp>
    #include <boost/type_traits/remove_pointer.hpp>

    #define PARAM_NAME(z,n,type)    BOOST_PP_CAT(a,n)
    #define PARAM_TYPE(z,n,type)    BOOST_PP_CAT(BOOST_PP_CAT(typename traits::arg,BOOST_PP_ADD(n,1)),_type)
    #define PARAM_ARG(z,n,type)     PARAM_TYPE(z,n,type) PARAM_NAME(z,n,type)

    namespace boost { namespace idl { namespace detail {

        template<typename Class, typename Signature, bool is_const = false>
        struct member_from_signature 
        {
            typedef Signature Class::*  type;
        /*BOOST_STATIC_ASSERT(false);*/ 
        };
        template<typename T>
        struct get_function_type { typedef T type; };

        template<int arity, typename Signature, bool IsConst = false>
        struct method
        {
            /*
            typename boost::function_traits<Signature>::result_type operator()(); 
            method operator=( const boost::function<Signature>& s );
            private:
                boost::function<Signature> m_delegate;
                */
        };
        namespace detail {
        template<typename C, typename M>
            static M& do_get( C* c, M C::* m )
            { return c->*m; }
        }

        template<typename Class, typename MemberType>
        struct member
        {
            typedef MemberType signature;
            static const bool is_const = false;

            operator MemberType& ()
            {
                return get();
            }
            operator const MemberType& ()const
            {
                return get();
            }
            MemberType& operator = ( const MemberType& a )
            {
                return get() = a;
            }
            const MemberType& operator * () const
            {
                return get();
            }
            MemberType& operator * ()
            {
                return get();
            }

            const MemberType& operator -> () const
            {
                return get();
            }

            MemberType& operator -> () 
            {
                return get();
            }

            template<typename C>
            void set_delegate(  C*& s, MemberType C::* m )
            {
                MemberType& (*do_get_impl)(C*, MemberType C::* ) = &detail::do_get;
                get = boost::bind( do_get_impl, s, m );
            }

            typedef member self_type;
            member& operator = ( const boost::function<MemberType&()> a ) { get = a; return *this; }
            boost::function<MemberType&()>                  get;
        };

        template<typename T>
        struct calculate_type
        {
        };

        template<typename Class, typename MemberType>
        struct calculate_type<MemberType (Class::*)>
        {
            typedef  boost::idl::detail::member<Class,MemberType> type;
        };

        template<typename T>
        boost::false_type implements_method( T );

#        ifndef BOOST_IDL_METHOD_IMPL_SIZE
#           define BOOST_IDL_METHOD_IMPL_SIZE 8
#        endif

#       include <boost/preprocessor/iteration/iterate.hpp>
#       define BOOST_PP_ITERATION_LIMITS (0, BOOST_IDL_METHOD_IMPL_SIZE -1 )
#       define BOOST_PP_FILENAME_1 <boost/idl/method.hpp>
#       include BOOST_PP_ITERATE()

    #undef PARAM_NAME
    #undef PARAM_TYPE
    #undef PARAM_ARG

    } } } // namespace boost::idl::detail

    //#endif // _BOOST_IDL_METHOD_HPP_
#else // BOOST_PP_IS_ITERATING

#define n BOOST_PP_ITERATION()
#define PARAM_NAMES     BOOST_PP_ENUM(n,PARAM_NAME,A) // name_N
#define PARAM_ARGS      BOOST_PP_ENUM(n,PARAM_ARG,A) // TYPE_N name_N
        template<typename C, typename R BOOST_PP_ENUM_TRAILING_PARAMS(n, typename A)>
        struct member_from_signature<C, R(BOOST_PP_ENUM_PARAMS(n,A)), false> 
        { 
            typedef typename boost::remove_pointer<R(C::*)(BOOST_PP_ENUM_PARAMS(n,A))>::type type; 
            static const int  arity    = n;
            static const bool is_const = false;
        };
        template<typename C, typename R BOOST_PP_ENUM_TRAILING_PARAMS(n, typename A)>
        struct member_from_signature<C, R(BOOST_PP_ENUM_PARAMS(n,A)), true> 
        { 
            typedef typename boost::remove_pointer<R(C::*)(BOOST_PP_ENUM_PARAMS(n,A))const>::type type; 
            static const int  arity    = n;
            static const bool is_const = true;
        };

        template<typename R, typename C BOOST_PP_ENUM_TRAILING_PARAMS(n, typename A)>
        struct calculate_type<R(C::*)(BOOST_PP_ENUM_PARAMS(n,A))> 
        { 
            typedef boost::idl::fast_delegate<R(BOOST_PP_ENUM_PARAMS(n,A))>                    traits;
            typedef typename boost::remove_pointer<R(C::*)(BOOST_PP_ENUM_PARAMS(n,A))>::type member_type; 
            typedef  boost::idl::detail::method< n,
                                                 R( BOOST_PP_ENUM_PARAMS(n,A) ),
                                                 false >  type;
        };

        template<typename R, typename C BOOST_PP_ENUM_TRAILING_PARAMS(n, typename A)>
        struct get_function_type<R(C::*)(BOOST_PP_ENUM_PARAMS(n,A))> 
        { 
            typedef typename boost::remove_pointer<R(C::*)(BOOST_PP_ENUM_PARAMS(n,A))>::type type; 
            static const int  arity = boost::function_traits<R(BOOST_PP_ENUM_PARAMS(n,A))>::arity;
            static const bool is_const = false;
        };
        template<typename R, typename C BOOST_PP_ENUM_TRAILING_PARAMS(n, typename A)>
        struct get_function_type<R(C::*)(BOOST_PP_ENUM_PARAMS(n,A))const> 
        { 
            typedef typename boost::remove_pointer<R(C::*)(BOOST_PP_ENUM_PARAMS(n,A))const>::type type; 
            static const int  arity = boost::function_traits<R(BOOST_PP_ENUM_PARAMS(n,A))>::arity;
            static const bool is_const = true;
        };

        template<typename R, typename C BOOST_PP_ENUM_TRAILING_PARAMS(n, typename A)>
        struct calculate_type<R(C::*)(BOOST_PP_ENUM_PARAMS(n,A))const> 
        { 
            typedef boost::idl::fast_delegate<R(BOOST_PP_ENUM_PARAMS(n,A))>                    traits;
            typedef typename boost::remove_pointer<R(C::*)(BOOST_PP_ENUM_PARAMS(n,A))const>::type member_type; 
            typedef  boost::idl::detail::method< n,
                                                 R( BOOST_PP_ENUM_PARAMS(n,A) ),
                                                 true >  type;
        };


        template<typename Signature>
        struct method<n,Signature,false>
        {
            typedef Signature signature;
            typedef boost::function_traits<Signature> traits;
            typedef method<n,Signature,false> self_type;
            static const bool is_const = false;

            inline typename traits::result_type 
                operator()( PARAM_ARGS ) { return m_delegate( PARAM_NAMES ); }
            method& operator=( const boost::idl::fast_delegate<Signature>& s )
            { m_delegate = s; return *this; }
 //           private:
 
            
            template<typename C, typename M>
            void set_delegate(  C*& s, M m )
            {
                m_delegate = make_delegate(s,m); 
            }
            boost::idl::fast_delegate<Signature> m_delegate;
        };

        template<typename Signature>
        struct method<n,Signature,true>
        {
            typedef Signature signature;
            typedef boost::function_traits<Signature> traits;
            typedef method<n,Signature,true> self_type;
            static const bool is_const = true;

            inline typename traits::result_type 
                operator()( PARAM_ARGS )const { return m_delegate( PARAM_NAMES ); }

            method& operator=( const boost::idl::fast_delegate<Signature>& s )
            { m_delegate = s; return *this; }

            template<typename C, typename M>
            void set_delegate(  C*& s, M m )
            {
                m_delegate = make_delegate(s,m); 
            }
//            private:
            boost::idl::fast_delegate<Signature> m_delegate;
        };


#undef PARAM_ARGS
#undef PARAM_NAMES
#undef n

#endif // BOOST_PP_IS_ITERATING
