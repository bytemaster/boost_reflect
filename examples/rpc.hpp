#ifndef _BOOST_IDL_RPC_HPP_
#define _BOOST_IDL_RPC_HPP_
#include <boost/idl/mirror_interface.hpp>
#include <boost/idl/idl.hpp>

#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/container/vector.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/fusion/container/vector/vector_fwd.hpp>
#include <boost/fusion/include/vector_fwd.hpp>
#include <boost/fusion/container/generation/make_vector.hpp>
#include <boost/fusion/include/make_vector.hpp>
#include <boost/fusion/sequence/comparison/equal_to.hpp>
#include <boost/fusion/include/equal_to.hpp>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/system/system_error.hpp>
#include <boost/asio.hpp>

namespace boost { namespace idl {

template<typename Archive>
struct item_serializer 
{
    item_serializer(Archive& ar):ar(ar) {}

    template<typename T>
    void operator()(const T& o) const {
        ar << o;
    }
    Archive& ar;
};

template<typename Archive, typename V>
Archive& serialize_fusion_vector(Archive& ar, const V& v) 
{
    boost::fusion::for_each(v, item_serializer<Archive>(ar));
    return ar;
}

template<typename Archive>
struct item_deserializer 
{
    item_deserializer(Archive& ar):ar(ar) {}

    template<typename T>
    void operator()(T& o) const {
        ar >> o;
    }
    Archive& ar;
};

template<typename Archive, typename V>
Archive& deserialize_fusion_vector(Archive& ar, V& v) 
{
    boost::fusion::for_each(v, item_deserializer<Archive>(ar));
    return ar;
}



/**
 *  For each method on the interface, create a functor that will accept the
 *  methods parameters, seralize them, and send them out the socket.
 */
template<typename InterfaceType>
class rpc_client : public boost::idl::visitor< rpc_client<InterfaceType> >, public idl::any<InterfaceType>
{
    public:
       rpc_client()
       :m_ios(),m_sock(m_ios)
       {
           start_visit(*this);
       }

       template<typename InterfaceName, typename M>
       bool accept( M& m, const char* name )
       {
            m.m_delegate = rpc_functor<typename M::fused_params, typename M::result_type>(*this,name);
            return true;
       }
       boost::function<std::string(const std::string)>& operator[]( const std::string& name ) 
       { return methods[name]; }

       bool connect_to( const std::string& host, uint16_t port )
       {
           m_sock.open(boost::asio::ip::udp::v4());
           m_ep = boost::asio::ip::udp::endpoint( boost::asio::ip::address::from_string(host), port );
       }

       std::string invoke( const char* name, const std::string& params )
       {
            std::ostringstream os;
            {  
                boost::archive::binary_oarchive oa(os);
                std::string n(name);
                oa << n;
                oa << params;
            }
            m_sock.send_to( boost::asio::buffer( os.str() ), m_ep );
            boost::asio::ip::udp::endpoint rep;
            std::vector<char> recv_buf(2048);
            size_t len = m_sock.receive_from( boost::asio::buffer(recv_buf), rep );
            return std::string(&recv_buf.front(),len);
       }
   private:
       template<typename Seq, typename ResultType>
       struct rpc_functor
       {
           rpc_functor( rpc_client& c, const char* name )
           :m_client(c),m_name(name){}

           ResultType operator()( const Seq& params )
           {
                std::ostringstream os;
                {  
                    boost::archive::binary_oarchive oa(os);
                    serialize_fusion_vector(oa, params);
                }
                ResultType  ret_val;
                std::istringstream is(m_client.invoke( m_name, os.str() ) );
                {
                    boost::archive::binary_iarchive ia(is);
                    ia >> ret_val;
                }
                return ret_val;
           }
           const char* m_name;
           rpc_client& m_client;
       };
       boost::asio::ip::udp::endpoint m_ep;
       boost::asio::io_service        m_ios;
       boost::asio::ip::udp::socket   m_sock;
       std::map<std::string, boost::function<std::string(const std::string)> > methods;
};

    template<typename T>
    class set_delegate_visitor< rpc_client<T> > : 
        public boost::idl::visitor< set_delegate_visitor< rpc_client<T> >  >
    {
        public:
           set_delegate_visitor( rpc_client<T>* self = 0)
           :m_self(self){}

           template<typename InterfaceName, typename M>
           bool accept( M& m, const char* name )
           {
                M::get_member_on_type( m_self, m.m_delegate );
                return true;
           }
       private:
           rpc_client<T>* m_self;
    };


/**
 *  Create a server socket that accepts new messages and then
 *  unpacks the parameters and then invokes them on the object.
 */
template<typename InterfaceType>
class rpc_server : public boost::idl::visitor< rpc_server<InterfaceType> >, public idl::any<InterfaceType>
{
    public:
       template<typename T>
       rpc_server( T v )
       :idl::any<InterfaceType>(v)
       {
            start_visit(*this); 
       }

       void listen( uint16_t port )
       {
            using namespace boost::asio::ip;
            boost::asio::io_service io_service;
            udp::socket  socket( io_service, udp::endpoint(udp::v4(), port ) );
            std::vector<char>  recv_buf(2048);
            for( ;; )
            {
                udp::endpoint remote_ep;
                boost::system::error_code err;
                size_t bytes_recv = socket.receive_from( boost::asio::buffer(recv_buf),
                                     remote_ep, 0, err );
                if( err && err != boost::asio::error::message_size )
                    throw boost::system::system_error(err);

                std::string         buf(&recv_buf.front(),bytes_recv );
                std::string         method;
                std::string         params;
                {
                    std::istringstream iss( buf );
                    boost::archive::binary_iarchive ia(iss);
                    ia >> method;
                    ia >> params;
                }
                boost::system::error_code ignored_error;
                socket.send_to( boost::asio::buffer( methods[method](params) ),
                                remote_ep, 0, ignored_error );
            }
       }

       template<typename InterfaceName, typename M>
       bool accept( M& m, const char* name )
       {
            methods[name] = rpc_functor<typename M::fused_params, M&>(m);
            return true;
       }
       boost::function<std::string(const std::string)>& operator[]( const std::string& name ) 
       { return methods[name]; }

    private:
       template<typename Seq, typename Functor>
       struct rpc_functor
       {
           rpc_functor( Functor f )
           :m_func(f){}

           std::string operator()( const std::string& params )
           {
                Seq paramv;
                std::istringstream is(params);
                {
                    boost::archive::binary_iarchive ia(is);
                    deserialize_fusion_vector(ia,paramv);                    
                }
                std::ostringstream os;
                {
                    boost::archive::binary_oarchive oa(os);
                    typename boost::remove_reference<Functor>::type::result_type r = m_func(paramv);
                    oa << r;
                }
                return os.str();
           }
           Functor m_func;
       };

       std::map<std::string, boost::function<std::string(const std::string)> > methods;
};

} } // namespace boost::idl

#endif
