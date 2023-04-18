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
        event_loop_thread_pool.cpp
        acceptor.cpp
        wakeup_channel.cpp
    )


add_library(yolanda ${yolanda_lib})
target_link_libraries(yolanda pthread rt)

install(TARGETS yolanda DESTINATION lib)

#file(GLOB HEADERS "*.h")
#install(FILES ${HEADERS} DESTINATION include/)

