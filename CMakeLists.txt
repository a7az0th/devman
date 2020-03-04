cmake_minimum_required (VERSION 2.6)
project (devman)

message(STATUS "PROJECT_SOURCE_DIR: " ${PROJECT_SOURCE_DIR})

include_directories(
	${PROJECT_SOURCE_DIR}/include
	${PROJECT_SOURCE_DIR}/cuew
)

if (WIN32)
	include_directories(
		${PROJECT_SOURCE_DIR}/sdk/FreeImage/include
	)
	link_directories (
		${PROJECT_SOURCE_DIR}/sdk/FreeImage/lib
	)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
	include_directories(
		/media/alexander/LinuxData/FreeImage/Source
	)
	link_directories(
		/media/alexander/LinuxData/FreeImage/Dist
	)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -o0")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(HEADERS
	include/utils.h
	include/devman.h
	include/image.h
	include/nvml.h
	include/version.h
)

set(SOURCES
	src/main.cpp
	src/devman.cpp
	src/image.cpp
	src/nvml.cpp
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
target_link_libraries(${PROJECT_NAME} freeimage)
if (UNIX)
	target_link_libraries(${PROJECT_NAME} pthread)
endif()

#install(TARGETS devman DESTINATION bin)

if (WIN32)
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"${PROJECT_SOURCE_DIR}/sdk/FreeImage/bin/FreeImage.dll"
			$<TARGET_FILE_DIR:${PROJECT_NAME}>)
endif()
