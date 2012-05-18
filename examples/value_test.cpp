//#include <boost/reflect/value_ref.hpp>
#include <boost/reflect/value_cref.hpp>
#include <boost/cmt/log/log.hpp>
#include <boost/exception/all.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/reflect/value.hpp>

struct test {
  test( int _a, std::string _b )
  :a(_a),b(_b){ }
  test():a(0),b("default"){ }

  test( const test& c )
  :a(c.a),b(c.b){  }

  ~test() { }

  int a;
  std::string b;
};
BOOST_REFLECT( test, (a)(b) )

void test_cr( const test& t ) {
  boost::reflect::value_cref r(t);
  slog("%1%", r["a"].get<int>());// == int(5) ); 
  slog("%1%", r["b"].get<std::string>() );
}
void test_rv( const boost::reflect::value_cref& v ) {
  slog( "b:%1%", v.ptr<test>()->b );
  test_cr(v.get<test>());
}


using namespace boost::reflect;


void test_visitor( value_ref& v ) {
  std::cout<<"as str: "<<v["a"].as<std::string>()<<std::endl;
  try {
  std::cout<<"as str: "<<v["b"].as<int>()<<std::endl;
    elog( "didn't catch bad cast!\n" );
  }catch(std::bad_cast& ){
    slog( "caught expected bad cast\n" );
  }
  slog( "try set as int" );
  v["b"].set_as<int>(55); // does not compile as expected, generates assert
  try {
    std::cout<<"as str: 55  ?= "<<v["b"].as<std::string>()<<std::endl;
    std::cout<<"as int: 55  ?= "<<v["b"].as<int>()<<std::endl;
    slog( "cast worked this time!\n");
  }catch(std::bad_cast& ){
    elog( "caught expected bad cast when it should have worked\n" );
  }
}

void test_const_visitor( const value_cref& v ) {
  std::cout<<"as str: "<<v["a"].as<std::string>()<<std::endl;
  try {
  std::cout<<"as str: "<<v["b"].as<int>()<<std::endl;
    elog( "didn't catch bad cast!\n" );
  }catch(std::bad_cast& ){
    slog( "caught expected bad cast\n" );
  }
  slog( "try set as int" );
  //v["b"].set_as<int>(5); // does not compile as expected, generates assert
}

void test_ref_passed_to_cref( const value_cref& v ) {
  slog( "ref passed to cref..." );
  BOOST_ASSERT( v["a"] == int(6) ); 
  BOOST_ASSERT( v["b"] == std::string("good bye\n") ); 
  slog( "passed const test\n" );

  value_cref v2;

  v2 = v;

  test_const_visitor(v2);
}

void test_value_ref() {
  test t;
  t.a = 5;
  t.b = "hello world";
  boost::reflect::value_ref  r(t);
  test_const_visitor(r);
  test_visitor(r);
  slog("passed ");
  BOOST_ASSERT( r["a"] == int(5) ); 
  BOOST_ASSERT( r["b"] == std::string("hello world") ); 
  slog("passed compare check");
  r["a"] = int(6);
  r["b"] = std::string( "good bye\n" );
  BOOST_ASSERT( r["a"] == int(6) ); 
  BOOST_ASSERT( r["b"] == std::string("good bye\n") ); 
  slog( "passed assign\n" );

  slog( "a: %1%   b: %2%", r["a"].get<int>(), r["b"].get<std::string>() );

  slog("calling tesT_ref_passed_to_cref" );
  test_ref_passed_to_cref(r);
}


void test_value_cref() {
  test t;
  t.a = 5;
  t.b = "hello world";
  boost::reflect::value_cref r(t);
  BOOST_ASSERT( r["a"] == int(5) ); 
  BOOST_ASSERT( r["b"] == std::string("hello world") ); 
  BOOST_ASSERT( !r["d"] );
  BOOST_ASSERT( !!r["a"] );

  //r["a"] = int(6);                      // this should not compile
  //r["b"] = std::string( "good bye\n" ); // this should not compile
}


int main( int argc, char** argv ) {
  try {
    test_value_ref();
    test_value_cref();
  } catch( const boost::exception& e ) {
    elog( "%1%", boost::diagnostic_information(e) );
  } catch( const std::exception& e ) {
    elog( "%1%", boost::diagnostic_information(e) );
  }
}
