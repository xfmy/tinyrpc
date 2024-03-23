#将当前目录下所有的源文件都放入变量SRC_LIST
aux_source_directory(. SRC_LIST)
aux_source_directory(./tinypb TINYPB_LIST)
aux_source_directory(./tools TOOLS_LIST)
aux_source_directory(./net NET_LIST)
aux_source_directory(./consul CONSUL_LIST)

# 设置项目编译头文件搜索路径 -I
include_directories(${PROJECT_SOURCE_DIR}/src/tinypb)
include_directories(${PROJECT_SOURCE_DIR}/src/tools)
include_directories(${PROJECT_SOURCE_DIR}/src/net)
include_directories(${PROJECT_SOURCE_DIR}/src/consul)


#由于muduo默认是静态库,所以这里也默认静态(如果需要动态,需要先将muduo编译成动态库)
add_library(mprpc STATIC ${SRC_LIST} ${TINYPB_LIST} ${NET_LIST} ${CONSUL_LIST} ${TOOLS_LIST})

#链接依赖库
target_link_libraries(mprpc protobuf muduo_net muduo_base pthread liblibconfig++.a liblibconfig.a libfmt.a ppconsul curl json11)  