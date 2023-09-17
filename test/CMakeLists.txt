project(mprpc_test)
aux_source_directory("./" var)

add_executable(mprpc_test ${var})

target_link_libraries(mprpc_test libgtest.a libprotobuf.a)