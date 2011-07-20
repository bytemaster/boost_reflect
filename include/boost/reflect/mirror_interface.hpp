#ifndef BOOST_PP_IS_ITERATING
    #ifndef BOOST_REFLECT_MIRROR_INTERFACE_HPP
    #define BOOST_REFLECT_MIRROR_INTERFACE_HPP
    #include <boost/reflect/member_from_signature.hpp>
    #include <boost/fusion/container/vector.hpp>
    #include <boost/fusion/container/generation/make_vector.hpp>
    #include <boost/fusion/functional/generation/make_fused_function_object.hpp>

    namespace boost { namespace reflect {
        
        template<typename MemberPtr>
        struct mirror_member
        {
        };

        namespace detail {
        template<typename C, typename M>
            static M& do_get( C* c, M C::* m )
            { return c->*m; }
        }
        
        struct mirror_interface 
        {
            template<typename MemberPointer>
            struct calculate_type {
                typedef mirror_member<MemberPointer>  type; 
            };

            template<typename T>
            class set_visitor {
                public:
                   set_visitor( T* self = 0)
                   :m_self(self){}

                   template<typename InterfaceName, typename M>
                   bool accept( M& m, const char* name ) {
                        assign<M> a(m_self,m);
                        M::template get_member_ptr<T>( a );
                        return true;
                   }
               private:
                   template<typename Member>
                   struct assign {
                        assign( T* _v, Member& _m ):v(_v),m(_m){}

                        template<typename MemberPtr>
                        void operator=( const MemberPtr& p ) {
                            m.set_delegate( v, p );
                        }
                        private:
                            T*      v;
                            Member& m;
                   };
                   T* m_self;
            };

        };


    #define PARAM_NAME(z,n,type)         BOOST_PP_CAT(a,n)
    #define PARAM_PLACE_HOLDER(z,n,type) BOOST_PP_CAT(_,BOOST_PP_ADD(n,1) )
    #define PARAM_TYPE_NAME(z,n,type)   BOOST_PP_CAT(typename A,n)
    #define PARAM_TYPE(z,n,type)   BOOST_PP_CAT(A,n)
    #define PARAM_ARG(z,n,type)     PARAM_TYPE(z,n,type) PARAM_NAME(z,n,type)

#        ifndef BOOST_REFLECT_MIRROR_IMPL_SIZE
#           define BOOST_REFLECT_MIRROR_IMPL_SIZE 8
#        endif

#       include <boost/preprocessor/iteration/iterate.hpp>
#       define BOOST_PP_ITERATION_LIMITS (0, BOOST_REFLECT_MIRROR_IMPL_SIZE -1 )
#       define BOOST_PP_FILENAME_1 <boost/reflect/mirror_interface.hpp>
#       include BOOST_PP_ITERATE()

    #undef PARAM_NAME
    #undef PARAM_TYPE
    #undef PARAM_ARG

    } } // namespace boost::reflect
    #endif // BOOST_REFLECT_MIRROR_INTERFACE_HPP

#else // BOOST_PP_IS_ITERATING

#define n BOOST_PP_ITERATION()
#define PARAM_NAMES          BOOST_PP_ENUM(n,PARAM_NAME,A) // name_N
#define PARAM_PLACE_HOLDERS  BOOST_PP_ENUM_TRAILING(n,PARAM_PLACE_HOLDER,A) // _(N+1)
#define PARAM_ARGS           BOOST_PP_ENUM(n,PARAM_ARG,A) // TYPE_N name_N
#define PARAM_TYPE_NAMES     BOOST_PP_ENUM(n,PARAM_TYPE_NAME,A) // typename TYPE_N
#define PARAM_TYPES          BOOST_PP_ENUM(n,PARAM_TYPE,A) // TYPE_N

    template<typename R, typename Class BOOST_PP_COMMA_IF(n) PARAM_TYPE_NAMES>
    struct mirror_member<R(Class::*)(PARAM_TYPES)const> 
    {
        typedef mirror_member                                             self_type;
        typedef boost::fusion::vector<PARAM_TYPES>                        fused_params;
        typedef boost::function_traits<R(PARAM_TYPES)>                    traits;
        typedef typename boost::remove_pointer<R(*)(PARAM_TYPES)>::type   signature;
        // boost::result_of
        typedef R                                                         result_type;
        static const bool                                                 is_const = true;
        R operator()( PARAM_ARGS )const {
            return m_delegate( boost::fusion::make_vector(PARAM_NAMES) );
        }
        R operator() ( const fused_params& fp )const {
            return m_delegate( fp );
        }
        mirror_member& operator=( const mirror_member& d )  {
            m_delegate = d.m_delegate;
            return *this;
        }
        template<typename T>
        mirror_member& operator=( const T& d )  {
            m_delegate = d;
            return *this;
        }
        template<typename C, typename M>
        void set_delegate(  C*& s, const M& m ) {
            m_delegate = boost::fusion::make_fused_function_object( boost::bind(m,s PARAM_PLACE_HOLDERS) );  
        }
        boost::function<R(const fused_params&)> m_delegate; 
    };

    template<typename R, typename Class  BOOST_PP_COMMA_IF(n) PARAM_TYPE_NAMES>
    struct mirror_member<R(Class::*)(PARAM_TYPES)> 
    {
        typedef mirror_member                                             self_type;
        typedef boost::fusion::vector<PARAM_TYPES>                        fused_params;
        typedef boost::function_traits<R(PARAM_TYPES)>                    traits;
        // boost::result_of
        typedef R                                                         result_type;
        typedef typename boost::remove_pointer<R(*)(PARAM_TYPES)>::type   signature;
        static const bool                                                 is_const = false;
        R operator()( PARAM_ARGS ) {
            return m_delegate( boost::fusion::make_vector(PARAM_NAMES) );
        }
        R operator() ( const fused_params& fp ) {
            return m_delegate( fp );
        }
        mirror_member& operator=( const  boost::function<R(PARAM_TYPES)>& d )  {
            m_delegate = d;
            return *this;
        }
        template<typename C, typename M>
        void set_delegate(  C*& s, M m ) {
            m_delegate = boost::fusion::make_fused_function_object( boost::bind(m,s PARAM_PLACE_HOLDERS) ); //make_fast_delegate(s,m) ); 
        }
        boost::function<R(const fused_params&)> m_delegate; 
    };


#undef n
#undef PARAM_NAMES         
#undef PARAM_PLACE_HOLDERS
#undef PARAM_ARGS        
#undef PARAM_TYPE_NAMES 
#undef PARAM_TYPES     

#endif // BOOST_PP_IS_ITERATING
