aux_source_directory(. SRC_LIST)
set(CMAKE_BUILD_TYPE Debug)
add_executable(calller ${SRC_LIST} ../protobuf.pb.cc)
target_link_libraries(calller mprpc)