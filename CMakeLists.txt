cmake_minimum_required(VERSION 3.2)

project(delta-limiter LANGUAGES C)

# Default to release builds
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

if(APPLE)
  set(CMAKE_INSTALL_PREFIX $ENV{HOME}/Library/Audio/Plug-Ins/LV2)
elseif(UNIX)
  set(CMAKE_INSTALL_PREFIX /usr/local/lib/lv2)
endif()

include(FindPkgConfig)
pkg_check_modules(LV2 lv2 REQUIRED)
if(MSYS OR MINGW)
  # Ugly but it should work in most cases
  string(REGEX REPLACE "C:" "/c/" LV2_INCLUDE_DIRS ${LV2_INCLUDE_DIRS})
endif()

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.lv2)

add_library(${PROJECT_NAME} SHARED delta-limiter.c)
target_include_directories(${PROJECT_NAME} PRIVATE ${LV2_INCLUDE_DIRS})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /Wall)
else()
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

configure_file(manifest.ttl.in ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/manifest.ttl)
configure_file(delta-limiter.ttl ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/delta-limiter.ttl)

if(APPLE OR UNIX)
  install(DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} DESTINATION ${CMAKE_INSTALL_PREFIX})
endif()
