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
#######################################################################################


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


file(WRITE ${SOURCES_DIR}/ParaViewIncludeDirs.txt "${VTK_INCLUDE_DIRS}")

# Collect library directories from all VTK libraries
set(LIB_DIRS "")
foreach(vtk_lib ${VTK_LIBS})
    get_target_property(lib_location ${vtk_lib} LOCATION)
    if(lib_location)
        get_filename_component(lib_dir ${lib_location} DIRECTORY)
        list(APPEND LIB_DIRS ${lib_dir})
    endif()
endforeach()

# Remove duplicates
list(REMOVE_DUPLICATES LIB_DIRS)

# Join directories with semicolon or newline
string(JOIN ";" LIB_DIRS_STR ${LIB_DIRS})

# Write to file
file(WRITE "${SOURCES_DIR}/ParaViewLibDirs.txt" "${LIB_DIRS_STR}")



###list(APPEND CMAKE_SWIG_FLAGS -namespace SpicyTech) ## --> %module SpicyTech ## same name in *.i file!!
list(APPEND CMAKE_SWIG_FLAGS -threads)
foreach(vtk_lib ${VTK_LIBS})
    get_target_property(vtk_inc ${vtk_lib} INTERFACE_INCLUDE_DIRECTORIES)
    foreach(inc ${vtk_inc})
        list(APPEND CMAKE_SWIG_FLAGS "-I${inc}")
    endforeach()
endforeach()

##message(STATUS "VTK_INCLUDE_DIRS: ${VTK_INCLUDE_DIRS}") ## several!!
message(STATUS "VTK_INCLUDE_DIRS:")
foreach(dir IN LISTS VTK_INCLUDE_DIRS)
    message(STATUS "  ${dir}")
endforeach()


list(APPEND CMAKE_SWIG_FLAGS "-I${SOURCES_DIR}") #  Unable to find 'SpicyTechDicom.hpp' in *.i file!!

list(APPEND ALL_LIBRARIES ${VTK_LIBS}) 


## Sources ###############################################################################
file(GLOB SRCS "${SOURCES_DIR}/*.cpp") 
message(STATUS "Files:")
foreach(file ${SRCS})
   message(STATUS " - ${file}")
endforeach()


set(LIBRARY_NAME "LibrarySnippet") 

## IMPORTANT: in '', 'class VtkTest' -> 'class __declspec(dllexport) VtkTest' 
add_definitions(-DSPICYTECH_API)
add_library(${LIBRARY_NAME} SHARED ${SRCS}) ## -> .so? / .dll 

##add_library(${LIBRARY_NAME} STATIC ${SRCS}) ## ->   .a / .lib  

target_link_libraries(${LIBRARY_NAME} ${ALL_LIBRARIES})


get_target_property(LIBS ${LIBRARY_NAME} LINK_LIBRARIES)
message(STATUS "Link libraries for MyTarget: ${LIBS}")



## Swig ###############################################################################
#######################################################################################


## LibraryName.dll needs 'zd.dll' in other case, it fails!! ###########################
set(ZLIB_DLL "${ZLIB_ROOT}/bin/zd.dll")
add_custom_command(TARGET ${LIBRARY_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${ZLIB_DLL}"
            "${CMAKE_SWIG_OUTDIR}/zd.dll"
    COMMENT "Copying zd.dll to output directory"
)


## Main ###############################################################################
set(EXECUTABLE_NAME SpicyTech)  

add_executable(${EXECUTABLE_NAME} "${SOURCES_DIR}/main.cxx") 
set_property(TARGET ${EXECUTABLE_NAME} PROPERTY LINKER_LANGUAGE CXX)
target_link_libraries(${EXECUTABLE_NAME} ${LIBRARY_NAME}) 



## Test ###############################################################################
#install(TARGETS ${EXECUTABLE_NAME} RUNTIME DESTINATION bin)
#install(FILES "${ZLIB_ROOT}/bin/zd.dll" DESTINATION bin)
#enable_testing()
#add_test(NAME "Run${EXECUTABLE_NAME}" COMMAND ${EXECUTABLE_NAME})


#######################################################################################
#######################################################################################