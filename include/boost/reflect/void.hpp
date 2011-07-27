#ifndef __BOOST_REFLECT_VOID_HPP
#define __BOOST_REFLECT_VOID_HPP
#include <iostream>

namespace boost { namespace reflect {
    /**
     * @brief A type to replace void in generic code.
     *
     * void cannot be treated like any other type and thus always introduces
     * many 'special cases' to generic code.  This type is used in generic code any
     * all functors that return void can be adapted to return void_t before being
     * used with generic code via boost::reflect::adapt_void.
     */
    struct void_t{
        friend std::ostream& operator<<(std::ostream& os,const void_t&) {return os;}
        friend std::istream& operator>>(std::istream& is,void_t&)       {return is;}
    };

    /**
     *  @brief Converts functors returning void to functors returning \link 
               boost::refelct::void_t void_t   \link

     *  Generic code that deals with functions returning void is a special case
     *  that requires many work arounds.  This class adapts a void(FusionSeq) functor
     *  into a void_t(FusionSeq) functor.
     */
    template<typename R, typename Functor=int>
    struct adapt_void {
        typedef R result_type;

        adapt_void( const Functor _f):f(_f){}

        template<typename Seq>
        result_type operator()( const Seq& seq )const {
            return f(seq);
        }
        template<typename Seq>
        result_type operator()( Seq& seq )const {
            return f(seq);
        }
        Functor f;
    };
    #ifndef DOXYGEN
    template<typename Functor>
    struct adapt_void<void,Functor> {
        typedef void_t result_type;
        adapt_void( const Functor _f):f(_f){}

        template<typename Seq>
        result_type operator()( const Seq& seq )const {
            f(seq); return result_type();
        }
        template<typename Seq>
        result_type operator()( Seq& seq )const {
            f(seq); return result_type();
        }
        Functor f;
    };
    #endif
} }

#endif
