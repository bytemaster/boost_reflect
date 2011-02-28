#include <boost/exception/diagnostic_information.hpp>
#include "calculator.hpp"
#include "rpc.hpp"
#include <boost/lexical_cast.hpp>

class CalculatorServer
{
    public:
        CalculatorServer():m_result(0){}

        std::string name()const            { return "CalculatorServer"; }
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

int main( int argc, char** argv )
{
    if( argc <= 1 )
    {
        std::cerr << "Usage: rpc_server PORT\n";
        return -1;
    }
    using namespace boost;
    try {
        boost::shared_ptr<CalculatorServer> calc(new CalculatorServer());
        reflect::rpc_server<Calculator> server( calc );
        server.listen( lexical_cast<uint16_t>(argv[1]) );
    } catch ( const boost::exception& e )
    {
        std::cerr << boost::diagnostic_information(e) << std::endl;
    }
    return 0;
}


