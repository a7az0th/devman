cmake_minimum_required (VERSION 2.6)
project (devman)

message(STATUS "PROJECT_SOURCE_DIR: " ${PROJECT_SOURCE_DIR})

include_directories(
	${PROJECT_SOURCE_DIR}/include
	${PROJECT_SOURCE_DIR}/cuew
	${PROJECT_SOURCE_DIR}/sdk/FreeImage/include
)

link_directories (
	${PROJECT_SOURCE_DIR}/sdk/FreeImage/lib
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(HEADERS
	include/utils.h
	include/devman.h
	include/image.h
)

set(SOURCES
	src/main.cpp
	src/devman.cpp
	src/image.cpp
	cuew/cuew.c
)

set(GPU_SOURCE
	gpu_code/kernel.cu
	gpu_code/greyscale.cu
)

add_subdirectory(utils)

include_directories(${utils_SOURCE_DIR}/include)

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES} ${GPU_SOURCE})
source_group("GPU Code" FILES ${GPU_SOURCE})

target_link_libraries(${PROJECT_NAME} ${CMAKE_DL_LIBS})
target_link_libraries(${PROJECT_NAME} utils)
target_link_libraries(${PROJECT_NAME} FreeImage)

#install(TARGETS devman DESTINATION bin)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECT_SOURCE_DIR}/sdk/FreeImage/bin/FreeImage.dll"
         $<TARGET_FILE_DIR:${PROJECT_NAME}>)