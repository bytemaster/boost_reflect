#ifndef BOOST_PP_IS_ITERATING

#   ifndef BOOST_REFLECT_FUNCTION_SIGNATURE_HPP
#       define BOOST_REFLECT_FUNCTION_SIGNATURE_HPP

#       include <boost/preprocessor/repetition.hpp>
#       include <boost/type_traits/remove_const.hpp>
#       include <boost/type_traits/remove_pointer.hpp>
#       include <boost/type_traits/remove_reference.hpp>
#       include <boost/preprocessor/arithmetic/sub.hpp>
#       include <boost/preprocessor/comparison/less.hpp>
#       include <boost/preprocessor/control/expr_if.hpp>
#       include <boost/preprocessor/punctuation/comma_if.hpp>

#        ifndef GET_REFLECT_PARAMETER_SIZE
#           define GET_REFLECT_PARAMETER_SIZE 8
#        endif


namespace boost { namespace reflect {

#ifndef RCR
#define RCR(X) typename boost::remove_const<typename boost::remove_reference<X>::type>::type
#endif
    
        #define ADD_PARAM(z,n,v)    *sig += get_typename<RCR(BOOST_PP_CAT(A,n))>() \
                                            BOOST_PP_EXPR_IF( BOOST_PP_LESS(n,v), + std::string(",") );

#       include <boost/preprocessor/iteration/iterate.hpp>
#       define BOOST_PP_ITERATION_LIMITS (0, GET_REFLECT_PARAMETER_SIZE )
#       define BOOST_PP_FILENAME_1 <boost/reflect/reflect_function_signature.hpp>
#       include BOOST_PP_ITERATE()

        #undef PARAM_TYPE
        #undef PARAM_NAME
        #undef PARAM_ARG
        #undef ADD_PARAM
        #undef RCR
} } // boost::reflect

#   endif // BOOST_REFLECT_FUNCTION_SIGNATURE_HPP

#else // BOOST_PP_IS_ITERATING

#   define n BOOST_PP_ITERATION()

template<typename Rtn BOOST_PP_ENUM_TRAILING_PARAMS(n, typename A)>
struct get_typeinfo<Rtn (BOOST_PP_ENUM_PARAMS(n, A))>
{
    static const char* name()
    {
        static std::string* sig = NULL;
        if( !sig )
        {
            sig = new std::string( get_typename<Rtn>() );
            *sig += '(';
            BOOST_PP_REPEAT( n, ADD_PARAM, BOOST_PP_SUB(n,1) )
            *sig += ')';
        }
        return sig->c_str();
    }
};



#undef n


#endif // BOOST_PP_IS_ITERATING
