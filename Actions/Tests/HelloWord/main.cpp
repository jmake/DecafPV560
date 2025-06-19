#include <vtkSmartPointer.h>
#include <vtkVersion.h>
#include <iostream>

int main(int argc, char* argv[])
{
    std::cout << "VTK version: " << vtkVersion::GetVTKVersion() << std::endl;
    std::cout << "Hello, VTK World!" << std::endl;
    return 0;
}
