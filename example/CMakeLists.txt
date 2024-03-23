# include_directories(${PROJECT_SOURCE_DIR}/example)
# add_subdirectory(server)
# add_subdirectory(calller)

add_executable(client ./client.cpp ./protobuf.pb.cc)
add_executable(service ./service.cpp ./protobuf.pb.cc)

target_link_libraries(client mprpc)
target_link_libraries(service mprpc)