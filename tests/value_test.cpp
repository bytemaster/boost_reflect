#include <boost/cmt/log/log.hpp>
#include <boost/exception/all.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <mace/reflect/value.hpp>
#include <mace/reflect/reflect.hpp>
#include <utility>
#include <boost/fusion/include/make_vector.hpp>

  struct to_json_visitor : public mace::reflect::read_value_visitor {
    std::ostream& os;
    to_json_visitor( const to_json_visitor& v ):os(v.os){}
    to_json_visitor( std::ostream& v ):os(v){}
      virtual void operator()( const mace::reflect::value_cref& s )  { 
        if( s.is_array() ) {
           os<<'[';
             mace::reflect::const_iterator itr = s.begin();
             if( itr != s.end() )
                 while( true ) {
                   itr.value().visit(to_json_visitor(os));
                   ++itr;
                   if( itr == s.end() )
                      break;
                   else { os<<","; }
                 }
           os<<']';
        } else {
           os<<'{';
          mace::reflect::const_iterator itr = s.begin();
             if( itr != s.end() )
                 while( true ) {
                   os << '"'<< itr.key() <<"\":";
                   itr.value().visit(to_json_visitor(os));
                   ++itr;
                   if( itr == s.end() )
                      break;
                   else { os<<","; }
                 }
           os<<'}';
        }
      }
      virtual void operator()( const std::string& s ) {  os << '"'<<s<<'"'; }
      virtual void operator()( const uint64_t& s )    {  os << s; }
      virtual void operator()( const int64_t& s )     {  os << s; }
      virtual void operator()( const uint32_t& s )    {  os << s; }
      virtual void operator()( const int32_t& s )     {  os << s; }
      virtual void operator()( const uint16_t& s )    {  os << s; }
      virtual void operator()( const int16_t& s )     {  os << s; }
      virtual void operator()( const uint8_t& s )     {  os << (int)s; }
      virtual void operator()( const int8_t& s )      {  os << (int)s; }
      virtual void operator()( const double& s )      {  os << s; }
      virtual void operator()( const float& s )       {  os << s; }
      virtual void operator()( const bool& s )        {  os << s ? "true" : "false"; }
  };

static int test_count = 0;

struct sub_val {
  sub_val( const std::string& h, double d ):happy(h),day(d){}
  sub_val():day(0.0){}
  std::string happy;
  double      day;
};
MACE_REFLECT( sub_val, (happy)(day) );
struct test {
  test( int _a, std::string _b )
  :a(_a),b(_b){ slog("test(%1%,%2%)",_a,_b); ++test_count;  }
  test():a(0),b("default"){ slog("test()"); ++test_count; }

  test( test&& m ):a(m.a),b(std::forward<std::string>(m.b)),sub(std::forward<sub_val>(m.sub))
  ,data(std::forward<std::vector<sub_val> >(m.data) ){
    slog( "test(move)"); 
  }

  test( const test& c )
  :a(c.a),b(c.b){ slog("test(copy)"); ++test_count; }

  ~test() { slog( "~test" ); --test_count; }

  int a;
  std::string b;
  sub_val     sub;
  std::vector<sub_val> data;
};
MACE_REFLECT( test, (a)(b)(sub)(data) )

using namespace mace::reflect;


template<typename ValueType>
void print_fields( const ValueType& t ) {
  t.visit( to_json_visitor(std::cout) );
/*
  slog( "printing fields: %1%", t.size() );
  mace::reflect::const_iterator itr = t.begin();
  while( itr != t.end() ) {
    slog( "%3% %1% = %2%", itr.key(), itr.value().as<std::string>(), itr.value().type() );
    ++itr;
  }
  */
}
void test_seq() {
  value v(boost::fusion::make_vector( std::string("sequence str"), test( 5, "seq test" ) ) );
  print_fields(v);
}

void test_stdmap() {
  elog( "stdmap test" );
  std::map<std::string,test> map_test;
  map_test["hello"] = test( 1, "world" );
  map_test["goodbye"] = test( 2, "cruel world" );

  value_cref mv(map_test);
  slog( "hello.b = %1%", mv["hello"]["b"].as<std::string>() );
  slog( "goodbye.b = %1%", mv["goodbye"]["b"].as<std::string>() );

  value_ref mvr(map_test);
  slog( "hello.b = %1%", mv["hello"]["b"].as<std::string>() );
  slog( "goodbye.b = %1%", mv["goodbye"]["b"].as<std::string>() );

  mvr["new_field"]["b"].set_as<std::string>("Howdy Partner");
  slog( "new_field.b = %1%", mv["new_field"]["b"].as<std::string>() );
  slog( "new_field.b = %1%", mvr["new_field"]["b"].as<std::string>() );

  map_test["hello"].data.push_back( sub_val( "win", 8.8 ) );
  map_test["hello"].data.push_back( sub_val( "lose", 4.4 ) );
  print_fields(mv["hello"]);
  slog( "printing map fields now\n" );
  print_fields(mv);
}


void test_sub() {
  test t(1,"test_ref");
  t.sub.happy = "oh happy pie day";
  t.sub.day = 3.1416;

  value_cref v(t);

  BOOST_ASSERT( v["sub"]["happy"].as<std::string>() == "oh happy pie day" );
  print_fields( value_ref(t.sub) );
  print_fields( value_cref(t.sub) );
  print_fields( value(t.sub) );
}

void test_ref() {
  slog( "testing value_ref...." );
  test t(1,"test_ref");
  //value_ref not_empty; // should not compile!


  value_ref v(t);

  //v = t; // should not compile because reference must be asigned at construction
  BOOST_ASSERT( v["a"].as<int>() == 1 );
  BOOST_ASSERT( v["a"].as<std::string>() == std::string("1") );
  bool threw = false;
  try {
    v["b"].as<int>();
  } catch ( std::bad_cast& ) {
    threw = true;
  }
  BOOST_ASSERT(threw);

  v["b"].set_as<int>(2);

  try {
    BOOST_ASSERT( 2 == v["b"].as<int>() );
  } catch ( std::bad_cast& ) {
    BOOST_ASSERT( !threw );
  }

  BOOST_ASSERT( v["b"].as<std::string>() == std::string("2") );
  BOOST_ASSERT( t.b == v["b"].as<std::string>() );
}

void test_cref() {
  slog( "testing value_cref...." );
  test t(1,"test_cref");
  value_cref v(t);
  BOOST_ASSERT( v["a"].as<int>() == 1 );
  BOOST_ASSERT( v["a"].as<std::string>() == std::string("1") );
  bool threw = false;
  try {
    v["b"].as<int>();
  } catch ( std::bad_cast& ) {
    threw = true;
  }
  BOOST_ASSERT(threw);

  //v["b"].set_as<int>(2); // this should not compile... 

  BOOST_ASSERT( v["b"].as<std::string>() == std::string("test_cref") );
}

void test_ref_to_cref() {

}

void test_val() {
  slog( "testing value...." );
  test t(1,"test_val");
  value v(t);
  BOOST_ASSERT( v["a"].as<int>() == 1 );
  BOOST_ASSERT( v["a"].as<std::string>() == std::string("1") );
  BOOST_ASSERT( v["b"].as<std::string>() == std::string("test_val") );
  bool threw = false;
  try {
    v["b"].as<int>();
  } catch ( std::bad_cast& ) {
    threw = true;
  }
  BOOST_ASSERT(threw);

  v["b"].set_as<int>(2);

  try {
    BOOST_ASSERT( 2 == v["b"].as<int>() );
  } catch ( std::bad_cast& ) {
    BOOST_ASSERT( !threw );
  }

  BOOST_ASSERT( v["b"].as<std::string>() == std::string("2") );
  BOOST_ASSERT( t.b != v["b"].as<std::string>() );

  value v2(test(2,"test_val rval const 2"));
  BOOST_ASSERT( test_count == 3 );
  const value_cref cr(v2);
  slog("%1% =? %2%", cr.const_ptr<test>(), v2.ptr<test>() );
  cr["a"];
  BOOST_ASSERT( v2["a"].ptr<int>() == cr["a"].const_ptr<int>() );
  v2["a"].set_as<int>(55);
  BOOST_ASSERT( v2["a"].as<int>() == cr["a"].as<int>() );


}



int main( int argc, char** argv ) {
try {
  test_ref();
  BOOST_ASSERT( &test_ref&& (test_count == 0) );
  test_cref();
  BOOST_ASSERT( &test_cref&& (test_count == 0) );
  test_ref_to_cref();
  BOOST_ASSERT( &test_ref_to_cref&& (test_count == 0) );
  test_val();
  BOOST_ASSERT( &test_val&& (test_count == 0) );
  wlog( "test sub!" );
  test_sub();
  BOOST_ASSERT( &test_val&& (test_count == 0) );
  test_stdmap();
  wlog( "testing sequence" );
  test_seq();
  } catch ( const boost::exception& e ) {
    elog( "%1%", boost::diagnostic_information(e) );
  }
  return 0;
}
