cmake_minimum_required(VERSION 2.8.12)

if(USE_EXTERNAL_LIBS)
# http://www.cmake.org/Wiki/CMake/Tutorials/Exporting_and_Importing_Targets
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++1y")

if (CMAKE_COMPILER_IS_GNUCC)
  execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion
    OUTPUT_VARIABLE GCC_VERSION)

  if (GCC_VERSION VERSION_GREATER 5.1)
    add_definitions("-D_GLIBCXX_USE_CXX11_ABI=1")
  endif()
endif()

set(CMAKE_BUILD_TYPE Release)
set(EXPORTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/exports/${PLATFORM_TARGET})
endif()


## yaml-cpp ##
##############
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "")
set(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "")
set(YAML_CPP_INSTALL OFF CACHE BOOL "")
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/yaml-cpp)

target_include_directories(yaml-cpp
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/yaml-cpp/include
)

## css-color-parser-cpp ##
##########################
add_library(css-color-parser-cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/css-color-parser-cpp/csscolorparser.cpp)

target_include_directories(css-color-parser-cpp
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/css-color-parser-cpp)


## geojsonvt ##
###############
file(GLOB_RECURSE GEOJSON_VT_CPP_SOURCES "geojson-vt-cpp/src/*.cpp")
add_library(geojson-vt-cpp ${GEOJSON_VT_CPP_SOURCES})
target_include_directories(geojson-vt-cpp
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/geojson-vt-cpp/include
  # use our variant and geojsonvt_types.hpp
  # instead of the provided ones
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../core/src
  ${CMAKE_CURRENT_SOURCE_DIR}/../core/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../core/include/mapbox)

set_target_properties(geojson-vt-cpp
  PROPERTIES COMPILE_FLAGS -DGEOJSONVT_CUSTOM_TAGS)

if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(geojson-vt-cpp
      PUBLIC
      -D_USE_MATH_DEFINES)
else()
    target_compile_options(geojson-vt-cpp
      PUBLIC
      -Wno-unknown-pragmas)
endif()


## duktape ##
#############
add_library(duktape
  ${CMAKE_CURRENT_SOURCE_DIR}/duktape/duktape.c)

target_compile_options(duktape PRIVATE
  -fstrict-aliasing
  -fomit-frame-pointer
  -std=c99
  -Wall)

target_include_directories(duktape
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/duktape)


## cmake exports ##
###################
if(USE_EXTERNAL_LIBS)
  export(TARGETS
    yaml-cpp
    css-color-parser-cpp
    geojson-vt-cpp
    duktape
    FILE ${EXPORTS_DIR}/core-dependencies.cmake)
endif()


## GLFW ##
##########
if(PLATFORM_LINUX OR PLATFORM_OSX OR PLATFORM_WINDOWS)
  if (USE_SYSTEM_GLFW_LIBS)
    include(FindPkgConfig)
    pkg_check_modules(GLFW REQUIRED glfw3)
  else()
    # configure GLFW to build only the library
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build the GLFW example programs")
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "Build the GLFW test programs")
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "Build the GLFW documentation")
    set(GLFW_INSTALL OFF CACHE BOOL "Generate installation target")
    if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
      set(GLFW_USE_EGL ON CACHE BOOL "Generate installation target")
    endif()
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/glfw)

    target_include_directories(glfw
      PUBLIC
      ${CMAKE_CURRENT_SOURCE_DIR}/glfw/include)

    if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
      target_link_libraries(glfw
        INTERFACE
        "${GLFW_LIBRARIES}")
      target_compile_options(glfw
        PRIVATE
        -DNDEBUG)
    else()
      target_link_libraries(glfw
        INTERFACE
        "${GLFW_LIBRARIES}"
        -ldl)
      target_compile_options(glfw
        PRIVATE
        -O3 -DNDEBUG)
    endif()

    if(USE_EXTERNAL_LIBS)
      export(TARGETS glfw
        FILE ${EXPORTS_DIR}/glfw.cmake
        EXPORT_LINK_INTERFACE_LIBRARIES)
    endif()
  endif()
endif()


## benchmark ##
###############
if((BENCHMARK OR USE_EXTERNAL_LIBS) AND (PLATFORM_LINUX OR PLATFORM_OSX))
  set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "")
  set(BENCHMARK_ENABLE_LTO OFF CACHE BOOL "")

  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/benchmark)
  target_compile_options(benchmark
    PRIVATE
    -O3 -DNDEBUG)

  if(USE_EXTERNAL_LIBS)
    export(TARGETS benchmark
      FILE ${EXPORTS_DIR}/benchmark.cmake)
  endif()
endif()


if(NOT USE_SYSTEM_FONT_LIBS)
  ## Harfbuzz - ICU-Common - UCDN - Freetype2 ##
  ##############################################
  set(HARFBUZZ_BUILD_ICU ON CACHE BOOL "Enable building of ICU")
  set(HARFBUZZ_BUILD_UCDN ON CACHE BOOL "Enable building of UCDN")
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/harfbuzz-icu-freetype)

  message(STATUS "harfbuzz" ${HARFBUZZ_LIBRARIES})

  set(ALFONS_DEPS_LIBRARIES
    ${ALFONS_DEPS_LIBRARIES}
    harfbuzz ${HARFBUZZ_LIBRARIES}
    CACHE INTERNAL "alfons-libs" FORCE)
endif()

## alfons ##
############
#set(GLM_ROOT ${PROJECT_SOURCE_DIR}/core/include/glm)
set(GLM_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../core/include/glm)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/alfons)
