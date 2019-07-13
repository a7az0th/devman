cmake_minimum_required (VERSION 2.6)
project (utils)

message(STATUS "PROJECT_SOURCE_DIR: " ${PROJECT_SOURCE_DIR})

include_directories(
	${PROJECT_SOURCE_DIR}/include
)

#_CRT_SECURE_NO_WARNINGS

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(HEADERS
	include/charstring.h
	include/error_code.h
	include/progress.h
	include/table.h
	include/timer.h
	include/threadman.h
	include/color.h
)

set(SOURCES
	src/progress.cpp
	src/color.cpp
)

add_library(${PROJECT_NAME} STATIC ${HEADERS} ${SOURCES})

set(UTILS_HEADERS ${HEADERS})

