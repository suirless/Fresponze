cmake_minimum_required(VERSION 2.8)
project(opusfile)

option(OPUSFILE_USE_ASSERTIONS "Enable assertions in code" NO)
#option(OPUSFILE_DISABLE_HTPP "Disable HTTP support" YES)
option(OPUSFILE_FIXED_POINT "Enable Fixed Point API" OFF)
option(OPUSFILE_DISABLE_FLOAT "Disable float API" OFF)
option(OPUSFILE_BUILD_EXAMPLES "Enable test examples" YES)
#option(OPUSFILE_ENABLE_DOC "Enables Doxygen generation" NO)

#opus is required to build
add_subdirectory(opus)
#ogg is required to build
add_subdirectory(ogg)

set(OPUSFILE_LIBS
  ${OPUSFILE_LIBS}
  ogg
  opus
  m
)

set(OPUSFILE_INCLUDE_DIR
  opusfile/include
)

set(OPUSFILE_HDR
  opusfile/include/opusfile.h
)

set(OPUSFILE_SRC
  opusfile/src/http.c
  opusfile/src/info.c
  opusfile/src/internal.c
  opusfile/src/internal.h
  opusfile/src/opusfile.c
  opusfile/src/stream.c
  opusfile/src/wincerts.c
  opusfile/src/winerrno.h
)

set(OPUSFILE_EXAMPLE_SRC
  opusfile/examples/win32utf8.c
  opusfile/examples/win32utf8.h
)

set(OPUSFILE_CODE
  ${OPUSFILE_SRC}
  ${OPUSFILE_HDR}
)

if (OPUSFILE_FIXED_POINT)
  add_definitions(-DFIXED_POINT=1 -DDISABLE_FLOAT_API)
endif (OPUSFILE_FIXED_POINT)

# These options affect performance
# HAVE_LRINTF: Use C99 intrinsics to speed up float-to-int conversion
add_definitions(-DHAVE_LRINTF)

if(MINGW)
  set(CMAKE_C_FLAGS
  "${CMAKE_C_FLAGS} -U__STRICT_ANSI__"
  )
endif(MINGW)

if(OPUSFILE_ENABLE_ASSERTIONS)
  add_definitions(-DOP_ENABLE_ASSERTIONS)
endif(OPUSFILE_ENABLE_ASSERTIONS)

#missing http build instructions

if(OPUSFILE_DISABLE_FLOAT)
  add_definitions(-DOP_DISABLE_FLOAT_API)
endif(OPUSFILE_DISABLE_FLOAT)

if(OPUSFILE_FIXED_POINT)
  add_definitions(-DOP_FIXED_POINT)
#TODO check for lib lrintf
#TODO add_definition OP_HAVE_LRINTF
endif(OPUSFILE_FIXED_POINT)

if(OPUSFILE_ENABLE_DOC)
  #TODO check if doxygen on system
find_package(Doxygen)
  if(NOT DOXYGEN_FOUND)
      message(FATAL_ERROR "Doxygen is needed to build the documentation.")
  endif()

  #set(doxyfile_in ${CMAKE_CURRENT_SOURCE_DIR}/opusfile/doc/Doxyfile.in)
  set(doxyfile ${CMAKE_CURRENT_SOURCE_DIR}/opusfile/doc/Doxyfil)
  message("source_dir" ${CMAKE_CURRENT_SOURCE_DIR})

  #configure_file(${doxyfile_in} ${doxyfile} @ONLY)

  add_custom_target(doc
      COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/opusfile
      COMMENT "Generating API documentation with Doxygen"
      VERBATIM)

  install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html DESTINATION share/doc)
endif(OPUSFILE_ENABLE_DOC)


add_library(opusfile STATIC ${OPUSFILE_CODE})
target_link_libraries(opusfile ${OPUSFILE_LIBS})
target_compile_definitions(opusfile PRIVATE
  ${OPUS_BUILD}
)
target_compile_options(opusfile PRIVATE $<$<OR:$<C_COMPILER_ID:Clang>,$<C_COMPILER_ID:GNU>>:
  -std=c89
  -pedantic
  -Wall
  -Wextra
  -Wno-parentheses
  -Wno-long-long
>)
target_include_directories(opusfile PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/opusfile/include
  ${CMAKE_CURRENT_SOURCE_DIR}/opusfile/src
)
target_include_directories(opusfile PUBLIC 
${OPUSFILE_INCLUDE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/opus-cmake/opus/include
  ${CMAKE_CURRENT_SOURCE_DIR}/ogg/include
  ${CMAKE_CURRENT_BINARY_DIR}/ogg/include
  ${CMAKE_CURRENT_SOURCE_DIR}/opusfile/include
)


if (OPUSFILE_ENABLE_EXAMPLES)
  add_executable(opusfile_example ${OPUSFILE_EXAMPLE_SRC} opusfile/examples/opusfile_example.c)
  target_link_libraries(opusfile_example opusfile)
  add_executable(seeking_example ${OPUSFILE_EXAMPLE_SRC} opusfile/examples/seeking_example.c)
  target_link_libraries(seeking_example opusfile)
endif(OPUSFILE_ENABLE_EXAMPLES)
