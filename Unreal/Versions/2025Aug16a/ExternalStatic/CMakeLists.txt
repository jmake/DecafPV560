cmake_minimum_required(VERSION 3.15)

project(SpicyLibrary LANGUAGES CXX)

# Use Release config and /MD runtime (matches Unreal Engine)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")  # /MD
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

## 'Release' folder 
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})  # .lib 
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})  # .dll (if any)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})  # .exe (if any)


# Add your source file
add_library(StaticLibrary STATIC
    StaticTest.cpp
)

# Optionally set include directory
target_include_directories(StaticLibrary PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
