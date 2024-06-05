# include_directories(${PROJECT_SOURCE_DIR}/example)
# add_subdirectory(server)
# add_subdirectory(calller)

add_executable(client ./client.cpp ./protobuf.pb.cc)
add_executable(service ./service.cpp ./protobuf.pb.cc)
#add_executable(http_service ./http_server.cpp ./protobuf.pb.cc)

target_link_libraries(client tinyrpc)
target_link_libraries(service tinyrpc)
#target_link_libraries(http_service tinyrpc)