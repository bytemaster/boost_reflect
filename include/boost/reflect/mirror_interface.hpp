#ifndef BOOST_PP_IS_ITERATING
  #ifndef BOOST_REFLECT_MIRROR_INTERFACE_HPP
  #define BOOST_REFLECT_MIRROR_INTERFACE_HPP
  #include <boost/reflect/member_from_signature.hpp>
  #include <boost/fusion/container/vector.hpp>
  #include <boost/fusion/container/generation/make_vector.hpp>
  #include <boost/fusion/functional/generation/make_fused_function_object.hpp>
  #include <boost/fusion/include/make_unfused.hpp>
  #include <boost/reflect/void.hpp>
  #include <boost/signals.hpp>
  
  #include <boost/cmt/log/log.hpp>

  namespace boost { namespace reflect {
      
    template<typename MemberPtr>
    struct mirror_member{};

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

    /**
     *  Blocks a signal if it is currently unblocked and 
     *  unblocks it when it goes out of scope if it was blocked
     *  when constructed. 
     */
    struct scoped_block_signal {
      scoped_block_signal( boost::signals::connection& _c )
      :c(_c),unblock(false){ 
        if( c != boost::signals::connection() && !c.blocked() )  {
          unblock = true;
          c.block();
        }
      }
      ~scoped_block_signal() { 
        if( unblock && c != boost::signals::connection() ) 
            c.unblock(); 
      }
      private:
        bool                        unblock;
        boost::signals::connection& c; 
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
  // boost::result_of
  typedef typename adapt_void<R>::result_type                    result_type;
  typedef mirror_member                                          self_type;
  typedef boost::fusion::vector<PARAM_TYPES>                     fused_params;
  typedef boost::function_traits<result_type(PARAM_TYPES)>       traits;
  static const bool                                              is_const = true;
  static const bool                                              is_signal = false;

  typedef typename boost::remove_pointer<result_type(*)(PARAM_TYPES)>::type   signature;

  result_type operator()( PARAM_ARGS )const {
    return m_delegate( boost::ref( boost::fusion::make_vector(PARAM_NAMES) ) );
  }
  result_type operator() ( const fused_params& fp )const {
    return m_delegate( fp );
  }
  mirror_member& operator=( const mirror_member& d )  {
    m_delegate = d.m_delegate;
    return *this;
  }
  template<typename T>
  mirror_member& operator=( const T& d )  {
    m_delegate = adapt_void<R,T>(d);
    return *this;
  }
  template<typename C, typename M>
  void set_delegate(  C*& s, const M& m ) {
    m_delegate = adapt_void<R, boost::function<R(const fused_params&)> >(
                    boost::fusion::make_fused_function_object( 
                                    boost::bind(m,s PARAM_PLACE_HOLDERS ) ));
  }
  private:
    boost::function<result_type(const fused_params&)> m_delegate; 
};

template<typename R, typename Class  BOOST_PP_COMMA_IF(n) PARAM_TYPE_NAMES>
struct mirror_member<R(Class::*)(PARAM_TYPES)> 
{
  typedef typename adapt_void<R>::result_type                result_type;
                                                                                       
  typedef mirror_member                                      self_type;
  typedef boost::fusion::vector<PARAM_TYPES>                 fused_params;
  typedef boost::function_traits<R(PARAM_TYPES)>             traits;
  typedef boost::function<result_type(const fused_params&)>  delegate_type;
  static const bool                                          is_const  = false;
  static const bool                                          is_signal = false;

  // boost::result_of
  typedef typename boost::remove_pointer<result_type(*)(PARAM_TYPES)>::type signature;

  result_type operator()( PARAM_ARGS ) {
    return m_delegate( boost::fusion::make_vector(PARAM_NAMES) );
  }
  result_type operator() ( const fused_params& fp ) {
    return m_delegate( fp );
  }
  template<typename T>
  mirror_member& operator=( const T& d )  {
    m_delegate = adapt_void<R,T>(d);
    return *this;
  }
  template<typename C, typename M>
  void set_delegate(  C*& s, M m ) {
    m_delegate = adapt_void<R, boost::function<R(const fused_params&)> >(
                      boost::fusion::make_fused_function_object( 
                                       boost::bind(m,s PARAM_PLACE_HOLDERS ) ));
  }
  private:
    delegate_type m_delegate; 
};


template<typename R, typename Class  BOOST_PP_COMMA_IF(n) PARAM_TYPE_NAMES>
struct mirror_member< boost::signal<R(PARAM_TYPES)> (Class::*) > 
{
  typedef typename adapt_void<R>::result_type                result_type;
  typedef mirror_member                                      self_type;
  typedef boost::fusion::vector<PARAM_TYPES>                 fused_params;
  typedef boost::function_traits<result_type(PARAM_TYPES)>   traits;
  typedef boost::signal<R(PARAM_TYPES)>                      signal_type;
  static const bool                                          is_const = false;
  static const bool                                          is_signal = true;

  // boost::result_of
  typedef typename boost::remove_pointer<result_type(*)(PARAM_TYPES)>::type   signature;

  result_type operator()( PARAM_ARGS ) {
    return (*this)( boost::fusion::make_vector(PARAM_NAMES) );
  }

  result_type operator() ( const fused_params& fp ) {
    scoped_block_signal block_reverse(m_reverse_con);
    if( int(m_signal.num_slots()) - 1 > 0 )  // do not count our reverse connection
        boost::fusion::make_fused_function_object( boost::ref(m_signal) )(fp);
    return m_delegate( fp );
  }

  // emits locally, but does not forward to delegate
  result_type emit( const fused_params& fp ) {
    scoped_block_signal block_reverse(m_reverse_con);
    return adapt_void<R,boost::function<R(const fused_params&)> >(
            boost::fusion::make_fused_function_object( boost::ref(m_signal) ) )(fp);
  }

  template<typename T>
  mirror_member& operator=( const T& d )  {
    m_delegate = adapt_void<R,T>(d);
    return *this;
  }

  template<typename Functor>
  boost::signals::connection connect( const Functor& f ) {
    boost::signals::connection c = m_signal.connect(adapt_void<R,Functor>(f) );
    if( m_connect_delegate )
        m_connect_delegate(m_signal.num_slots());
    return c;
  }
  void disconnect( const boost::signals::connection& c ) {
    c.disconnect();
    if( m_connect_delegate )
        m_connect_delegate(m_signal.num_slots());
  }

  void set_connect_delegate( const boost::function<void(int)>& f ) {
    m_connect_delegate = f;
  }

  // sets the delegate that will be called when the signal is 'emited'
  template<typename C, typename M>
  void set_delegate(  C*& s, M m ) {
    m_signal.disconnect_all_slots();
    m_reverse_con = 
        (s->*m).connect( boost::fusion::make_unfused( 
                            boost::bind( &mirror_member::emit, this, _1) )  );

    m_delegate = emit_or_throw( s->*m );
  }

  ~mirror_member() {
    m_signal.disconnect_all_slots();
    if( m_reverse_con != boost::signals::connection() )
        m_reverse_con.disconnect();
  }

  private:

    struct emit_or_throw {
        struct no_connected_slots : public std::exception, public boost::exception {
          const char* what()const throw() { return "no connected slots"; }
        };
        emit_or_throw( signal_type& s ):sig(s){}
        result_type operator()( const fused_params& p ) {
          if( int(sig.num_slots()) -1 > 0 ) { // do not count our reverse connection
            return adapt_void<R, boost::function<R(const fused_params&)> >(
                      boost::fusion::make_fused_function_object(boost::ref(sig)))(p);
          }
          BOOST_THROW_EXCEPTION( no_connected_slots() );
        }
        signal_type&            sig;
    };

    boost::function<void(int)>                         m_connect_delegate;
    boost::function<result_type(const fused_params&)>  m_delegate; 
    boost::signals::connection                         m_reverse_con;
    boost::signal<R(PARAM_TYPES)>                      m_signal;
};

#undef n
#undef PARAM_NAMES         
#undef PARAM_PLACE_HOLDERS
#undef PARAM_ARGS        
#undef PARAM_TYPE_NAMES 
#undef PARAM_TYPES     

#endif // BOOST_PP_IS_ITERATING
