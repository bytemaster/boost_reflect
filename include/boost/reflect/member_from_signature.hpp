/**
 *  @file member_from_signature.hpp
 *
 *  This file provides templates to convert between member function pointers and 
 *  as well as creating a member function signature given a type and signature.
 *
 */
#ifndef BOOST_PP_IS_ITERATING
    #ifndef _BOOST_REFLECT_MEMBER_FROM_SIGNATURE_HPP_
    #define _BOOST_REFLECT_MEMBER_FROM_SIGNATURE_HPP_
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

    namespace boost { namespace reflect { namespace detail {

        template<typename Class, typename Signature, bool is_const = false>
        struct member_from_signature 
        {
            typedef Signature Class::*  type;
        };
        template<typename T>
        struct get_member_type { typedef T type; };

        template<typename Class, typename MemberType>
        struct get_member_type<MemberType (Class::*)> 
        { 
            typedef MemberType  type; 
            static const int  arity      = 0;
            static const bool is_const   = false;
        };

#        ifndef BOOST_REFLECT_MEMBER_FROM_SIGNATURE_IMPL_SIZE
#           define BOOST_REFLECT_MEMBER_FROM_SIGNATURE_IMPL_SIZE 8
#        endif

#       include <boost/preprocessor/iteration/iterate.hpp>
#       define BOOST_PP_ITERATION_LIMITS (0, BOOST_REFLECT_MEMBER_FROM_SIGNATURE_IMPL_SIZE -1 )
#       define BOOST_PP_FILENAME_1 <boost/reflect/member_from_signature.hpp>
#       include BOOST_PP_ITERATE()

    #undef PARAM_NAME
    #undef PARAM_TYPE
    #undef PARAM_ARG

    } } } // namespace boost::reflect::detail

    #endif // _BOOST_REFLECT_MEMBER_FROM_SIGNATURE_HPP_
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
        struct get_member_type<R(C::*)(BOOST_PP_ENUM_PARAMS(n,A))> 
        { 
            typedef typename boost::remove_pointer<R(C::*)(BOOST_PP_ENUM_PARAMS(n,A))>::type type; 
            static const int  arity = boost::function_traits<R(BOOST_PP_ENUM_PARAMS(n,A))>::arity;
            static const bool is_const = false;
        };
        template<typename R, typename C BOOST_PP_ENUM_TRAILING_PARAMS(n, typename A)>
        struct get_member_type<R(C::*)(BOOST_PP_ENUM_PARAMS(n,A))const> 
        { 
            typedef typename boost::remove_pointer<R(C::*)(BOOST_PP_ENUM_PARAMS(n,A))const>::type type; 
            static const int  arity = boost::function_traits<R(BOOST_PP_ENUM_PARAMS(n,A))>::arity;
            static const bool is_const = true;
        };


#undef PARAM_ARGS
#undef PARAM_NAMES
#undef n

#endif // BOOST_PP_IS_ITERATING
