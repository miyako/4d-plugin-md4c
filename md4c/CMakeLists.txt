cmake_minimum_required(VERSION 3.20)

set(PLUGIN_NAME "md4c" CACHE STRING "Plugin name")
project(${PLUGIN_NAME} LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 17)

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/../VERSION" PLUGIN_VERSION)
string(STRIP "${PLUGIN_VERSION}" PLUGIN_VERSION)
if(NOT PLUGIN_VERSION MATCHES "^(0|[1-9][0-9]*)\\.(0|[1-9][0-9]*)\\.(0|[1-9][0-9]*)$")
    message(FATAL_ERROR "VERSION must be a numeric semantic version")
endif()
set(SDK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../4D-Plugin-SDK/4D Plugin API")
set(MD4C_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../md4c-src/src")
if(WIN32)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# --- md4c: compiled into the plugin from the pinned md4c-src submodule (UTF-8 build) --------

set(MD4C_SOURCES "${MD4C_DIR}/md4c.c" "${MD4C_DIR}/md4c-html.c" "${MD4C_DIR}/entity.c")
add_library(md4c_static STATIC ${MD4C_SOURCES})
target_include_directories(md4c_static PUBLIC "${MD4C_DIR}")
target_compile_definitions(md4c_static PUBLIC MD4C_USE_UTF8)
set_target_properties(md4c_static PROPERTIES POSITION_INDEPENDENT_CODE ON C_VISIBILITY_PRESET hidden)
if(MSVC)
    target_compile_definitions(md4c_static PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# --- plugin --------------------------------------------------------------------------------

set(PLUGIN_SOURCES ${PLUGIN_NAME}-4dplugin.cpp "${SDK_DIR}/4DPluginAPI.c")
set(PLUGIN_HEADERS ${PLUGIN_NAME}-4dplugin.h 4d-helpers.h
    "${SDK_DIR}/4DPluginAPI.h" "${SDK_DIR}/EntryPoints.h"
    "${SDK_DIR}/PublicTypes.h" "${SDK_DIR}/PrivateTypes.h" "${SDK_DIR}/Flags.h")
set(PLUGIN_RESOURCES manifest.json constants.xlf)

if(APPLE)
    add_library(${PLUGIN_NAME} MODULE ${PLUGIN_SOURCES} ${PLUGIN_HEADERS} ${PLUGIN_RESOURCES})
    set_target_properties(${PLUGIN_NAME} PROPERTIES
        BUNDLE TRUE
        BUNDLE_EXTENSION bundle
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.4d.${PLUGIN_NAME}"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PLUGIN_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PLUGIN_VERSION}"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${PLUGIN_NAME}-test/Plugins"
        LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/${PLUGIN_NAME}-test/Plugins"
        LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/${PLUGIN_NAME}-test/Plugins")
    set_source_files_properties(${PLUGIN_RESOURCES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    find_library(COREGRAPHICS_FRAMEWORK CoreGraphics REQUIRED)
    target_link_libraries(${PLUGIN_NAME} PRIVATE "${COREGRAPHICS_FRAMEWORK}" md4c_static)
    set_source_files_properties("${SDK_DIR}/4DPluginAPI.c" PROPERTIES
        COMPILE_FLAGS "-Wno-int-conversion -Wno-multichar")
elseif(WIN32)
    add_library(${PLUGIN_NAME} SHARED ${PLUGIN_SOURCES} ${PLUGIN_HEADERS})
    set(PLUGIN_OUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${PLUGIN_NAME}-test/Plugins/${PLUGIN_NAME}/Contents/Windows64")
    set(RESOURCE_OUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${PLUGIN_NAME}-test/Plugins/${PLUGIN_NAME}/Contents/Resources")
    set_target_properties(${PLUGIN_NAME} PROPERTIES
        SUFFIX ".4DX"
        LINK_FLAGS "/DEF:\"${SDK_DIR}/4DPluginAPI.def\""
        RUNTIME_OUTPUT_DIRECTORY "${PLUGIN_OUT_DIR}"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PLUGIN_OUT_DIR}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PLUGIN_OUT_DIR}")
    target_link_libraries(${PLUGIN_NAME} PRIVATE md4c_static)
    add_custom_command(TARGET ${PLUGIN_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${RESOURCE_OUT_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/manifest.json"
            "${CMAKE_CURRENT_SOURCE_DIR}/constants.xlf" "${RESOURCE_OUT_DIR}")
else()
    message(FATAL_ERROR "Only macOS and Windows plugins are supported")
endif()

target_include_directories(${PLUGIN_NAME} PRIVATE "${SDK_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
set_source_files_properties("${SDK_DIR}/4DPluginAPI.c" PROPERTIES LANGUAGE CXX)
target_compile_definitions(${PLUGIN_NAME} PRIVATE UNICODE _UNICODE)
if(WIN32)
    target_compile_definitions(${PLUGIN_NAME} PRIVATE NOMINMAX _CRT_SECURE_NO_WARNINGS)
endif()
