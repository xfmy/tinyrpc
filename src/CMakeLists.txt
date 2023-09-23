#将当前目录下所有的源文件都放入变量SRC_LIST
aux_source_directory(. SRC_LIST)

#由于muduo默认是静态库,所以这里也默认静态(如果需要动态,需要先将muduo编译成动态库)
add_library(mprpc STATIC ${SRC_LIST})

#链接依赖库
target_link_libraries(mprpc protobuf muduo_net muduo_base pthread liblibconfig++.a liblibconfig.a libfmt.a) 