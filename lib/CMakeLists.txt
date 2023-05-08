#if (EPOLL_EXISTS)
#    set(yolanda_lib
#            channel.cpp
#            )
#else ()
#    set(yolanda_lib
#            channel.cpp
#            )
#endif ()
set(yolanda_lib
        channel.cpp
        buffer.cpp
        log.cpp
        sync_cond.cpp
        epoll_dispatcher.cpp
        event_loop.cpp
        event_loop_thread.cpp
        tcp_server.cpp
        acceptor.cpp
        tcp_connection.cpp
        http_request.cpp
        http_response.cpp
        http_layer.cpp
    )


add_library(yolanda ${yolanda_lib})
target_link_libraries(yolanda pthread rt)

install(TARGETS yolanda DESTINATION lib)

#file(GLOB HEADERS "*.h")
#install(FILES ${HEADERS} DESTINATION include/)

