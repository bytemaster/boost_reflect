#ifndef _BOOST_IDL_CLI_HPP_
#define _BOOST_IDL_CLI_HPP_
#include <sstream>
#include <iomanip>
#include <iostream>
#include <boost/fusion/sequence/io.hpp>
#include <boost/reflect/reflect.hpp>
#include <boost/utility/result_of.hpp>
#include <boost/rpc/json/value_io.hpp>

inline std::string cin_getline() {
    std::string s;
    std::getline( std::cin, s );
    return s;
}

/**
 *  Takes any interface object and provides a command line interface for it.
 */
class cli {
    public:
       template<typename AnyPtr>
       cli( AnyPtr aptr):my_ptr(aptr) { 
          // AnyPtr will create a copy which will go out of scope unless we
          // are smart about keeping a reference to it.
          boost::reflect::visit( aptr, visitor<typename AnyPtr::vtable_type>( *this, **boost::any_cast<AnyPtr>(&my_ptr)) ); 
       }
       ~cli() {
        try {
          read_done.wait();
        } catch ( ... ) {
        }
       }

       boost::function<std::string(const std::string&)>& operator[]( const std::string& name ) 
       { return methods[name]; }

       void start() {  
          read_done = boost::cmt::async<void>( boost::bind( &cli::read_loop, this ) );
       }

   private:
       boost::any my_ptr;

       boost::cmt::future<void> read_done;
       void read_loop() {
           boost::cmt::thread* getline_thread = boost::cmt::thread::create();
           while( true )
           {
               std::string cmd, line, args;
               std::cerr << "Enter Method: ";
               line = getline_thread->sync<std::string>( cin_getline );
               cmd = line.substr( 0, line.find(' ') );
               args = line.substr( cmd.size(), line.size() );
               try {
               std::cerr << methods[cmd](args) << std::endl;
               } catch ( const std::exception& e ) {
               std::cerr << e.what() << std::endl;
               }
           }
       }

       template<typename VTableType> struct visitor {
           visitor( cli& c, VTableType& vtbl ):m_cli(c),m_vtbl(vtbl){}
           /*
           template<typename M, typename InterfaceName, M InterfaceName::*m>
           void operator()( const char* name ) const {
                std::cerr << std::setw(10) << std::setiosflags(std::ios::left) << std::string("'")+name+std::string("'")
                          << "         " << boost::reflect::get_typename<typename M::signature>()
                          << (M::is_const ? "const" : "") <<std::endl;
                m_cli.methods[name] = cli_functor<typename M::fused_params, M&>(m_vtbl.*m);
           }
           */
           template<typename MemberPtr, MemberPtr m>
           void operator()( const char* name ) const {
                typedef typename boost::function_types::result_type<MemberPtr>::type member_ref;
                typedef typename boost::remove_reference<member_ref>::type member;
                m_cli.methods[name] = cli_functor<typename member::fused_params,member_ref>(m_vtbl.*m);
            /*
                typedef typename boost::remove_reference<typename boost::function_types::result_type<M>::type>::type R;
                std::cerr << std::setw(10) << std::setiosflags(std::ios::left) << std::string("'")+name+std::string("'")
                          << "         " << boost::reflect::get_typename<typename R::signature>()
                          << (R::is_const ? "const" : "") <<std::endl;
                m_cli.methods[name] = cli_functor<typename R::fused_params, R&>(m_vtbl.*m);
                */
           }
           VTableType&   m_vtbl;
           cli&          m_cli;
       };

       template<typename Seq, typename Functor>
       struct cli_functor {
           cli_functor( Functor f )
           :m_func(f){}

           typedef typename boost::remove_reference<Functor>::type functor_type;

           template<typename T>
           const T& wait_future( const boost::cmt::future<T>& f ) { return f.wait(); }
           template<typename T>
           const T& wait_future( const T& f ) { return f; }

           std::string operator()( const std::string& cli )
           {
              typedef typename boost::fusion::traits::deduce_sequence<Seq>::type param_type;
              boost::rpc::json::value v;
              boost::rpc::json::from_string(cli, v);
              param_type args;
              boost::rpc::json::io::unpack( v, args );
              boost::rpc::json::value r;
              boost::rpc::json::io::pack( r, wait_future(m_func(args)) );
              return r;
           }
           Functor m_func;
       };
       std::map<std::string, boost::function<std::string(const std::string&)> > methods;
};


#endif
