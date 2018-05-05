cmake_minimum_required (VERSION 2.6)
project (devman)


include_directories(
	/Users/a7az0th/code/devman
	/Users/a7az0th/code/devman/cuew
)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")


SET(SOURCES
	main.cpp
	devman.cpp
	cuew/cuew.c
)

add_executable(devman.bin ${SOURCES})

