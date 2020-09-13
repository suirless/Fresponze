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
set(OPUSFILE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/opusfile)
set(GLFW_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/glfw)
set(UPDATE_RESONANCE_AUDIO_DEPS_DIR "${RESONANCE_AUDIO_DIR}/third_party")
set(BUILD_SHARED_LIBS ON)

# Please, use vcpkg library for Windows builds (MSVC recommended)
option(BUILD_RESONANCE_AUDIO_API "Build Resonance Audio API." ON)
option(BUILD_FRESPONZE_STATIC "Build Fresponze in static mode." OFF)
option(BUILD_EXAMPLES "Build examples for Fresponze library." ON)
option(USE_VCPKG "Use vcpkg instead of source packages" OFF)

if (WIN32)
    option(BUILD_STATIC "Build libraries in static mode." ON)
else()
    option(BUILD_STATIC "Build libraries in static mode." OFF)
endif()

if (USE_VCPKG)
    find_package(Ogg CONFIG REQUIRED)
    find_package(Opus CONFIG REQUIRED)
else()
    if (UNIX AND NOT APPLE)
        find_package(OpenGL REQUIRED)
        find_package(GLEW REQUIRED)
        if (GLEW_FOUND)
            include_directories(${GLEW_INCLUDE_DIRS})
            link_libraries(${GLEW_LIBRARIES})
        endif()

        find_package(ALSA REQUIRED)
        if (ALSA_FOUND)
            include_directories(${ALSA_INCLUDE_DIRS})
        endif(ALSA_FOUND)

        download_project(
            PROJ           glfw
            GIT_REPOSITORY https://github.com/glfw/glfw.git
            GIT_TAG        master
            SOURCE_DIR     ${GLFW_DIR}
            UPDATE_DISCONNECTED 1
        )

        add_subdirectory(${GLFW_DIR})
    endif()

    git_clone(
            PROJECT_NAME    opus
            GIT_URL         https://github.com/xiph/opus.git
            GIT_BRANCH      master
            DIRECTORY       ${OPUSFILE_DIR}
    )

    git_clone(
            PROJECT_NAME    ogg
            GIT_URL         https://github.com/xiph/ogg.git
            GIT_BRANCH      master
            DIRECTORY       ${OPUSFILE_DIR}
    )

    git_clone(
            PROJECT_NAME    opusfile
            GIT_URL         https://github.com/xiph/opusfile.git
            GIT_BRANCH      master
            DIRECTORY       ${OPUSFILE_DIR}
    )

    set(OPUS_INCLUDE_DIR ${OPUSFILE_DIR}/opus/include)
    set(OGG_INCLUDE_DIR ${OPUSFILE_DIR}/ogg/include)
    add_subdirectory(${OPUSFILE_DIR})
endif()

if (WIN32)
    set(FRESPONZE_LIBRARY_NAME libfresponze)
else()
    set(FRESPONZE_LIBRARY_NAME fresponze)
endif()

# vcpkg 'opusfile' package provide only header and .lib file,
# so we must manually find this include and library
if (USE_VCPKG)
    find_path(OPUS_INCLUDE_DIR opus/opusfile.h)
    find_library(OPUSFILE_LIBRARY opusfile REQUIRED)
    message(STATUS "opusfile library: ${OPUSFILE_LIBRARY}")
endif()

if (WIN32)
    file(GLOB FRESPONZE_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/windows/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/windows/*.h"
    )
elseif (APPLE AND UNIX)
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
        GIT_URL         https://github.com/suirless/libeigen.git
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
    add_library(${FRESPONZE_LIBRARY_NAME} STATIC ${FRESPONZE_SOURCES})
else()
    add_library(${FRESPONZE_LIBRARY_NAME} SHARED ${FRESPONZE_SOURCES})
    target_compile_definitions(${FRESPONZE_LIBRARY_NAME} PUBLIC DLL_PLATFORM)
    target_compile_definitions(${FRESPONZE_LIBRARY_NAME} PUBLIC LIB_EXPORTS)
endif()

target_include_directories(${FRESPONZE_LIBRARY_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if (WIN32)
    target_include_directories(${FRESPONZE_LIBRARY_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/windows)
    target_include_directories(${FRESPONZE_LIBRARY_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/windows)
    target_compile_definitions(${FRESPONZE_LIBRARY_NAME} PUBLIC WINDOWS_PLATFORM)
elseif (APPLE)
    target_include_directories(${FRESPONZE_LIBRARY_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/macos)
    target_include_directories(${FRESPONZE_LIBRARY_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/macos)
    target_compile_definitions(${FRESPONZE_LIBRARY_NAME} PUBLIC MACOS_PLATFORM)
elseif (UNIX AND NOT APPLE)
    target_include_directories(${FRESPONZE_LIBRARY_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/unix)
    target_include_directories(${FRESPONZE_LIBRARY_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/unix)
    target_compile_definitions(${FRESPONZE_LIBRARY_NAME} PUBLIC LINUX_PLATFORM)
endif() 

target_include_directories(${FRESPONZE_LIBRARY_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/deps/r8brain-free-src)
target_include_directories(${FRESPONZE_LIBRARY_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/deps/resonance-audio/resonance_audio/api)
target_include_directories(${FRESPONZE_LIBRARY_NAME} PUBLIC ${OPUS_INCLUDE_DIR})
target_include_directories(${FRESPONZE_LIBRARY_NAME} PUBLIC ${OGG_INCLUDE_DIR})

if (WIN32)
    if (USE_VCPKG)
        target_link_libraries(${FRESPONZE_LIBRARY_NAME} Opus::opus Ogg::ogg ResonanceAudioStatic ${OPUSFILE_LIBRARY} r8brain)
    else()
        target_link_libraries(${FRESPONZE_LIBRARY_NAME} opusfile ResonanceAudioStatic r8brain)
    endif()
elseif (UNIX AND NOT APPLE)
    target_link_libraries(${FRESPONZE_LIBRARY_NAME} opusfile ResonanceAudioStatic r8brain pthread ${ALSA_LIBRARIES})
endif()

if (BUILD_EXAMPLES)
    file(GLOB CUSTOM_CALLBACK_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/examples/custom_callback/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/examples/custom_callback/*.h"
    )

    file(GLOB DEVICE_MANAGMENT_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/examples/device_managment/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/examples/device_managment/*.h"
    )

    add_executable(device_managment ${DEVICE_MANAGMENT_SOURCES})
    add_executable(custom_callback ${CUSTOM_CALLBACK_SOURCES})

    download_project(
            PROJ           imgui
            GIT_REPOSITORY https://github.com/suirless/imgui.git
            GIT_TAG        master
            SOURCE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/deps/imgui
            UPDATE_DISCONNECTED 1
    )

    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/deps/imgui)

    if (WIN32)
        target_link_libraries(device_managment imgui ${FRESPONZE_LIBRARY_NAME} d3d11.lib)
        target_link_libraries(custom_callback imgui ${FRESPONZE_LIBRARY_NAME} d3d11.lib)
        if (MSVC)
            set_property(TARGET device_managment PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/examples/device_managment")
            set_property(TARGET device_managment PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/examples/custom_callback")
        endif()
    else()
        target_link_libraries(device_managment imgui ${FRESPONZE_LIBRARY_NAME} OpenGL::GL glfw pthread GLEW::GLEW)
        target_link_libraries(custom_callback imgui ${FRESPONZE_LIBRARY_NAME} OpenGL::GL glfw pthread GLEW::GLEW)
    endif()
endif()
