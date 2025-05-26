cmake_minimum_required(VERSION 3.10)
project(TinyWebServer)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 源码文件收集
aux_source_directory(${CMAKE_SOURCE_DIR}/code/buffer BUFFER_SRC)
aux_source_directory(${CMAKE_SOURCE_DIR}/code/http HTTP_SRC)
aux_source_directory(${CMAKE_SOURCE_DIR}/code/log LOG_SRC)
aux_source_directory(${CMAKE_SOURCE_DIR}/code/pool POOL_SRC)
aux_source_directory(${CMAKE_SOURCE_DIR}/code/server SERVER_SRC)
aux_source_directory(${CMAKE_SOURCE_DIR}/code/timer TIMER_SRC)

set(SRC_FILES
    ${BUFFER_SRC}
    ${HTTP_SRC}
    ${LOG_SRC}
    ${POOL_SRC}
    ${SERVER_SRC}
    ${TIMER_SRC}
    ${CMAKE_SOURCE_DIR}/code/main.cpp
)

# 头文件目录
include_directories(
    ${CMAKE_SOURCE_DIR}/code
    ${CMAKE_SOURCE_DIR}/code/buffer
    ${CMAKE_SOURCE_DIR}/code/http
    ${CMAKE_SOURCE_DIR}/code/log
    ${CMAKE_SOURCE_DIR}/code/pool
    ${CMAKE_SOURCE_DIR}/code/server
    ${CMAKE_SOURCE_DIR}/code/timer
)

# MySQL
find_path(MYSQL_INCLUDE_DIR mysql/mysql.h)
find_library(MYSQL_LIB mysqlclient)

if(NOT MYSQL_INCLUDE_DIR OR NOT MYSQL_LIB)
    message(FATAL_ERROR "MySQL client library or headers not found. Please install libmysqlclient-dev.")
endif()

include_directories(${MYSQL_INCLUDE_DIR})

add_executable(server ${SRC_FILES})

target_link_libraries(server pthread ${MYSQL_LIB})

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
