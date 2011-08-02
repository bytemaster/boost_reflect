Boost.Reflect - Simple Reflection Library
---------------------------------------

The Boost Reflect library provides a means to describe the interface
of a class or struct to provide compile-time and run-time reflection.

Some of the useful features provided by the Reflect library include:

* boost::reflect::any<Interface>  - a class that can contain any object
    that implements Interface.
* Use visitors to create dynamic interfaces.
    - RPC client/server stubs
    - Command Line Interface
    - Scripting Interfaces
    - Asynchronous Actors


Example: Simple Command Line Calculator
---------------------------------------

An [example](https://github.com/bytemaster/boost_reflect/blob/master/examples/simple.cpp) 
of how to create a command line calculator can be found in the examples directory.


    #include <boost/reflect/any.hpp>

    struct calculator {
        calculator():result_(0){}
        double add( double v ) { return result_ += v; }
        double sub( double v ) { return result_ -= v; }
        double result()        { return result_;      }

        private:
            double result_;
    };

    BOOST_REFLECT_ANY( calculator, BOOST_PP_NIL, (add)(sub)(result) )

    int main( int argc, char** argv ) {
      boost::reflect::any_ptr<Calculator> s( new CalculatorService() );
      printf( "Result: %f\n", s->result() );

      cli  m_cli(s);

      std::string line;
      std::string cmd;
      std::string args;

      while( true ) {
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

Boost.CMT can be built by checking out my development repository: https://github.com/bytemaster/dev

### Installation ## 

    git clone https://github.com/bytemaster/dev
    cd dev
    git submodule init
    git submodule update
    cmake .
    make
    make install
    @endcode
