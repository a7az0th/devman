cmake_minimum_required (VERSION 3.10)
project (devman)

message(STATUS "PROJECT_SOURCE_DIR: " ${PROJECT_SOURCE_DIR})
set(INSTALL_PATH ${PROJECT_SOURCE_DIR}/bin)

function(BUILD_PTX name)
	set(GPU_INCLUDES -I${PROJECT_SOURCE_DIR}/include)

	if (WIN32)
		set(NVCC "$ENV{CUDA_PATH}/bin/nvcc.exe")
	else()
		set(NVCC "/usr/local/cuda/bin/nvcc")
	endif()

	set(NVCC_FLAGS
		-m64
		--use_fast_math
		-cudart static
		--gpu-architecture=compute_50
		--gpu-code=sm_50,compute_50
		-O4
	)
	set(SOURCE_FILE ${PROJECT_SOURCE_DIR}/gpu_code/${name}.cu)
	set(OUTPUT_FILE ${CMAKE_BINARY_DIR}/${name}.ptx)

	set(EXECUTE_COMMAND
		${NVCC}
		${NVCC_FLAGS}
		-ccbin ${CMAKE_CXX_COMPILER}
		${GPU_INCLUDES}
		-ptx -o ${OUTPUT_FILE} ${SOURCE_FILE}
	)

	add_custom_target(${name} ALL DEPENDS ${OUTPUT_FILE})

	add_custom_command(
		COMMAND    ${EXECUTE_COMMAND}
		DEPENDS    ${SOURCE_FILE}
		OUTPUT     ${OUTPUT_FILE}
	)
	install(FILES ${OUTPUT_FILE} DESTINATION ${INSTALL_PATH})
endfunction(BUILD_PTX)

include_directories(
	${PROJECT_SOURCE_DIR}/gpu_code
	${PROJECT_SOURCE_DIR}/include
	${PROJECT_SOURCE_DIR}/cuew
)

if (WIN32)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

build_ptx(kernel)
build_ptx(greyscale)

set(HEADERS
	include/utils.h
	include/devman.h
	include/nvml.h
	include/version.h
)

set(SOURCES
	src/main.cpp
	src/devman.cpp
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
if (UNIX)
	target_link_libraries(${PROJECT_NAME} pthread)
endif()

install(TARGETS ${PROJECT_NAME} DESTINATION ${INSTALL_PATH})
