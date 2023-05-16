add_executable(http_server http_server.cpp)
target_link_libraries(http_server networking)

add_executable(echo_client echo_client.cpp)
target_link_libraries(echo_client networking)

add_executable(echo_server echo_server.cpp)
target_link_libraries(echo_server networking)