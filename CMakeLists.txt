cmake_minimum_required (VERSION 2.6)
project (devman)

MESSAGE(STATUS "PROJECT_SOURCE_DIR: " ${PROJECT_SOURCE_DIR})

include_directories(
	${PROJECT_SOURCE_DIR}
	${PROJECT_SOURCE_DIR}/cuew
)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

SET(SOURCES
	main.cpp
	devman.cpp
	cuew/cuew.c
)

add_executable(devman.bin ${SOURCES})

target_link_libraries(devman.bin ${CMAKE_DL_LIBS})
