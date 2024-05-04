cmake_minimum_required(VERSION 3.5)

add_executable(AsyncReader AsyncReader.cpp)
target_link_libraries(AsyncReader AioWrapperLib)
