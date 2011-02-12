#ifndef BOOST_PP_IS_ITERATING

#   ifndef BOOST_IDL_GET_FUNCTION_TYPE_HPP_INCLUDED
#       define BOOST_IDL_GET_FUNCTION_TYPE_HPP_INCLUDED


#       include <boost/typeof/typeof.hpp>
#       include <boost/preprocessor/repetition.hpp>
#       include <boost/preprocessor/arithmetic/sub.hpp>
#       include <boost/preprocessor/punctuation/comma_if.hpp>
        #include <boost/type_traits/remove_pointer.hpp>
        #include <boost/type_traits/function_traits.hpp>

#        ifndef GET_FUNCTION_TYPE_SIZE
#           define GET_FUNCTION_TYPE_SIZE 8
#        endif

namespace boost { namespace idl {

#          define TN(X,Y) typename idl::get_function_type<BOOST_TYPEOF(&X::Y)>::type
#          define SARITY(X,Y) idl::get_function_type<BOOST_TYPEOF(&X::Y)>::arity

/**
 *  @class get_function_type<T>
 *  @brief Used to deduce the function signature of signals/member function pointers.
 *
 *  Given a class name and function or signal name, the combination of BOOST_TYPEOF() and get_function_type
 *  will return a type of the form R(ARGS) which can then be used in other templates.
 *
 */
            template<typename T>
            struct get_function_type { typedef T type; };

            template<typename Class, typename Signature, bool is_const = false>
            struct member_from_signature { /*BOOST_STATIC_ASSERT(false);*/ };

#       include <boost/preprocessor/iteration/iterate.hpp>
#       define BOOST_PP_ITERATION_LIMITS (0, GET_FUNCTION_TYPE_SIZE -1 )
#       define BOOST_PP_FILENAME_1 <boost/idl/get_function_type.hpp>
#       include BOOST_PP_ITERATE()


} } // namespace boost::idl

#   endif // BOOST_IDL_GET_FUNCTION_TYPE_HPP_INCLUDED

#else // BOOST_PP_IS_ITERATING

#   define n BOOST_PP_ITERATION()



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


#undef n

#endif // BOOST_PP_IS_ITERATING

