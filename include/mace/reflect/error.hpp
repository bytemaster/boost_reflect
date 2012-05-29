#ifndef _MACE_REFLECT_ERROR_HPP_
#define _MACE_REFLECT_ERROR_HPP_

namespace mace { namespace reflect {

    typedef boost::error_info<struct err_msg_,std::string> err_msg;
    
    /**
     *  @brief thrown when a conversion using mace::value, mace::value_ref, or mace::value_cref fails.
     */
    class bad_value_cast : public std::bad_cast {
        public:
            virtual const char * what() const throw() {
                return "mace::reflect::bad_value_cast: "
                       "failed conversion using mace::value";
            }
    };
    /**
     *  @brief thrown when attempt to access non-existant (non-reflected) member by name.
     */
    class unknown_field : public virtual boost::exception, public virtual std::exception {
        public:
            virtual const char * what() const throw() {
                return "mace::reflect::unknown_field: "
                       "attempted to access an unknown field";
            }
    };
} } // mace::reflect
#endif 
