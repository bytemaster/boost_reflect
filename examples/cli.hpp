#ifndef _BOOST_IDL_CLI_HPP_
#define _BOOST_IDL_CLI_HPP_
#include <sstream>
#include <iomanip>
#include <iostream>

#include <boost/fusion/sequence/io.hpp>
#include <boost/reflect/mirror_interface.hpp>
#include <boost/reflect/any.hpp>

/**
 *  Takes any interface object and provides a command line interface for it.
 */
class cli : public boost::reflect::visitor< cli >
{
    public:
       template<typename InterfaceName, typename M>
       bool accept( M& m, const char* name )
       {
            std::cerr << std::setw(10) << std::setiosflags(std::ios::left) << name 
                      << " " << boost::reflect::get_typename<typename M::signature>()
                      << (M::is_const ? "const" : "") <<std::endl;
            methods[name] = cli_functor<typename M::fused_params, M&>(m);
            return true;
       }
       boost::function<std::string(const std::string&)>& operator[]( const std::string& name ) 
       { return methods[name]; }

   private:
       template<typename Seq, typename Functor>
       struct cli_functor
       {
           cli_functor( Functor f )
           :m_func(f){}

           std::string operator()( const std::string& cli )
           {
                std::stringstream ss(cli);
                Seq s; ss >> boost::fusion::tuple_delimiter(',') >> s;
                std::stringstream rtn;
                rtn << m_func( s );
                return rtn.str();
           }
           Functor m_func;
       };
       std::map<std::string, boost::function<std::string(const std::string&)> > methods;
};


#endif
