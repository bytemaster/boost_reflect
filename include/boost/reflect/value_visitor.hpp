#ifndef _BOOST_REFLECT_VALUE_VISITOR_HPP_
#define _BOOST_REFLECT_VALUE_VISITOR_HPP_
#include <string>
#include <boost/lexical_cast.hpp>
#include <boost/reflect/error.hpp>

namespace boost { namespace reflect {
  struct read_value_visitor {
      virtual ~read_value_visitor(){}
      template<typename T>
      void operator()( const T& s ){ BOOST_THROW_EXCEPTION( bad_value_cast() ); }

      virtual void operator()(){}
      virtual void operator()( const std::string& s ){}
      virtual void operator()( const uint64_t& s ){}
      virtual void operator()( const int64_t& s ){}
      virtual void operator()( const uint32_t& s ){}
      virtual void operator()( const int32_t& s ){}
      virtual void operator()( const uint16_t& s ){}
      virtual void operator()( const int16_t& s ){}
      virtual void operator()( const uint8_t& s ){}
      virtual void operator()( const int8_t& s ){}
      virtual void operator()( const double& s ){}
      virtual void operator()( const float& s ){}
      virtual void operator()( const bool& s ){}
  };

  struct write_value_visitor {
      virtual ~write_value_visitor(){}
      template<typename T>
      void operator()( T& s ){ BOOST_THROW_EXCEPTION( bad_value_cast() ); }

      virtual void operator()(){}
      virtual void operator()( std::string& s ){}
      virtual void operator()( uint64_t& s ){}
      virtual void operator()( int64_t& s ){}
      virtual void operator()( uint32_t& s ){}
      virtual void operator()( int32_t& s ){}
      virtual void operator()( uint16_t& s ){};
      virtual void operator()( int16_t& s ){}
      virtual void operator()( uint8_t& s ){}
      virtual void operator()( int8_t& s ){}
      virtual void operator()( double& s ){}
      virtual void operator()( float& s ){}
      virtual void operator()( bool& s ){}
  };

  template<typename T>
  struct get_visitor : read_value_visitor {
    T& m_val;
    get_visitor( T& v ):m_val(v){}
      virtual void operator()( const std::string& s ) { m_val = boost::lexical_cast<T>(s); }
      virtual void operator()( const uint64_t& s )    { m_val = boost::lexical_cast<T>(s); }
      virtual void operator()( const int64_t& s )     { m_val = boost::lexical_cast<T>(s); }
      virtual void operator()( const uint32_t& s )    { m_val = boost::lexical_cast<T>(s); }
      virtual void operator()( const int32_t& s )     { m_val = boost::lexical_cast<T>(s); }
      virtual void operator()( const uint16_t& s )    { m_val = boost::lexical_cast<T>(s); }
      virtual void operator()( const int16_t& s )     { m_val = boost::lexical_cast<T>(s); }
      virtual void operator()( const uint8_t& s )     { m_val = boost::lexical_cast<T>(s); }
      virtual void operator()( const int8_t& s )      { m_val = boost::lexical_cast<T>(s); }
      virtual void operator()( const double& s )      { m_val = boost::lexical_cast<T>(s); }
      virtual void operator()( const float& s )       { m_val = boost::lexical_cast<T>(s); }
      virtual void operator()( const bool& s )        { m_val = boost::lexical_cast<T>(s); }
  };

  template<typename T>
  struct set_visitor : write_value_visitor {
      const T& m_val;
      set_visitor( const T& v ):m_val(v){}

      virtual void operator()( std::string& s ) { s = boost::lexical_cast<std::string>(m_val); }
      virtual void operator()( uint64_t& s )    { s = boost::lexical_cast<uint64_t>(m_val);    }
      virtual void operator()( int64_t& s )     { s = boost::lexical_cast<int64_t>(m_val);     }
      virtual void operator()( uint32_t& s )    { s = boost::lexical_cast<uint32_t>(m_val);    }
      virtual void operator()( int32_t& s )     { s = boost::lexical_cast<int32_t>(m_val);     }
      virtual void operator()( uint16_t& s )    { s = boost::lexical_cast<uint16_t>(m_val);    }
      virtual void operator()( int16_t& s )     { s = boost::lexical_cast<int16_t>(m_val);     }
      virtual void operator()( uint8_t& s )     { s = boost::lexical_cast<uint8_t>(m_val);     }
      virtual void operator()( int8_t& s )      { s = boost::lexical_cast<int8_t>(m_val);      }
      virtual void operator()( double& s )      { s = boost::lexical_cast<double>(m_val);      }
      virtual void operator()( float& s )       { s = boost::lexical_cast<float>(m_val);       }
      virtual void operator()( bool& s )        { s = boost::lexical_cast<bool>(m_val);        }
  };


} }

#endif 
