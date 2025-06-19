cmake_minimum_required(VERSION 3.5)

project(HelloVTK LANGUAGES CXX)

# Pass CMAKE_INSTALL_PREFIX from command line or use default
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Install path" FORCE)
endif()

message("[SpicyTech] VTK_DIR:${VTK_DIR}") 

find_package(VTK REQUIRED COMPONENTS
  vtkCommonCore
  vtkCommonDataModel
  vtkFiltersSources
)

include(${VTK_USE_FILE})

add_executable(HelloVTK main.cpp)

target_link_libraries(HelloVTK PRIVATE ${VTK_LIBRARIES})

install(TARGETS HelloVTK RUNTIME DESTINATION bin)

enable_testing()

add_test(NAME RunHelloVTK COMMAND HelloVTK)
