cmake_minimum_required(VERSION 3.7.2)
project(DistortedOutput)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_EXTENSIONS off)

include(CheckCXXCompilerFlag)

function(enable_cxx_compiler_flag_if_supported flag)
    string(FIND "${CMAKE_CXX_FLAGS}" "${flag}" flag_already_set)
    if(flag_already_set EQUAL -1)
        check_cxx_compiler_flag("${flag}" flag_supported)
        if(flag_supported)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}" PARENT_SCOPE)
        endif()
    endif()
endfunction()

enable_cxx_compiler_flag_if_supported(-Wall)
enable_cxx_compiler_flag_if_supported(-Wextra)
enable_cxx_compiler_flag_if_supported(-Wpedantic)
enable_cxx_compiler_flag_if_supported(-Wsuggest-override)

find_package(PkgConfig REQUIRED)

pkg_search_module(FreeImage REQUIRED IMPORTED_TARGET freeimage)
pkg_search_module(GLFW REQUIRED IMPORTED_TARGET glfw3)
pkg_search_module(GLES2 REQUIRED IMPORTED_TARGET glesv2)
pkg_search_module(GLM REQUIRED IMPORTED_TARGET glm)

file(GLOB_RECURSE PROJECT_SOURCES *.cpp)
add_executable(
  ${CMAKE_PROJECT_NAME}
  ${PROJECT_SOURCES})
target_link_libraries(
  ${CMAKE_PROJECT_NAME}
  PRIVATE PkgConfig::FreeImage
  PRIVATE PkgConfig::GLES2
  PRIVATE PkgConfig::GLFW
  PRIVATE PkgConfig::GLM)
