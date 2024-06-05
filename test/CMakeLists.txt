project(tinyrpc_test)
#aux_source_directory(. SRC_LIST)

add_executable(tinyrpc_test tinypb_test.cpp)

target_link_libraries(tinyrpc_test tinyrpc)