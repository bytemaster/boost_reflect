#ifndef _BOOST_IDL_STRUCTURE_HPP_
#define _BOOST_IDL_STRUCTURE_HPP_
#include <boost/idl/varint.hpp>
#include <boost/variant.hpp>

namespace boost { namespace idl {

    /**
     *  A record is a key value pair.
     *  The value is either a standard type (number or string) 
     *  or it a vector of records.
     */
    template<typename Value>
    struct protobuf_record
    {
        uint32_t     key;
        Value        value;
    };

    template<typename Value>
    struct json_record
    {
        std::string  key;
        Value        value;
    };

    /**
     *  This structure is a generic representation of any JSON or
     *  Google Protocol Buffer structure complete with type information.
     *  
     *  When parsing a JSON file, this is the 'generic' storage 
     *  structure.
     */
    template<template<typename> struct RecordType>
    struct record_value 
    {
        typedef boost::make_recursive_variant<
            int32_t, 
            uint32_t, 
            int64_t, 
            uint64_t, 
            bool, 
            double, 
            float, 
            signed_int, 
            unsigned_int,
            // unsigned_int size, followed by size bytes
            std::string, 
            // packed array, all having same key as 'this'
            std::vector< boost::recursive_variant_ > >::type record_value;
            // nested key/value pairs
            std::vector< RecordType<boost::recursive_variant_> > >::type type;
    };

    typedef protobuf_record< record_value<protobuf_record>::type> protobuf_kvp;
    typedef json_record< record_value<json_record>::type>         json_kvp;

    struct protobuf_variant
    {
        enum types { varint           = 0, 
                     bit64            = 1,
                     lengh_delimited  = 2,
                     bit32            = 5, 
                     bit16            = 6,
                     bit8             = 7 
                    }
        protobuf_variant()
        :number(0),bytes(NULL),type(0){}

        uint64_t number; // value or size
        char*    bytes;  // points to location in buf with data
        uint8_t  type;   // type.
    };

    // when parsing a protocol buffer, the keys/types and data location 
    // are stored in this temporary generic structure.  
    std::vector<std::pair<uint32_t,protobuf_variant> > protobuf_message;


    template<typename StreamType = char*> 
    class datastream
    {
        public:
            datastream( char* buf = 0, size_t len = 0 )
            :buffer(buf),pos(buf),end(buf+len){}

            int64_t pos()const  { return pos - buffer; }
            size_t  size()const { return end-buffer;   }

            datastream& pack ( const std::vector<char>& v, const char* name = NULL, uint32_t key = 0 )
            {
               pack( unsigned_int(v.size()) ); 
               if( pos + v.size() <= end && v.size() )
                    memcpy( pos, &v.front(), v.size() )
               pos += v.size();
               return *this;
            }

            datastream& unpack ( std::vector<char>& v, const char* name = NULL, uint32_t key = 0 )
            {
               unsigned_int s;
               unpack(s);
               if( pos + s.value <= end && s.value )
               {
                    v.resize(s.value);
                    memcpy( &v.front(), pos, s.value )
               }
               pos += s.value;
               return *this;
            }

            datastream& write( const char* data, size_t len, const char* name = NULL, uint32_t key = 0 )

            template<typename Fundamental>
            datastream& pack ( const Fundamental& v, const char* name = NULL, uint32_t key = 0 );
            {
                BOOST_STATIC_ASSERT( boost::is_fundamental<Fundamental>::value );
                if( pos + sizeof(v) <= end )
                    memcpy( pos, &v, sizeof(v) );
                pos += sizeof(v);
                return *this;
            }

            template<typename Fundamental>
            datastream& unpack( Fundamental& v, const char* name = NULL, uint32_t key = 0 )
            {
                BOOST_STATIC_ASSERT( boost::is_fundamental<Fundamental>::value );
                if( pos + sizeof(v) <= end )
                    memcpy( &v, pos, sizeof(v) );
                pos += sizeof(v);
                return *this;
            }

        private:
            char*       buffer;
            char*       pos;
            const char* end;
    };











    BOOST_IDL_STRUCTURE( Name,
        (field, id, required)
        (field, id, optional)
        (field, id, deprecated)
    )

    struct Nested
    {
        std::string a;
        std::string b;
    };
    struct TestCase
    {
        int         value;  // key 1
        double      dub;    // key 2
        std::string str;    // key 3
        Nested      n;      // key 4
    };

    // generated with macro
    template<typename Visitor>
    inline void visit( const TestCase& tc, Visitor& v, uint32_t field = -1 )
    {
        switch( field )
        {
            case -1:
                v.accept( tc.value, "value", flags );
                v.accept( tc.dup, "dub", flags );
                v.accept( tc.str, "str", flags );
                v.accept( tc.n, "n", flags );
                break;
            case 1: v.accept( &tc, &TestCase::value, "value", flags ); break;
            case 2: v.accept( &tc, &TestCasetc.value, "value", flags ); break;
        }
    }

    template<typename Visitor>
    inline void visit( TestCase& tc, Visitor& v, uint32_t field = -1)
    {
        visit( *(BaseClass*(&tc)), v, field );
        v.accept_member( tc.value, "value", flags );
        v.accept_member( tc.dup, "dub", flags );
        v.accept_member( tc.str, "str", flags );
        v.accept_member( tc.n, "n", flags );
    }


    struct json_write
    {
        template<typename T, typename Flags>
        inline void accept( const T& v, const char* name, Flags f )
        {
            // start nested value
            visit( v, *this ); 
            // end nested value
        }
        template<typename T, typename Flags>
        inline void accept( const std::vector<T>& v, const char* name, Flags f )
        {
            // start array with name
            for( uint32_t i = 0; i < v.size(); ++i )
            {
                // start key
                visit( v[i], *this );
                // end key
            }
            // end array with name
        }
        template<typename Flags>
        inline void accept( int8_t i, const char* name, Flags f )
        {
            // proto buffer would convert i to varint
        }
        template<typename Flags>
        inline void accept( int16_t i, const char* name, Flags f )
        {

        }
        template<typename Flags>
        inline void accept( int32_t i, const char* name, Flags f )
        {

        }
        template<typename Flags>
        inline void accept( int64_t i, const char* name, Flags f )
        {

        }
        template<typename Flags>
        inline void accept( uint8_t i, const char* name, Flags f )
        {

        }
        template<typename Flags>
        inline void accept( uint16_t i, const char* name, Flags f )
        {

        }
        template<typename Flags>
        inline void accept( uint32_t i, const char* name, Flags f )
        {

        }
        template<typename Flags>
        inline void accept( uint64_t i, const char* name, Flags f )
        {

        }
        template<typename Flags>
        inline void accept( double i, const char* name, Flags f )
        {

        }
        template<typename Flags>
        inline void accept( float i, const char* name, Flags f )
        {

        }
        template<typename Flags>
        inline void accept( const std::string& i, const char* name, Flags f )
        {

        }
    };


    record_value unpack( int32_t i&, const record_value& rv )
    {
        
    }

    template<typename T>
    record_value pack( const TestCase& tc )
    {
        std::vector<kvp> tmpv();
        tmpv.reserve(4);
        tmpv.push_back( kvp( 1, pack( value ) ) );
        tmpv.push_back( kvp( 2, pack( dub   ) ) );
        tmpv.push_back( kvp( 3, pack( str   ) ) );
        tmpv.push_back( kvp( 8, pack( n     ) ) );
        return tmpv; 
    }

    /**
     *  When unpacking, we assume fields are in the same order
     *  that we packed; however, they are allowed to be
     *  in any order.  If the expected key is not where we
     *  put it, then 'find it'.  
     *
     */
    void unpack( TestCase& tc, const record_value& rv )
    {
        // throw if there are required fields and rv != vector<kvp> 
        std::vector<kvp>& kvps = boost::get<std::vector<kvp>(rv);
        for( uint32_t i = 0; i < kvps.size(); ++i )
        {
            switch( kvps[i].key )
            {
                case 1: unpack( tc.value, kvps[i].value ); break;
                case 2: unpack( tc.dub, kvps[i].value ); break;
                case 3: unpack( tc.str, kvps[i].value ); break;
                case 8: unpack( tc.n, kvps[i].value ); break;
                defaut:
                    //  do nothing
            }
        }
    }

    
    structure tmp = std::move( tmpv );
    
    




    struct record_ref
    {
        uint32_t key;
        char*    data;
    }


} }

#endif
