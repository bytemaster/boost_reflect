#include <boost/reflect/any_ptr.hpp>
#include "cli.hpp"

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
    boost::signal<void(double)> got_result;
};

BOOST_REFLECT_ANY( Service, BOOST_PP_SEQ_NIL, (name)(exit) )
BOOST_REFLECT_ANY( Calculator, (Service), (add)(add2)(sub)(mult)(div)(result)(got_result) )

class CalculatorService
{
    public:
        CalculatorService():m_result(0){}

        std::string name()const            { return "CalculatorService"; }
        int   exit()                       { ::exit(0);                  }
        double add( double v )             { got_result(m_result += v);      return m_result;}
        double sub( double v )             { got_result(m_result -= v);      return m_result; }
        double mult( double v )            { got_result(m_result *= v);      return m_result; }
        double div( double v )             { got_result(m_result /= v);      return m_result; }
        double add2( double v, double v2 ) { got_result(m_result += v + v2); return m_result; }

        double result()const { return m_result; }

        boost::signal<void(double)> got_result;
    private:
        double m_result;
};

void print( double r ) {
    std::cerr<<"got result "<<r<<std::endl;
}
void print2( double r ) {
    std::cerr<<"got result2 "<<r<<std::endl;
}

int main( int argc, char** argv )
{
    CalculatorService* se = new CalculatorService();
    se->got_result(5);
    if( se->got_result.empty() )
        std::cerr<<"EMPTY!\n";
    se->got_result.connect(print);
    if( !se->got_result.empty() )
        std::cerr<<"NOT EMPTY!\n";
    se->got_result(5);


    boost::reflect::any_ptr<Calculator> s( se );
    s->got_result.connect(print2);
    se->got_result(1234);
    printf( "Result: %f\n", s->result() );

    cli  m_cli(s);

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
