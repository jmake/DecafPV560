cmake_minimum_required(VERSION 3.15)

project(SpicyLibrary LANGUAGES CXX)

# /MD -> matches Unreal Engine (dynamic CRT)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Output all artifacts to source directory
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})  # .lib (import library)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})  # .dll
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})  # .dll as well

# Define the DLL target
add_library(DynamicLibrary SHARED
    ExternalLibrary.cpp
)

# Export macro definition, equivalent to: /D EXTERNALMATH_EXPORTS
target_compile_definitions(DynamicLibrary PRIVATE EXTERNALMATH_EXPORTS)

# Optional: include current dir
target_include_directories(DynamicLibrary PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
