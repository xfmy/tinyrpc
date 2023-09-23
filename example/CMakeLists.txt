aux_source_directory(. SRC_LIST)
add_executable(userService ${SRC_LIST})
target_link_libraries(userService mprpc)