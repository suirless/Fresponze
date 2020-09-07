#*********************************************************************
# Copyright (C) Anton Kovalev (vertver), 2019-2020. All rights reserved.
# Copyright (C) Suirless, 2020. All rights reserved.
# Fresponze - fast, simple and modern multimedia sound library
# Apache-2 License
#*********************************************************************
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#****************************************************************/
cmake_minimum_required(VERSION 3.16)
project(Fresponze C CXX)

include(DownloadProject.cmake)
include(GitUtils.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(RESONANCE_AUDIO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/resonance-audio)
set(R8BRAIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/r8brain-free-src)
set(UPDATE_RESONANCE_AUDIO_DEPS_DIR "${RESONANCE_AUDIO_DIR}/third_party")

# Please, use vcpkg library for Windows builds (MSVC recommended)
option(BUILD_RESONANCE_AUDIO_API "Build Resonance Audio API." ON)
option(BUILD_FRESPONZE_STATIC "Build Fresponze in static mode." OFF)
option(BUILD_STATIC "Build libraries in static mode." ON)

find_package(Ogg CONFIG REQUIRED)
find_package(Opus CONFIG REQUIRED)

# vcpkg 'opusfile' package provide only header and .lib file,
# so we must manually find this include and library
if (WIN32)
    find_path(OPUS_INCLUDE_DIR opus/opusfile.h)
    find_library(OPUSFILE_LIBRARY opusfile REQUIRED)
    message(STATUS "opusfile library: ${OPUSFILE_LIBRARY}")
else()
    find_package(opusfile)
endif()

if (WIN32)
    file(GLOB FRESPONZE_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/windows/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/windows/*.h"
    )
elseif (APPLE)
    file(GLOB FRESPONZE_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/macos/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/macos/*.h"
    )
elseif (UNIX AND NOT APPLE)
    file(GLOB FRESPONZE_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/unix/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/unix/*.h"
    )
endif() 

download_project(
        PROJ           resonance-audio
        GIT_REPOSITORY https://github.com/resonance-audio/resonance-audio.git
        GIT_TAG        master
        SOURCE_DIR     ${RESONANCE_AUDIO_DIR}
        UPDATE_DISCONNECTED 1
)

# Provide custom repository with CMakeLists.txt to make this library
# without any problems
download_project(
        PROJ           r8brain
        GIT_REPOSITORY https://github.com/Vertver/r8brain-free-src.git
        GIT_TAG        master
        SOURCE_DIR     ${R8BRAIN_DIR}
        UPDATE_DISCONNECTED 1
)

# Resonance Audio library require use Shell script to clone core
# libraies. We don't want to do it manually, and that's reason why
# we are cloning repos by yourself
git_clone(
        PROJECT_NAME    eigen
        GIT_URL         https://gitlab.com/libeigen/eigen.git
        GIT_BRANCH      master
        DIRECTORY       ${UPDATE_RESONANCE_AUDIO_DEPS_DIR}
)

git_clone(
        PROJECT_NAME    pffft
        GIT_URL         https://bitbucket.org/jpommier/pffft.git
        GIT_BRANCH      master
        DIRECTORY       ${UPDATE_RESONANCE_AUDIO_DEPS_DIR}
)

git_clone(
        PROJECT_NAME    googletest
        GIT_URL         https://github.com/google/googletest.git
        GIT_BRANCH      master
        DIRECTORY       ${UPDATE_RESONANCE_AUDIO_DEPS_DIR}
)

add_subdirectory(${RESONANCE_AUDIO_DIR})
add_subdirectory(${R8BRAIN_DIR})

if (BUILD_FRESPONZE_STATIC)
    add_library(libfresponze STATIC ${FRESPONZE_SOURCES})
else()
    add_library(libfresponze SHARED ${FRESPONZE_SOURCES})
    target_compile_definitions(libfresponze PUBLIC DLL_PLATFORM)
    target_compile_definitions(libfresponze PUBLIC LIB_EXPORTS)
endif()

target_include_directories(libfresponze PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if (WIN32)
    target_include_directories(libfresponze PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/windows)
    target_include_directories(libfresponze PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/windows)
elseif (APPLE)
    target_include_directories(libfresponze PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/macos)
    target_include_directories(libfresponze PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/macos)
elseif (UNIX AND NOT APPLE)
    target_include_directories(libfresponze PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/unix)
    target_include_directories(libfresponze PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/unix)
endif() 

target_include_directories(libfresponze PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/deps/r8brain-free-src)
target_include_directories(libfresponze PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/deps/resonance-audio/api)
target_include_directories(libfresponze PUBLIC ${OPUS_INCLUDE_DIR}) 
target_include_directories(libfresponze PUBLIC ${OGG_INCLUDE_DIR}) 

if (WIN32)
    target_link_libraries(libfresponze Opus::opus Ogg::ogg ResonanceAudioStatic ${OPUSFILE_LIBRARY} r8brain)
else()
    target_link_libraries(libfresponze opus ogg opusfile ResonanceAudioStatic r8brain)
endif()
