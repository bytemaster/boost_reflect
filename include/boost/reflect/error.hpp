#ifndef _BOOST_REFLECT_ERROR_HPP_
#define _BOOST_REFLECT_ERROR_HPP_

namespace boost { namespace reflect {

    typedef boost::error_info<struct err_msg_,std::string> err_msg;

    class bad_value_cast : public std::bad_cast {
        public:
            virtual const char * what() const throw() {
                return "boost::reflect::bad_value_cast: "
                       "failed conversion using boost::value";
            }
    };
    class unknown_field : public virtual boost::exception, public virtual std::exception {
        public:
            virtual const char * what() const throw() {
                return "boost::reflect::unknown_field: "
                       "attempted to access an unknown field";
            }
    };
} } // boost::reflect
#endif 
