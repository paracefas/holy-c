cmake_minimum_required(VERSION 3.15)
project(parser)

set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build" FORCE)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")

if(MSVC OR (WIN32 AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang"))
    add_compile_definitions(_ITERATOR_DEBUG_LEVEL=0)
    add_compile_definitions(_HAS_ITERATOR_DEBUGGING=0)
endif()

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(LLVM_ROOT "C:/LLVM/llvm") 
set(LLVM_LIB_DIR "${LLVM_ROOT}/lib")
include_directories(SYSTEM BEFORE "${LLVM_ROOT}/include")

include_directories(SYSTEM "C:/Program Files (x86)/Windows Kits/10/Include/10.0.26100.0/ucrt")
include_directories(SYSTEM "C:/Program Files/Microsoft Visual Studio/18/Insiders/VC/Tools/MSVC/14.51.36231/include")

add_executable(holy-c main.cpp ast.cpp lexer.cpp parser.cpp util.cpp)

# 3. Librerías
link_directories("${LLVM_ROOT}/lib")
target_link_libraries(holy-c PRIVATE 
    "${LLVM_LIB_DIR}/LLVMRemarks.lib"
    "${LLVM_LIB_DIR}/LLVMCore.lib"
    "${LLVM_LIB_DIR}/LLVMBitReader.lib"
    "${LLVM_LIB_DIR}/LLVMBitstreamReader.lib"
    "${LLVM_LIB_DIR}/LLVMBinaryFormat.lib"
    "${LLVM_LIB_DIR}/LLVMTargetParser.lib"
    "${LLVM_LIB_DIR}/LLVMSupport.lib"
    "${LLVM_LIB_DIR}/LLVMDemangle.lib"
    ntdll.lib
    psapi.lib
    shell32.lib
    advapi32.lib
    libcmt.lib
)
message(STATUS "Usando librerías desde: ${LLVM_LIB_DIR}")