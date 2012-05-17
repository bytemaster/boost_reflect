#ifndef _BOOST_REFLECT_ERROR_HPP_
#define _BOOST_REFLECT_ERROR_HPP_

namespace boost { namespace reflect {

    class bad_value_cast : public std::bad_cast {
        public:
            virtual const char * what() const throw() {
                return "boost::bad_value_cast: "
                       "failed conversion using boost::value";
            }
    };
} } // boost::reflect
#endif 
