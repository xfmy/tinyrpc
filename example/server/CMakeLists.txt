aux_source_directory(. SRC_LIST)
set(CMAKE_BUILD_TYPE Debug)
add_executable(userService ${SRC_LIST} ../protobuf.pb.cc)
target_link_libraries(userService mprpc)