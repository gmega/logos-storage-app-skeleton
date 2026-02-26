cmake_minimum_required(VERSION 3.16)
project(LogosStorageCLI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

########### DEPENDENCIES ###########

# This section locates the required dependencies in 3 different ways:
# 1- With NIX, the root folders (LOGOS_LIBLOGOS_ROOT, LOGOS_CPP_SDK_ROOT,
#    and LOGOS_STORAGE_ROOT) are defined and point to the correct locations in
#    the Nix store.
# 2- If the root folders are fetched from source (basically using git) in the
#    parent folder, this is detected and used.
# 3- If none of the above apply, the vendor folders inside this project are used,
#    meaning the dependencies need to be fetched using git submodules.

if(NOT DEFINED LOGOS_LIBLOGOS_ROOT)
    set(_parent_liblogos "${CMAKE_SOURCE_DIR}/../../logos-liblogos")
    set(_use_vendor ${LOGOS_STORAGE_CLI_USE_VENDOR})
    if(NOT _use_vendor)
        if(NOT EXISTS "${_parent_liblogos}/src/common/interface.h")
            set(_use_vendor ON)
        endif()
    endif()
    if(_use_vendor)
        set(LOGOS_LIBLOGOS_ROOT "${CMAKE_SOURCE_DIR}/../vendor/logos-liblogos")
    else()
        set(LOGOS_LIBLOGOS_ROOT "${_parent_liblogos}")
    endif()
endif()

if(NOT DEFINED LOGOS_CPP_SDK_ROOT)
    set(_parent_cpp_sdk "${CMAKE_SOURCE_DIR}/../../logos-cpp-sdk")
    set(_use_vendor ${LOGOS_STORAGE_CLI_USE_VENDOR})
    if(NOT _use_vendor)
        if(NOT EXISTS "${_parent_cpp_sdk}/cpp/logos_api.h")
            set(_use_vendor ON)
        endif()
    endif()
    if(_use_vendor)
        set(LOGOS_CPP_SDK_ROOT "${CMAKE_SOURCE_DIR}/../vendor/logos-cpp-sdk")
    else()
        set(LOGOS_CPP_SDK_ROOT "${_parent_cpp_sdk}")
    endif()
endif()

if(NOT DEFINED LOGOS_STORAGE_ROOT)
    set(_parent_storage_module "${CMAKE_SOURCE_DIR}/../../logos-storage-module")
    set(_use_vendor ${LOGOS_STORAGE_CLI_USE_VENDOR})
    if(NOT _use_vendor)
        if(NOT EXISTS "${_parent_storage_module}/storage_module_plugin.h")
            set(_use_vendor ON)
        endif()
    endif()
    if(_use_vendor)
        set(LOGOS_STORAGE_ROOT "${CMAKE_SOURCE_DIR}/../vendor/logos-storage-module")
    else()
        set(LOGOS_STORAGE_ROOT "${_parent_storage_module}")
    endif()
endif()

set(_liblogos_found FALSE)
if(EXISTS "${LOGOS_LIBLOGOS_ROOT}/src/common/interface.h")
    set(_liblogos_found TRUE)
    set(_liblogos_is_source TRUE)
elseif(EXISTS "${LOGOS_LIBLOGOS_ROOT}/include/interface.h")
    set(_liblogos_found TRUE)
    set(_liblogos_is_source FALSE)
endif()

set(_cpp_sdk_found FALSE)
if(EXISTS "${LOGOS_CPP_SDK_ROOT}/cpp/logos_api.h")
    set(_cpp_sdk_found TRUE)
    set(_cpp_sdk_is_source TRUE)
elseif(EXISTS "${LOGOS_CPP_SDK_ROOT}/include/cpp/logos_api.h")
    set(_cpp_sdk_found TRUE)
    set(_cpp_sdk_is_source FALSE)
endif()

set(_storage_module_found FALSE)
if(EXISTS "${LOGOS_STORAGE_ROOT}/storage_module_plugin.h")
    set(_storage_module_found TRUE)
    set(_storage_module_is_source TRUE)
elseif(EXISTS "${LOGOS_STORAGE_ROOT}/include/storage_module_api.h")
    set(_storage_module_found TRUE)
    set(_storage_module_is_source FALSE)
endif()

if(NOT _liblogos_found)
    message(FATAL_ERROR "logos-liblogos not found at ${LOGOS_LIBLOGOS_ROOT}. "
                        "Set LOGOS_LIBLOGOS_ROOT or run git submodule update --init --recursive.")
endif()

if(NOT _cpp_sdk_found)
    message(FATAL_ERROR "logos-cpp-sdk not found at ${LOGOS_CPP_SDK_ROOT}. "
                        "Set LOGOS_CPP_SDK_ROOT or run git submodule update --init --recursive.")
endif()

if(NOT _storage_module_found)
    message(FATAL_ERROR "logos-storage-module not found at ${LOGOS_STORAGE_ROOT}. "
                        "Set LOGOS_STORAGE_ROOT or run git submodule update --init --recursive.")
endif()

message(STATUS "Using logos-liblogos at: ${LOGOS_LIBLOGOS_ROOT}")
message(STATUS "Using logos-cpp-sdk at: ${LOGOS_CPP_SDK_ROOT}")
message(STATUS "Using logos-storage-module at: ${LOGOS_STORAGE_ROOT}")

########### DEPENDENCIES END ###########

########### SOURCES ###########

set(COMMON_SOURCES
    logos_manager.cpp
    logos_manager.h
)

# Add SDK sources (only if source layout, installed layout uses the library)
if(_cpp_sdk_is_source)
    list(APPEND COMMON_SOURCES
        ${LOGOS_CPP_SDK_ROOT}/cpp/logos_api.cpp
        ${LOGOS_CPP_SDK_ROOT}/cpp/logos_api.h
        ${LOGOS_CPP_SDK_ROOT}/cpp/logos_api_client.cpp
        ${LOGOS_CPP_SDK_ROOT}/cpp/logos_api_client.h
        ${LOGOS_CPP_SDK_ROOT}/cpp/logos_api_consumer.cpp
        ${LOGOS_CPP_SDK_ROOT}/cpp/logos_api_consumer.h
        ${LOGOS_CPP_SDK_ROOT}/cpp/logos_api_provider.cpp
        ${LOGOS_CPP_SDK_ROOT}/cpp/logos_api_provider.h
        ${LOGOS_CPP_SDK_ROOT}/cpp/token_manager.cpp
        ${LOGOS_CPP_SDK_ROOT}/cpp/token_manager.h
        ${LOGOS_CPP_SDK_ROOT}/cpp/module_proxy.cpp
        ${LOGOS_CPP_SDK_ROOT}/cpp/module_proxy.h
        ${LOGOS_CPP_SDK_ROOT}/cpp/logos_types.cpp
        ${LOGOS_CPP_SDK_ROOT}/cpp/logos_types.h
    )
endif()

########### SOURCES END ###########

########### CPP GENERATOR ###########

# Define metadata json path
set(METADATA_JSON "${CMAKE_CURRENT_SOURCE_DIR}/metadata.json")

# Run the cpp generator for the storage module (produces storage_module_api.h/cpp).
# Only for source layout - nix builds already have pre-generated files.
if(_storage_module_is_source)
    set(PLUGINS_OUTPUT_DIR "${CMAKE_BINARY_DIR}/generated_code")

    set(CPP_GENERATOR_BUILD_DIR "${LOGOS_CPP_SDK_ROOT}/../build/cpp-generator")
    set(CPP_GENERATOR "${CPP_GENERATOR_BUILD_DIR}/bin/logos-cpp-generator")

    if(NOT TARGET cpp_generator_build)
        add_custom_target(cpp_generator_build
            COMMAND bash "${LOGOS_CPP_SDK_ROOT}/cpp-generator/compile.sh"
            WORKING_DIRECTORY "${LOGOS_CPP_SDK_ROOT}/.."
            COMMENT "Building logos-cpp-generator via ${LOGOS_CPP_SDK_ROOT}/cpp-generator/compile.sh"
            VERBATIM
        )
    endif()

    add_custom_target(run_build_storage_module
        COMMAND "${CMAKE_COMMAND}" -S "${LOGOS_STORAGE_ROOT}" -B "${LOGOS_STORAGE_ROOT}/build"
        COMMAND "${CMAKE_COMMAND}" --build "${LOGOS_STORAGE_ROOT}/build" --target all
        WORKING_DIRECTORY "${LOGOS_STORAGE_ROOT}/.."
        COMMENT "Building storage module in ${LOGOS_STORAGE_ROOT}/build"
        VERBATIM
    )

    if(APPLE)
        set(PLUGIN_FILE "${LOGOS_STORAGE_ROOT}/build/modules/storage_module_plugin.dylib")
    elseif(UNIX)
        set(PLUGIN_FILE "${LOGOS_STORAGE_ROOT}/build/modules/storage_module_plugin.so")
    else()
        message(FATAL_ERROR "storage_module_plugin is not found.")
    endif()

    add_custom_target(run_cpp_generator_storage_module
        COMMAND ${CMAKE_COMMAND} -E make_directory "${PLUGINS_OUTPUT_DIR}"
        COMMAND "${CPP_GENERATOR}" "${PLUGIN_FILE}" --interface "${LOGOS_STORAGE_ROOT}/storage_module_interface.h" --module-only --output-dir "${PLUGINS_OUTPUT_DIR}"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMENT "Running logos-cpp-generator on ${PLUGIN_FILE} with output-dir ${PLUGINS_OUTPUT_DIR}"
        VERBATIM
    )

    add_dependencies(run_cpp_generator_storage_module cpp_generator_build)
    add_dependencies(run_cpp_generator_storage_module run_build_storage_module)
endif()

# Generate logos_sdk.cpp via --general-only --metadata.
# This file #includes all module APIs (storage_module_api.cpp etc.) based on
# the dependencies declared in metadata.json.
# storage_module_api.cpp is NOT added directly to SOURCES to avoid AUTOMOC issues
# with files that don't exist at configure time.
if(_cpp_sdk_is_source)
    if(NOT DEFINED PLUGINS_OUTPUT_DIR)
        set(PLUGINS_OUTPUT_DIR "${CMAKE_BINARY_DIR}/generated_code")
    endif()

    if(NOT DEFINED CPP_GENERATOR_BUILD_DIR)
        set(CPP_GENERATOR_BUILD_DIR "${LOGOS_CPP_SDK_ROOT}/../build/cpp-generator")
    endif()

    if(NOT DEFINED CPP_GENERATOR)
        set(CPP_GENERATOR "${CPP_GENERATOR_BUILD_DIR}/bin/logos-cpp-generator")
    endif()

    if(NOT TARGET cpp_generator_build)
        add_custom_target(cpp_generator_build
            COMMAND bash "${LOGOS_CPP_SDK_ROOT}/cpp-generator/compile.sh"
            WORKING_DIRECTORY "${LOGOS_CPP_SDK_ROOT}/.."
            COMMENT "Building logos-cpp-generator via ${LOGOS_CPP_SDK_ROOT}/cpp-generator/compile.sh"
            VERBATIM
        )
    endif()

    add_custom_command(
        OUTPUT "${PLUGINS_OUTPUT_DIR}/logos_sdk.cpp"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${PLUGINS_OUTPUT_DIR}"
        COMMAND "${CPP_GENERATOR}" --metadata "${METADATA_JSON}" --general-only --output-dir "${PLUGINS_OUTPUT_DIR}"
        WORKING_DIRECTORY "${LOGOS_CPP_SDK_ROOT}/.."
        DEPENDS "${METADATA_JSON}" cpp_generator_build
        VERBATIM
    )

    add_custom_target(run_cpp_generator_logos_cli DEPENDS "${PLUGINS_OUTPUT_DIR}/logos_sdk.cpp")

    if(_storage_module_is_source)
        add_dependencies(run_cpp_generator_logos_cli run_cpp_generator_storage_module)
    endif()

    list(APPEND COMMON_SOURCES ${PLUGINS_OUTPUT_DIR}/logos_sdk.cpp)

    # Mark as generated so CMake knows to wait for it
    set_source_files_properties(
        ${PLUGINS_OUTPUT_DIR}/logos_sdk.cpp
        PROPERTIES GENERATED TRUE
    )
else()
    # For installed/nix layout, logos_sdk.cpp is already pre-generated
    set(PLUGINS_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/generated_code")
    list(APPEND COMMON_SOURCES ${PLUGINS_OUTPUT_DIR}/logos_sdk.cpp)
endif()

########### CPP GENERATOR END ###########

########### APP DEFINITION ###########

# Find Qt packages
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core RemoteObjects)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core RemoteObjects)

# Link directories
link_directories(
    ${LOGOS_LIBLOGOS_ROOT}/lib
    ${LOGOS_CPP_SDK_ROOT}/lib
)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Shared library target holding common sources, includes, and link libraries
add_library(logos-common STATIC ${COMMON_SOURCES})

target_link_libraries(logos-common PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::RemoteObjects
    logos_core
)

# Link SDK library if using installed layout (source layout compiles the SDK directly)
if(NOT _cpp_sdk_is_source)
    find_library(LOGOS_SDK_LIB logos_sdk PATHS ${LOGOS_CPP_SDK_ROOT}/lib NO_DEFAULT_PATH REQUIRED)
    target_link_libraries(logos-common PUBLIC ${LOGOS_SDK_LIB})
endif()

# Create executables
add_executable(logos-uploader uploader.cpp)
target_link_libraries(logos-uploader PRIVATE logos-common)

add_executable(logos-downloader downloader.cpp)
target_link_libraries(logos-downloader PRIVATE logos-common)

########### APP DEFINITION END ###########

########### HEADERS ###########

# Include directories on the common target so all executables inherit them
target_include_directories(logos-common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PLUGINS_OUTPUT_DIR}
)

if(_liblogos_is_source)
    target_include_directories(logos-common PUBLIC ${LOGOS_LIBLOGOS_ROOT})
else()
    target_include_directories(logos-common PUBLIC ${LOGOS_LIBLOGOS_ROOT}/include)
endif()

if(_cpp_sdk_is_source)
    target_include_directories(logos-common PUBLIC
        ${LOGOS_CPP_SDK_ROOT}/cpp
        ${LOGOS_CPP_SDK_ROOT}/cpp/generated
    )
else()
    target_include_directories(logos-common PUBLIC
        ${LOGOS_CPP_SDK_ROOT}/include
        ${LOGOS_CPP_SDK_ROOT}/include/cpp
        ${LOGOS_CPP_SDK_ROOT}/include/core
        ${PLUGINS_OUTPUT_DIR}/include
    )
endif()

########### HEADERS END ###########

########### GENERATOR DEPENDENCY ###########

if(_cpp_sdk_is_source)
    add_dependencies(logos-common run_cpp_generator_logos_cli)
endif()

if(_storage_module_is_source)
    add_dependencies(logos-common run_cpp_generator_storage_module)
endif()

########### GENERATOR DEPENDENCY END ###########

########### LOGOS DEPENDENCIES LIBRARIES ###########

# Check if logos_sdk library exists
if(NOT EXISTS "${LOGOS_CPP_SDK_ROOT}/lib/liblogos_sdk.a" AND NOT EXISTS "${LOGOS_CPP_SDK_ROOT}/lib/liblogos_sdk.dylib" AND NOT EXISTS "${LOGOS_CPP_SDK_ROOT}/lib/liblogos_sdk.so")
    set(LOGOS_CPP_SDK_BUILD_DIR "${LOGOS_CPP_SDK_ROOT}/build")

    add_custom_target(run_build_cpp_sdk
        COMMAND ${CMAKE_COMMAND} -E make_directory "${LOGOS_CPP_SDK_BUILD_DIR}"
        COMMAND ${CMAKE_COMMAND} -E chdir "${LOGOS_CPP_SDK_BUILD_DIR}" cmake ../cpp -GNinja -DCMAKE_POSITION_INDEPENDENT_CODE=ON
        COMMAND ${CMAKE_COMMAND} -E chdir "${LOGOS_CPP_SDK_BUILD_DIR}" ninja
        COMMAND ${CMAKE_COMMAND} -E make_directory "${LOGOS_CPP_SDK_ROOT}/lib"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${LOGOS_CPP_SDK_BUILD_DIR}/lib/liblogos_sdk.a"
                "${LOGOS_CPP_SDK_ROOT}/lib/liblogos_sdk.a"
        WORKING_DIRECTORY "${LOGOS_CPP_SDK_ROOT}"
        COMMENT "Build cpp sdk and copy liblogos_sdk.a to ${LOGOS_CPP_SDK_ROOT}/lib"
        VERBATIM
    )

    add_dependencies(logos-common run_build_cpp_sdk)
endif()

if(NOT EXISTS "${LOGOS_LIBLOGOS_ROOT}/lib/liblogos_core.a" AND NOT EXISTS "${LOGOS_LIBLOGOS_ROOT}/lib/liblogos_core.dylib" AND NOT EXISTS "${LOGOS_LIBLOGOS_ROOT}/lib/liblogos_core.so")
    set(LOGOS_LIBLOGOS_BUILD_DIR "${LOGOS_LIBLOGOS_ROOT}/build")

    add_custom_target(run_build_liblogos
      COMMAND ${CMAKE_COMMAND} -E make_directory "${LOGOS_LIBLOGOS_BUILD_DIR}"
      COMMAND ${CMAKE_COMMAND} -E chdir "${LOGOS_LIBLOGOS_BUILD_DIR}" cmake .. -GNinja -DCMAKE_POSITION_INDEPENDENT_CODE=ON
      COMMAND ${CMAKE_COMMAND} -E chdir "${LOGOS_LIBLOGOS_BUILD_DIR}" ninja
      COMMAND ${CMAKE_COMMAND} -E make_directory "${LOGOS_LIBLOGOS_ROOT}/lib"
      COMMAND ${CMAKE_COMMAND} -E copy_directory
              "${LOGOS_LIBLOGOS_BUILD_DIR}/lib"
              "${LOGOS_LIBLOGOS_ROOT}/lib"
      WORKING_DIRECTORY "${LOGOS_LIBLOGOS_ROOT}"
      COMMENT "Build logos-liblogos and copy liblogos_core.a to ${LOGOS_LIBLOGOS_ROOT}/lib"
      VERBATIM
    )

    add_dependencies(logos-common run_build_liblogos)
endif()

########### LOGOS DEPENDENCIES END ###########

########### RUNTIME LINKS ###########

# Set RPATH settings for each executable
foreach(_target logos-uploader logos-downloader)
    if(APPLE)
        set_target_properties(${_target} PROPERTIES
            INSTALL_RPATH "@executable_path/../lib"
            BUILD_WITH_INSTALL_RPATH TRUE
        )
    elseif(UNIX)
        set_target_properties(${_target} PROPERTIES
            INSTALL_RPATH "$ORIGIN/../lib"
            BUILD_WITH_INSTALL_RPATH TRUE
        )
    endif()
endforeach()

########### RUNTIME LINKS END ###########

########### INSTALL ###########

# Install rules
install(TARGETS logos-uploader logos-downloader
    RUNTIME DESTINATION bin
)

########### INSTALL END ###########
