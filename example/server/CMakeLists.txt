aux_source_directory(. SRC_LIST)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)
add_executable(userService ${SRC_LIST})
target_link_libraries(userService mprpc)