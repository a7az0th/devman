
setlocal

@REM Obtain the current directory without trailing backslash
@set HERE=%~dp0
@set HERE=%HERE:~0,-1%
@cd %HERE%

@REM Name the input arguments
@set SUFFIX=ptx
@set CC=50

@if exist kernel.%SUFFIX% @del kernel.%SUFFIX%

@set HOST_COMPILER="C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\bin\cl.exe"
 rem 
@set  COMPILER_OPTIONS= --default-stream per-thread -ccbin %HOST_COMPILER% -gencode arch=compute_%CC%,code=sm_%CC% -use_fast_math -O0 -lineinfo --maxrregcount 64 --machine 64 -v
@echo COMPILER OPTIONS:
@echo %COMPILER_OPTIONS%
@echo ----------------------------------------------------
@set  LINKER_OPTIONS= -%SUFFIX%
@echo LINKER_OPTIONS:
@echo %LINKER_OPTIONS%
@echo ----------------------------------------------------
@set  INCLUDES=-I"../include"
@echo ----------------------------------------------------

@echo NVCC Compiling kernel.cu for Compute Capability %CC% x64
@"C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v9.0\bin\nvcc.exe" %COMPILER_OPTIONS% %LINKER_OPTIONS% %INCLUDES% kernel.cu -o kernel.%SUFFIX% > cudaoutput.txt 2>&1

@REM Sort out warnings and errors in separate files
@findstr /C:"warning:" cudaoutput.txt > CUDAwarnings.txt
@findstr /C:"error:" cudaoutput.txt > CUDAerrors.txt

endlocal

@PAUSE
@exit 0