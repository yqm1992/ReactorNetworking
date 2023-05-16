#if (EPOLL_EXISTS)
#    set(networking_lib
#            channel.cpp
#            )
#else ()
#    set(networking_lib
#            channel.cpp
#            )
#endif ()
set(networking_lib
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
        tcp_client.cpp
        echo_service.cpp
    )


add_library(networking ${networking_lib})
target_link_libraries(networking pthread rt)

install(TARGETS networking DESTINATION lib)

#file(GLOB HEADERS "*.h")
#install(FILES ${HEADERS} DESTINATION include/)

