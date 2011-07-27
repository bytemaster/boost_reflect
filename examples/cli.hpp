#ifndef _BOOST_IDL_CLI_HPP_
#define _BOOST_IDL_CLI_HPP_
#include <sstream>
#include <iomanip>
#include <iostream>
#include <boost/fusion/sequence/io.hpp>
#include <boost/reflect/reflect.hpp>
#include <boost/utility/result_of.hpp>

/**
 *  Takes any interface object and provides a command line interface for it.
 */
class cli 
{
    public:
       template<typename T>
       cli( T aptr) { 
            boost::reflect::visit( aptr, visitor<typename T::vtable_type>( *this, *aptr) ); 
       }

       boost::function<std::string(const std::string&)>& operator[]( const std::string& name ) 
       { return methods[name]; }

   private:
       template<typename VTableType> struct visitor {
           visitor( cli& c, VTableType& vtbl ):m_cli(c),m_vtbl(vtbl){}
           template<typename M, typename InterfaceName>
           void operator()( M InterfaceName::* m, const char* name ) const {
                std::cerr << std::setw(10) << std::setiosflags(std::ios::left) << name 
                          << " " << boost::reflect::get_typename<typename M::signature>()
                          << (M::is_const ? "const" : "") <<std::endl;
                m_cli.methods[name] = cli_functor<typename M::fused_params, M&>(m_vtbl.*m);
           }
           VTableType&   m_vtbl;
           cli&          m_cli;
       };

       template<typename Seq, typename Functor>
       struct cli_functor
       {
           cli_functor( Functor f )
           :m_func(f){}

           typedef typename boost::remove_reference<Functor>::type functor_type;

           std::string operator()( const std::string& cli )
           {
                std::stringstream ss(cli);
                Seq s; ss >> boost::fusion::tuple_delimiter(',') >> s;
                std::stringstream rtn;
                rtn << m_func(s);
                return rtn.str();
           }
           Functor m_func;
       };
       std::map<std::string, boost::function<std::string(const std::string&)> > methods;
};


#endif
