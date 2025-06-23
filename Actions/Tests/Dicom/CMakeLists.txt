cmake_minimum_required(VERSION 3.5)

project(SpicyTech VERSION 0.1 LANGUAGES CXX)  
set(CMAKE_CXX_STANDARD 11)

## VTK   
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
target_link_libraries(${exec01} ${LIBS})   

install(TARGETS ${exec01} RUNTIME DESTINATION bin)

## Testing 
enable_testing()
add_test(NAME "Run${exec01}" COMMAND ${exec01})
