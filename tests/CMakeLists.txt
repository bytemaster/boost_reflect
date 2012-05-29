include_directories( ../include )
include_directories( ../../atomic/include )
include_directories( ../../cmt/include )
include_directories( ../../move/include )
SET( libraries 
    #/usr/local/lib/tcmalloc/libtcmalloc.a
    mace_cmt
    boost_context 
    pthread
    ${rt_library}
    ${Boost_THREAD_LIBRARY} 
    ${Boost_ASIO_LIBRARY} 
    ${Boost_SYSTEM_LIBRARY} 
    ${Boost_SIGNALS_LIBRARY})

add_executable( value_test  value_test.cpp  )
target_link_libraries( value_test ${libraries} )
#add_executable( value_function  value_function.cpp  )
#target_link_libraries( value_test ${libraries} )
