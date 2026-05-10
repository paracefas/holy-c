cmake_minimum_required(VERSION 3.15)
project(parser)

set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")

# 1. Rutas
set(LLVM_ROOT "C:/LLVM/llvm")
set(VS_INSIDER_DIA "C:/Program Files/Microsoft Visual Studio/18/Insiders/DIA SDK/lib/amd64/diaguids.lib")

# 2. Buscar LLVM
find_package(LLVM REQUIRED CONFIG PATHS "${LLVM_ROOT}" NO_DEFAULT_PATH)
include_directories(SYSTEM BEFORE "${LLVM_ROOT}/include")
add_definitions(${LLVM_DEFINITIONS})

# 3. Librerías
llvm_map_components_to_libnames(llvm_libs core support target native codegen mc all-targets)

# 4. Ejecutable
add_executable(holy-c main.cpp ast.cpp lexer.cpp parser.cpp util.cpp)

# 5. Linkeo (Pasamos las librerías como lista de strings, no como targets, para saltarnos sus dependencias internas)
target_link_libraries(holy-c PRIVATE 
    ${llvm_libs}
    "${VS_INSIDER_DIA}"
    ntdll.lib
    psapi.lib
    shell32.lib
    advapi32.lib
)

# 6. Forzar al linker a ignorar la ruta maldita
set_target_properties(holy-c PROPERTIES 
    LINK_FLAGS "/NODEFAULTLIB:diaguids.lib /IGNORE:4099"
)