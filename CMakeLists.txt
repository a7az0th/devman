cmake_minimum_required (VERSION 2.6)
project (devman)

message(STATUS "PROJECT_SOURCE_DIR: " ${PROJECT_SOURCE_DIR})

include_directories(
	${PROJECT_SOURCE_DIR}/include
	${PROJECT_SOURCE_DIR}/cuew
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(HEADERS
	include/utils.h
	include/devman.h
)

set(SOURCES
	src/main.cpp
	src/devman.cpp
	cuew/cuew.c
)

add_subdirectory(utils)

include_directories(${utils_SOURCE_DIR}/include)

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES})

target_link_libraries(${PROJECT_NAME} ${CMAKE_DL_LIBS})
target_link_libraries(${PROJECT_NAME} utils)

#install(TARGETS myapp DESTINATION bin)