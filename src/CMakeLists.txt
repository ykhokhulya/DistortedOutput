file(GLOB_RECURSE PROJECT_SOURCES *.cpp)
add_executable(
  ${CMAKE_PROJECT_NAME}
  ${PROJECT_SOURCES})
target_include_directories(
  ${CMAKE_PROJECT_NAME}
  PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(
  ${CMAKE_PROJECT_NAME}
  PRIVATE PkgConfig::FreeImage
  PRIVATE PkgConfig::GLES2
  PRIVATE PkgConfig::GLFW
  PRIVATE PkgConfig::GLM)
