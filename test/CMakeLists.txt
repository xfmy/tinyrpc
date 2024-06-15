#生成可执行程序
add_executable(tinyrpc_test tinypb_test.cpp)

#连接库文件
target_link_libraries(tinyrpc_test tinyrpc)