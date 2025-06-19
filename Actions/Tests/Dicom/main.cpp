#include <iostream>
#include <vtkVersion.h>
#include <vtkSmartPointer.h>

#include <string>
#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <vtkDICOMImageReader.h>

#include "vtktools.hpp" 


vtkSmartPointer<vtkImageData> ReadDICOMSeries(const std::string& path) 
{
    auto reader = vtkSmartPointer<vtkDICOMImageReader>::New();
    reader->SetDirectoryName(path.c_str());
    reader->Update();

    std::cout << "GetStudyUID: " << reader->GetStudyUID() << std::endl;
    std::cout << "GetPatientName: " << reader->GetPatientName() << std::endl;
    std::cout << "GetNumberOfComponents: " << reader->GetNumberOfComponents() << std::endl;

    vtkImageData* image = reader->GetOutput();
    std::cout << "GetNumberOfScalarComponents: " << image->GetNumberOfScalarComponents() << std::endl;

    return image;
}


int main(int argc, char* argv[])
{
    std::cout << "[SpicyTech] VTK version: " << vtkVersion::GetVTKVersion() << std::endl;
    std::cout << "[SpicyTech] Hello, VTK World!" << std::endl;

    std::string directory; 
    //directory = "F:\\z2025_1\\Dicom\\DicomTestImages1\\matlab\\examples\\sample_data\\DICOM\\digest_article"; 
    directory = "F:/z2025_1/Dicom/DicomTestImages4/series-000001"; 

    ReadDICOMSeries(directory); 

    return 0;
}
