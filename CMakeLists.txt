#定义最低版本
cmake_minimum_required(VERSION 3.0)

#定义项目
project(mprpc)

#设置c++标准
set(CMAKE_CXX_STANDARD 20) 

#包含相关头文件目录
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories("/home/xf/install/vcpkg/installed/x64-linux/include/")
link_directories("/home/xf/install/vcpkg/installed/x64-linux/lib/")

#设置可执行程序的输出目录
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin) 
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

#生成库文件
add_subdirectory("./src") 

#进行单元测试
add_subdirectory("./test") 

#编译示例
add_subdirectory(example)