cmake_minimum_required(VERSION 3.0...3.7)
project(Sylar)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -ggdb -std=c++17 -Wall -Wno-deprecated -Werror -Wno-unused-function")
set(CMAKE_CXX_COMPILER_LAUNCHER ccache)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
file(GLOB SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc ${CMAKE_CURRENT_SOURCE_DIR}/include/*.cc)
set(CXX_STANDARD 17)
include_directories(${PROJECT_SOURCE_DIR}/include)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)


# add_library(threadPool SHARED ${SRC})
add_executable(Sylar ${SRC})

target_link_libraries(Sylar pthread)
