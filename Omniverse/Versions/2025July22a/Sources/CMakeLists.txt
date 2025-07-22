## Setup ###############################################################################
cmake_minimum_required(VERSION 3.16)

cmake_policy(SET CMP0122 NEW) ## ensures C# bindings use .NET-standard naming??
project(spicytech CXX)

## Setting ... 
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Release)
##set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>") 

#set(CMAKE_BUILD_TYPE Debug)

include_directories(${PROJECT_SOURCE_DIR})

message("[WRAPPER] CMAKE_SOURCE_DIR = '${CMAKE_SOURCE_DIR}' ")
message("[WRAPPER] CMAKE_CURRENT_SOURCE_DIR = '${CMAKE_CURRENT_SOURCE_DIR}' ")


## PYTHON #############################################################################
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
message("[WRAPPER] Python3_LIBRARIES = '${Python3_LIBRARIES}' ")
message("[WRAPPER] Python3_INCLUDE_DIRS = '${Python3_INCLUDE_DIRS}' ")


if(NOT DEFINED NUMPY_INCLUDE_DIR)
    execute_process(
    COMMAND python.exe -c "import numpy; print(numpy.get_include())"
    OUTPUT_VARIABLE NUMPY_INCLUDE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )
endif()

if(NOT IS_DIRECTORY "${NUMPY_INCLUDE_DIR}")
  message(FATAL_ERROR "NUMPY_INCLUDE_DIR '${NUMPY_INCLUDE_DIR}' does not exist or is not a directory. Aborting.")
endif()

message("[WRAPPER] NUMPY_INCLUDE_DIR = '${NUMPY_INCLUDE_DIR}' ")

include_directories(${NUMPY_INCLUDE_DIR})
include_directories(${Python3_INCLUDE_DIRS})
list(APPEND ALL_LIBRARIES ${Python3_LIBRARIES}) 


## ZLIB ###############################################################################
message(STATUS "  [ZLIB_ROOT]:'${ZLIB_ROOT}'")

cmake_policy(SET CMP0074 NEW)

find_package(ZLIB REQUIRED)
message(STATUS "  [ZLIB_ROOT] ZLIB_LIBRARIES:'${ZLIB_LIBRARIES}'")
message(STATUS "  [ZLIB_ROOT] ZLIB_INCLUDE_DIRS:'${ZLIB_INCLUDE_DIRS}'")

include_directories(${ZLIB_INCLUDE_DIRS})
##set(ZLIB_USE_STATIC_LIBS ON CACHE BOOL "Use static zlib libraries")

list(APPEND ALL_LIBRARIES ${ZLIB_LIBRARIES}) ## 'unresolved external symbol gzdopen referenced in function znzdopen'

add_definitions(-DHAVE_ZLIB)


## Paraview ###############################################################################
list(APPEND VTK_LIBS 
    vtkCommonCore  
    vtkPVCatalyst 
    vtkPVVTKExtensionsDefault 
    vtkPVClientServerCoreCore
)
find_package(ParaView REQUIRED COMPONENTS ${VTK_LIBS})
include("${PARAVIEW_USE_FILE}")


#list(APPEND CMAKE_SWIG_FLAGS -namespace SpicyTech) ## --> %module SpicyTech ## same name in *.i file!!
list(APPEND CMAKE_SWIG_FLAGS -threads)
foreach(vtk_lib ${VTK_LIBS})
    get_target_property(vtk_inc ${vtk_lib} INTERFACE_INCLUDE_DIRECTORIES)
    foreach(inc ${vtk_inc})
        list(APPEND CMAKE_SWIG_FLAGS "-I${inc}")
    endforeach()
endforeach()

##message(STATUS "VTK_INCLUDE_DIRS: ${VTK_INCLUDE_DIRS}") ## several!!
list(APPEND CMAKE_SWIG_FLAGS "-I${SOURCES_DIR}") #  Unable to find 'SpicyTechDicom.hpp' in *.i file!!

list(APPEND ALL_LIBRARIES ${VTK_LIBS}) 


## Sources ###############################################################################
file(GLOB SRCS "${SOURCES_DIR}/*.cpp") 
message(STATUS "Files:")
foreach(file ${SRCS})
   message(STATUS " - ${file}")
endforeach()

add_library(LibrarySnippet STATIC ${SRCS})


## Swig ###############################################################################
find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

#set(CMAKE_SWIG_OUTDIR "${CMAKE_SOURCE_DIR}/../Assets/Plugins")
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SWIG_OUTDIR})
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SWIG_OUTDIR})

set_source_files_properties(configuration.i PROPERTIES CPLUSPLUS ON)

#set(MODULE_NAME "ModuleName") ## ?? 
swig_add_library(LibraryName LANGUAGE python SOURCES configuration.i)
swig_link_libraries(LibraryName LibrarySnippet ${ALL_LIBRARIES}) 

target_include_directories(${SWIG_MODULE_LibraryName_REAL_NAME} PRIVATE ${SOURCES_DIR})
target_link_libraries(${SWIG_MODULE_LibraryName_REAL_NAME} ${ALL_LIBRARIES})

## LibraryName.dll needs 'zd.dll' in other case, it fails!!
set(ZLIB_DLL "${ZLIB_ROOT}/bin/zd.dll")
add_custom_command(TARGET ${SWIG_MODULE_LibraryName_REAL_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${ZLIB_DLL}"
            "${CMAKE_SWIG_OUTDIR}/zd.dll"
    COMMENT "Copying zd.dll to output directory"
)
