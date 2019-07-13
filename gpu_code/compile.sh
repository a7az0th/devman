#!/bin/bash

unameOut="$(uname -s)"
case "${unameOut}" in
    Linux*)     machine=Linux;;
    Darwin*)    machine=Mac;;
    CYGWIN*)    machine=Cygwin;;
    MINGW*)     machine=MinGw;;
    *)          machine="UNKNOWN:${unameOut}"
esac

#if [ ! -d "external/shaders" ]
#then
#	mkdir external/shaders
#fi

if [ "${machine}" == "Linux" ]
then
	echo "Setting up variables for Linux"
	export INCLUDES=""
	export NVCC="/usr/local/cuda/bin/nvcc"
	export COMPILER="g++"
else 
	if [ "${machine}" == "MinGw" ]
	then
		echo "Setting up variables for Windows (Git Bash)"

		export INCLUDES=(-I"../include")
		export NVCC="/c/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v10.1/bin/nvcc"
		export COMPILER="/c/Program Files (x86)/Microsoft Visual Studio 14.0/VC/bin/amd64"
	else
		echo "Unsupported OS : ${machine}"
	fi
fi

echo "NVCC compiler currently set: $NVCC"
echo "C++ compiler currently set: $COMPILER"

export NVCC_FLAGS="-m64 --use_fast_math -cudart static --gpu-architecture=compute_30 --gpu-code=sm_30,compute_30 -O0 --maxrregcount=64 "

exec "$NVCC" $NVCC_FLAGS -ccbin "$COMPILER" "${INCLUDES[@]}" -ptx -o greyscale.ptx  greyscale.cu >> cudaoutput.txt | tee
