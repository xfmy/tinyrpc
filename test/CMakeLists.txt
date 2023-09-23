project(mprpc_test)
aux_source_directory(. SRC_LIST)

add_executable(mprpc_test ${SRC_LIST})

target_link_libraries(mprpc_test libgtest.a libprotobuf.a)