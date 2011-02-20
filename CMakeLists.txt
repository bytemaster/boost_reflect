# Distributed under the Boost Software License, Version 1.0.
# See http://www.boost.org/LICENSE_1_0.txt

boost_project("IDL"
  AUTHORS
    "Daniel Larimer"
  DESCRIPTION
    "Interface Description Library proposal for Boost"
  )

file(GLOB_RECURSE headers boost/*.* )
boost_add_headers(${headers})

if( ENABLE_EXAMPLES )
    option( ENABLE_IDL_EXAMPLES "Build Boost.IDL Examples" ON )
    if( ENABLE_IDL_EXAMPLES )
        boost_add_executable( idl_simple  SOURCES examples/simple.cpp  LINK_BOOST_LIBRARIES )
        boost_add_executable( idl_rpc_client  SOURCES examples/rpc_client.cpp  LINK_BOOST_LIBRARIES system serialization)
        boost_add_executable( idl_rpc_server  SOURCES examples/rpc_server.cpp  LINK_BOOST_LIBRARIES system serialization)
    endif( ENABLE_IDL_EXAMPLES )
endif( ENABLE_EXAMPLES )

add_subdirectory(doc)
