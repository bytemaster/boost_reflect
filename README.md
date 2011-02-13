Boost.IDL - Interface Description Language 
---------------------------------------

The Boost IDL library provides a means to describe the interface
of a class or struct to provide compile-time and run-time reflection.

Some of the useful features provided by the IDL library include:

* boost::idl::any<Interface>  - a class that can contain any object
    that implements Interface.
* Use visitors to create dynamic interfaces.
    - RPC client/server stubs
    - Command Line Interface
    - Scripting Interfaces
    - Asynchronous Actors


Example: Simple Command Line Calculator
---------------------------------------

An [example](https://github.com/bytemaster/boost_idl/blob/master/examples/simple.cpp) 
of how to create a command line calculator can be found in the examples directory.


    #include <boost/idl/idl.hpp>

    struct calculator {
        calculator():result_(0){}
        double add( double v ) { return result_ += v; }
        double sub( double v ) { return result_ -= v; }
        double result()        { return result_;      }

        private:
            double result_;
    };

    BOOST_IDL_INTERFACE( calculator, BOOST_PP_NIL, (add)(sub)(result) )

    int main( int argc, char** argv )
    {
        boost::idl::any<calculator> calc = calculator();

        cli  m_cli;
        m_cli.start_visit(calc);

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

### Output ###

    add        double(double)
    sub        double(double)
    result     double()const

    Enter Method: add(5)
    5
    Enter Method: sub(3)
    2
    Enter Method: result()
    2

