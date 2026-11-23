cmake_minimum_required(VERSION 3.16)
project(LabTransformacionesAfines2D LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compilador con advertencias estrictas para detectar errores tempranos
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Ejecutable 1: Pruebas numericas y exportacion reproducible (sin dependencias graficas)
add_executable(test_experiments test_experiments.cpp)

# Ejecutable 2: Laboratorio interactivo OpenGL 2.1 + GLFW 3
find_package(OpenGL REQUIRED)
find_package(glfw3 QUIET)
if(TARGET glfw)
    set(GLFW_LINK glfw)
elseif(TARGET glfw3)
    set(GLFW_LINK glfw3)
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GLFW REQUIRED IMPORTED_TARGET glfw3)
    set(GLFW_LINK PkgConfig::GLFW)
endif()

add_executable(lab_afines main.cpp)
target_link_libraries(lab_afines PRIVATE OpenGL::GL ${GLFW_LINK})
