cmake_minimum_required(VERSION 3.0...3.7)
project(Sylar)

# 编译选项
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -ggdb -std=c++20 -Wall -Wno-deprecated -Werror -Wno-unused-function")
set(CMAKE_CXX_COMPILER_LAUNCHER ccache)

# 输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# 源文件
file(GLOB SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc ${CMAKE_CURRENT_SOURCE_DIR}/include/*.cc)



# 库和可执行文件
add_library(muqiu0614 SHARED ${SRC})
add_executable(test_http_server ./test/test_http_server.cc)

add_executable(test_fina ./test/test_fina.cc)


# 包含目录
#target_include_directories(test_http_server PRIVATE ${PROJECT_SOURCE_DIR}/include)

# 链接库
target_link_libraries(test_http_server muqiu0614 pthread)

target_link_libraries(test_fina muqiu0614 pthread)

add_executable(muqiu_Default ${SRC} ./test/echo_server.cc)
target_link_libraries(muqiu_Default muqiu0614 pthread)
