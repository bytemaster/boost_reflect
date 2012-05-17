//#include <boost/reflect/value_ref.hpp>
#include <boost/reflect/value_cref.hpp>
#include <boost/cmt/log/log.hpp>
#include <boost/reflect/detail/value_cref.ipp>
#include <boost/reflect/detail/value_ref.ipp>
#include <boost/reflect/detail/value.ipp>
#include <boost/exception/all.hpp>
#include <boost/exception/diagnostic_information.hpp>

struct test {
  test( int _a, std::string _b )
  :a(_a),b(_b){
    slog( "this: %1%", this );
  }
  test():a(0),b("default"){ slog( "default: %1%", this); }

  test( const test& c )
  :a(c.a),b(c.b){ slog( "%1% copy tesT from %2%", this, &c ); }

  ~test() {
    slog("%1%",this);
  }

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

void test_ref_passed_to_cref( const value_cref& v ) {
  slog( "ref passed to cref..." );
  BOOST_ASSERT( v["a"] == int(6) ); 
  BOOST_ASSERT( v["b"] == std::string("good bye\n") ); 
  slog( "passed const test\n" );

  value_cref v2;

  v2 = v;
}

void test_value_ref() {
  test t;
  t.a = 5;
  t.b = "hello world";
  boost::reflect::value_ref  r(t);
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
