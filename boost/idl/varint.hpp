#ifndef __BOOST_IDL_VARINT_HPP__
#define __BOOST_IDL_VARINT_HPP__

namespace boost { namespace idl {

    /**
     
      Varints are a method of serializing integers using one or more bytes. Smaller numbers take a smaller number of bytes.
      Each byte in a varint, except the last byte, has the most significant bit (msb) set – this indicates that there are further bytes to come. The lower 7 bits of each byte are used to store the two's complement representation of the number in groups of 7 bits, least significant group first.

      So, for example, here is the number 1 – it's a single byte, so the msb is not set:
      @code
      0000 0001
      @endcode

      And here is 300 – this is a bit more complicated:

      @code
      1010 1100 0000 0010
      @endcode

      How do you figure out that this is 300? First you drop the msb from each byte, as this is just there to tell us whether we've reached the end of the number (as you can see, it's set in the first byte as there is more than one byte in the varint):

      @code
      1010 1100 0000 0010
      → 010 1100  000 0010
      @endcode

      You reverse the two groups of 7 bits because, as you remember, varints store numbers with the least significant group first. Then you concatenate them to get your final value:

      @code
      000 0010  010 1100
      →  000 0010 ++ 010 1100
      →  100101100
      →  256 + 32 + 8 + 4 = 300
      @endcode
     
     */
    struct unsigned_int
    {
        unsigned_int( uint32_t v ):value(v){}
        
        uint32_t value;
    };

    /**
     
     However, there is an important difference between the signed int types (sint32 and sint64) and the "standard" int types (int32 and int64) when it comes to encoding negative numbers. If you use int32 or int64 as the type for a negative number, the resulting varint is always ten bytes long – it is, effectively, treated like a very large unsigned integer. If you use one of the signed types, the resulting varint uses ZigZag encoding, which is much more efficient.

     ZigZag encoding maps signed integers to unsigned integers so that numbers with a small absolute value (for instance, -1) have a small varint encoded value too. It does this in a way that "zig-zags" back and forth through the positive and negative integers, so that -1 is encoded as 1, 1 is encoded as 2, -2 is encoded as 3, and so on, as you can see in the following table:

    @code
      Original => Encoded
      0        => 0
      -1       => 1
      1        => 2
      -2       => 3
      2        => 4
    @endcode

    In other words, each value n is encoded using

    @code
    (n << 1) ^ (n >> 31)
    @endcode
    
     
     */
    struct signed_int
    {
        signed_int( int32_t v ):value(v){}
        int32_t value;
    };


    template<typename T, int V, int E>
    inline fl::datastream<T,V,E>& operator<<( fl::datastream<T,V,E>& ds, const unsigned_int& vi )
    {
        uint64_t v = vi.value;
        do 
        {
           uint8_t b = uint8_t(v) & 0x7f;
           v >>= 7;
           b |= ((v > 0) << 7);
           ds << b;
        }while( v );
        return ds;
    }

    template<typename T, int V, int E>
    inline fl::datastream<T,V,E>& operator>>( fl::datastream<T,V,E>& ds, unsigned_int& vi )
    {
        uint64_t v = 0;
        uint8_t b = 0;
        uint8_t by = 0;
        do {
            ds >> b;
            v |= uint32_t(b & 0x7f) << by;
            by += 7;
        } while( b & 0x80 );

        vi.value = v;

        return ds;
    }

    template<typename T, int V, int E>
    inline fl::datastream<T,V,E>& operator<<( fl::datastream<T,V,E>& ds, const signed_int& vi )
    {
        uint32_t v = (vi.value<<1) ^ (vi.value>>31);
        do
        {
           uint8_t b = uint8_t(v) & 0x7f;
           v >>= 7;
           b |= ((v > 0) << 7);
           ds << b;
        }
        while( v );
        return ds;
    }

    template<typename T, int V, int E>
    inline fl::datastream<T,V,E>& operator>>( fl::datastream<T,V,E>& ds, signed_int& vi )
    {
        uint32_t v = 0;
        uint8_t b = 0;
        int by = 0;
        do {
            ds >> b;
            v |= uint32_t(b & 0x7f) << by;
            by += 7;
        } while( b & 0x80 );

        vi.value = ((v>>1) ^ (v>>31)) + (v&0x01);
        vi.value = v&0x01 ? vi.value : -vi.value;

        return ds;
    }

} } // namespace boost::idl

#endif // __BOOST_IDL_VARINT_HPP__
