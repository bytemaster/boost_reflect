#ifndef __BOOST_REFLECT_VOID_HPP
#define __BOOST_REFLECT_VOID_HPP
#include <iostream>

namespace boost { namespace reflect {
    struct void_t{
        friend std::ostream& operator<<(std::ostream& os,const void_t&) {return os;}
        friend std::istream& operator>>(std::istream& is,void_t&)       {return is;}
    };

    /**
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
} }

#endif
