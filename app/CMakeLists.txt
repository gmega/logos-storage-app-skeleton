cmake_minimum_required(VERSION 3.16)
project(LogosStorageCLI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

########### DEPENDENCIES ###########

if(NOT EXISTS "${LOGOS_LIBLOGOS_ROOT}/include/interface.h")
    message(FATAL_ERROR "logos-liblogos not found at ${LOGOS_LIBLOGOS_ROOT}. "
                        "Set LOGOS_LIBLOGOS_ROOT or run git submodule update --init --recursive.")
endif()

if(NOT EXISTS "${LOGOS_CPP_SDK_ROOT}/include/cpp/logos_api.h")
    message(FATAL_ERROR "logos-cpp-sdk not found at ${LOGOS_CPP_SDK_ROOT}. "
                        "Set LOGOS_CPP_SDK_ROOT or run git submodule update --init --recursive.")
endif()

if(NOT EXISTS "${LOGOS_STORAGE_ROOT}/include/storage_module_api.h")
    message(FATAL_ERROR "logos-storage-module not found at ${LOGOS_STORAGE_ROOT}. "
                        "Set LOGOS_STORAGE_ROOT or run git submodule update --init --recursive.")
endif()

message(STATUS "Using logos-liblogos at: ${LOGOS_LIBLOGOS_ROOT}")
message(STATUS "Using logos-cpp-sdk at: ${LOGOS_CPP_SDK_ROOT}")
message(STATUS "Using logos-storage-module at: ${LOGOS_STORAGE_ROOT}")

########### DEPENDENCIES END ###########

# Sources
set(COMMON_SOURCES
    logos_manager.cpp
)

# CPP Generator
set(PLUGINS_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/generated_code")
list(APPEND COMMON_SOURCES ${PLUGINS_OUTPUT_DIR}/logos_sdk.cpp)

########### APP DEFINITION ###########

# Find Qt packages
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core RemoteObjects)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core RemoteObjects)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Shared library target holding common sources, includes, and link libraries
add_library(logos-common STATIC ${COMMON_SOURCES})

find_library(LOGOS_CORE_LIB logos_core PATHS ${LOGOS_LIBLOGOS_ROOT}/lib NO_DEFAULT_PATH REQUIRED)

target_link_libraries(logos-common PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::RemoteObjects
    ${LOGOS_CORE_LIB}
)

# Link SDK library if using installed layout (source layout compiles the SDK directly)
find_library(LOGOS_SDK_LIB logos_sdk PATHS ${LOGOS_CPP_SDK_ROOT}/lib NO_DEFAULT_PATH REQUIRED)
target_link_libraries(logos-common PUBLIC ${LOGOS_SDK_LIB})

# Create executables
add_executable(storage-app main.cpp)
target_link_libraries(storage-app PRIVATE logos-common)

########### APP DEFINITION END ###########

########### HEADERS ###########

# Include directories on the common target so all executables inherit them
target_include_directories(logos-common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PLUGINS_OUTPUT_DIR}
)

target_include_directories(logos-common PUBLIC ${LOGOS_LIBLOGOS_ROOT}/include)
target_include_directories(logos-common PUBLIC
    ${LOGOS_CPP_SDK_ROOT}/include
    ${LOGOS_CPP_SDK_ROOT}/include/cpp
    ${LOGOS_CPP_SDK_ROOT}/include/core
    ${PLUGINS_OUTPUT_DIR}/include
)

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
foreach(_target storage-app)
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
install(TARGETS storage-app
    RUNTIME DESTINATION bin
)

########### INSTALL END ###########
