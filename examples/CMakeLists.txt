include_directories( ../include )
SET( libraries 
    #/usr/local/lib/tcmalloc/libtcmalloc.a
    boost_cmt
    boost_context 
    ${Boost_THREAD_LIBRARY} 
    ${Boost_ASIO_LIBRARY} 
    ${Boost_SYSTEM_LIBRARY} 
    ${Boost_SIGNALS_LIBRARY})

add_executable( reflect_simple  simple.cpp  )
target_link_libraries( reflect_simple ${libraries} )
add_executable( reflect_signal  signal.cpp  )
target_link_libraries( reflect_signal ${libraries} )
