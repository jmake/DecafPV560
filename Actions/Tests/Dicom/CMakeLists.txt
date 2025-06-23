cmake_minimum_required(VERSION 3.5)

project(SpicyTech VERSION 0.1 LANGUAGES CXX)  
set(CMAKE_CXX_STANDARD 11)


string(REPLACE "/W3" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
string(REPLACE "/W3" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /w")

#add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/w>")
##add_compile_options("/Wd4101")
##set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4101")


## ZLIB 
message(STATUS "  [ZLIB_ROOT]:'${ZLIB_ROOT}'")

cmake_policy(SET CMP0074 NEW)

find_package(ZLIB REQUIRED)
message(STATUS "  [ZLIB_ROOT] ZLIB_LIBRARIES:'${ZLIB_LIBRARIES}'")
message(STATUS "  [ZLIB_ROOT] ZLIB_INCLUDE_DIRS:'${ZLIB_INCLUDE_DIRS}'")

include_directories(${ZLIB_INCLUDE_DIRS})
##set(ZLIB_USE_STATIC_LIBS ON CACHE BOOL "Use static zlib libraries")

add_definitions(-DHAVE_ZLIB)


## VTK   
message(STATUS "  [ParaView_DIR]:'${ParaView_DIR}'")

list(APPEND LIBS vtkPVCatalyst vtkPVVTKExtensionsDefault vtkPVClientServerCoreCore)
#list(APPEND LIBS vtkFiltersParallelGeometry vtkFiltersParallelMPI vtkFiltersParallel)

##list(APPEND LIBS vtkPVPythonCatalyst) # Only for python 
##set(CMAKE_CXX_FLAGS "-DWITH_PYTHON")  # Only for python 

find_package(ParaView REQUIRED COMPONENTS ${LIBS})
include("${PARAVIEW_USE_FILE}")

## Library
FILE(GLOB SRCs *.cpp)
##add_library(PAAKAT ${SRCs})

## Executable
set(exec01 SpicyTech)  

##add_executable(${exec01} main.cpp)  
add_executable(${exec01} ${SRCs} main.cxx)  

set_property(TARGET ${exec01} PROPERTY LINKER_LANGUAGE CXX)
target_link_libraries(${exec01} ${LIBS} ${ZLIB_LIBRARIES})   

install(TARGETS ${exec01} RUNTIME DESTINATION bin)
install(FILES "${ZLIB_ROOT}/bin/zd.dll" DESTINATION bin)


## Testing 
enable_testing()
add_test(NAME "Run${exec01}" COMMAND ${exec01})
