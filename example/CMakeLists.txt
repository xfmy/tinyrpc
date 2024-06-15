# 生成client and server示例程序
add_executable(client ./client.cpp ./protobuf.pb.cc)
add_executable(service ./service.cpp ./protobuf.pb.cc)

# 链接tinyrpc库文件
target_link_libraries(client tinyrpc)
target_link_libraries(service tinyrpc)
