cmake_minimum_required(VERSION 2.6)

PROJECT(CoProcessingTest)

find_package(ParaView 4.1 REQUIRED COMPONENTS vtkPVCatalyst)
## vtkPVCatalyst: vtkCPDataDescription.h

INCLUDE(${PARAVIEW_USE_FILE})

ADD_EXECUTABLE(CoProcessingTest CoProcessingTest.cxx)
TARGET_LINK_LIBRARIES(CoProcessingTest vtkPVCatalyst)

