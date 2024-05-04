cmake_minimum_required(VERSION 3.5)

set(PROJECT_NAME AioWrapper)
project(${PROJECT_NAME})

set(LIBRARY_NAME "${PROJECT_NAME}Lib")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

if (CMAKE_BUILD_TYPE MATCHES Debug)
    set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
elseif(CMAKE_BUILD_TYPE MATCHES Release)
    set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall -Werror -DNDEBUG")
endif()

aux_source_directory(${PROJECT_SOURCE_DIR}/src SRC_LIST)

set(SOURCE_LIST
    ${SRC_LIST})

set(INCLUDE_LIST ${PROJECT_SOURCE_DIR}/include)

add_subdirectory(dependencies)

add_library(${LIBRARY_NAME} STATIC ${SOURCE_LIST})
target_include_directories(${LIBRARY_NAME} PUBLIC ${INCLUDE_LIST})
target_link_libraries(${LIBRARY_NAME} pthread)
target_link_libraries(${LIBRARY_NAME} fmt)
install(TARGETS ${LIBRARY_NAME} DESTINATION lib)

add_subdirectory(examples)
