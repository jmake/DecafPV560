## Setup ###############################################################################
cmake_minimum_required(VERSION 3.16)

cmake_policy(SET CMP0122 NEW) ## ensures C# bindings use .NET-standard naming??
project(spicytech CXX C)

## Setting ... 
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUERIRED ON)

#set(CMAKE_BUILD_TYPE Release)
#set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>") 

set(CMAKE_BUILD_TYPE Debug)

include_directories(${PROJECT_SOURCE_DIR})

message("[WRAPPER] CMAKE_SOURCE_DIR = '${CMAKE_SOURCE_DIR}' ")
message("[WRAPPER] CMAKE_CURRENT_SOURCE_DIR = '${CMAKE_CURRENT_SOURCE_DIR}' ")


## Paraview ###############################################################################
list(APPEND VTK_LIBS 
    vtkCommonCore  
    vtkPVCatalyst 
    vtkPVVTKExtensionsDefault 
    vtkPVClientServerCoreCore
)
find_package(ParaView REQUIRED COMPONENTS ${VTK_LIBS})
include("${PARAVIEW_USE_FILE}")

##message(STATUS "VTK_INCLUDE_DIRS: ${VTK_INCLUDE_DIRS}") ## several!!

set(CMAKE_SWIG_FLAGS -namespace SpicyTech) # --> %module SpicyTech ## same name!!
foreach(vtk_lib ${VTK_LIBS})
    get_target_property(vtk_inc ${vtk_lib} INTERFACE_INCLUDE_DIRECTORIES)
    foreach(inc ${vtk_inc})
        list(APPEND CMAKE_SWIG_FLAGS "-I${inc}")
    endforeach()
endforeach()
list(APPEND CMAKE_SWIG_FLAGS "-I${SOURCES_DIR}") #  Unable to find 'SpicyTechDicom.hpp'



## Sources ###############################################################################
#include_directories("${PHYSX_ROOT_DIR}/include")  

file(GLOB SRCS "${SOURCES_DIR}/*.cpp") 
message(STATUS "Files:")
foreach(file ${SRCS})
   message(STATUS " - ${file}")
endforeach()

add_library(LibrarySnippet STATIC ${SRCS})


## Swig ###############################################################################
find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

set(CMAKE_SWIG_OUTDIR "${CMAKE_SOURCE_DIR}/../Assets/Plugins")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SWIG_OUTDIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SWIG_OUTDIR})

##set(CMAKE_SWIG_FLAGS "-I${SOURCES_DIR}")

set_source_files_properties(configuration.i PROPERTIES CPLUSPLUS ON)

#set(MODULE_NAME "ModuleName") ## ?? 
swig_add_library(LibraryName LANGUAGE csharp SOURCES configuration.i)
swig_link_libraries(LibraryName LibrarySnippet ${VTK_LIBS}) ## ${PHYSX_STATIC_LIBS} ${LIBRARIES} 

target_include_directories(${SWIG_MODULE_LibraryName_REAL_NAME} PRIVATE ${SOURCES_DIR})
target_link_libraries(${SWIG_MODULE_LibraryName_REAL_NAME} ${VTK_LIBS})


## Wrapper Test 
#file(GLOB WRAPPER_FILES "${CMAKE_SWIG_OUTDIR}/*.cs")
#
#add_custom_command(
#    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/wrapper.exe
#    COMMAND csc.exe 
#            /unsafe /platform:x64 /target:exe /out:wrapper.exe
#            ${CMAKE_CURRENT_SOURCE_DIR}/wrapper.cs
#            ${WRAPPER_FILES}
#    DEPENDS ${WRAPPER_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/wrapper.cs
#    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
#    COMMENT "Compiling wrapper.exe with csc.exe")
#
#add_custom_target(BuildRunMe ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/wrapper.exe)
#add_dependencies(BuildRunMe ${SWIG_MODULE_LibraryName_REAL_NAME} LibrarySnippet)


##
##install(TARGETS ${exec01} RUNTIME DESTINATION bin)
##
#### Testing 
##enable_testing()
##add_test(NAME "Run${exec01}" COMMAND ${exec01})
##