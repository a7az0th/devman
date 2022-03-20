cmake_minimum_required (VERSION 3.10)
project (utils)

message(STATUS "PROJECT_SOURCE_DIR: " ${PROJECT_SOURCE_DIR})

include_directories(
	${PROJECT_SOURCE_DIR}/include
)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(HEADERS
	include/charstring.h
	include/error_code.h
	include/progress.h
	include/table.h
	include/timer.h
	include/threadman.h
	include/color.h
	include/image.h
)

set(SOURCES
	src/progress.cpp
	src/color.cpp
	src/image.cpp
)

add_library(${PROJECT_NAME} STATIC ${HEADERS} ${SOURCES})

set(UTILS_HEADERS ${HEADERS})

