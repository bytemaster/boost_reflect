#include <boost/idl/mirror_interface.hpp>
#include <boost/idl/idl.hpp>
#include <boost/fusion/sequence/io.hpp>
#include <sstream>
#include <iomanip>
#include <iostream>

struct Service
{
    std::string name()const;
    int         exit();
};
struct Calculator : Service
{
    double add( double v );           
    double add2( double v, double v2 );
    double sub( double v );           
    double mult( double v );           
    double div( double v );           
    double result()const;
};

BOOST_IDL_INTERFACE( Service, BOOST_PP_NIL, (name)(exit) )
BOOST_IDL_INTERFACE( Calculator, (Service, BOOST_PP_NIL), (add)(add2)(sub)(mult)(div)(result) )

class CalculatorService
{
    public:
        CalculatorService():m_result(0){}

        std::string name()const            { return "CalculatorService"; }
        int   exit()                       { ::exit(0);                  }
        double add( double v )             { return m_result += v;       }
        double sub( double v )             { return m_result -= v;       }
        double mult( double v )            { return m_result *= v;       }
        double div( double v )             { return m_result /= v;       }
        double add2( double v, double v2 ) { return m_result += v + v2;  }

        double result()const { return m_result; }

    private:
        double m_result;
};


/**
 *  Takes any interface object and provides a command line interface for it.
 */
class cli : public boost::idl::visitor< cli >
{
    public:
       template<typename InterfaceName, typename M>
       bool accept( M& m, const char* name )
       {
            std::cerr << std::setw(10) << std::setiosflags(std::ios::left) << name 
                      << " " << boost::idl::get_typestring<typename M::signature>::str()
                      << (M::is_const ? "const" : "") <<std::endl;
            methods[name] = cli_functor<typename M::fused_params, M&>(m);
            return true;
       }
       boost::function<std::string(const std::string)>& operator[]( const std::string& name ) 
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
       std::map<std::string, boost::function<std::string(const std::string)> > methods;
};

int main( int argc, char** argv )
{
    boost::idl::any<Calculator> s = CalculatorService();

    printf( "Result: %f\n", s.result() );

    cli  m_cli;
    m_cli.start_visit(s);

    std::string line;
    std::string cmd;
    std::string args;

    while( true )
    {
        std::cerr << "Enter Method: ";
        std::getline( std::cin, line );
        cmd = line.substr( 0, line.find('(') );
        args = line.substr( cmd.size(), line.size() );
        std::cerr << m_cli[cmd](args) << std::endl;
    }

    return 0;
}
