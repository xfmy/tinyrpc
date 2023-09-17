aux_source_directory(. SRC_LIST)

add_library(libmprpc STATIC ${SRC_LIST})

#target_link_libraries(chatServer muduo_net muduo_base pthread mysqlcppconn liblibconfig++.a liblibconfig.a libfmtd.a)