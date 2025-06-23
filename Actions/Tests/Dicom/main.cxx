#include <iostream>

#include "SpicyTechDicom.hpp"

/*
F:\z2025_1\Dicom\DecafPV560\SRC\VTK\Utilities\DICOMParser\DICOMAppHelper.cxx
-> [DICOMAppHelper_] (ptrIncr == 2)

F:\z2025_1\Dicom\DecafPV560\SRC\VTK\IO\Image\vtkDICOMImageReader.cxx

*/
int main(int argc, char* argv[])
{
    std::string directory; 
    //directory = "F:/z2025_1/Dicom/DicomTestImages4/series-000001"; // :( 
    // nrows: 1310720 ndims:1, range: (0, 1059) 
    directory = "F:\\z2025_1\\Dicom\\DicomTestImages1\\matlab\\examples\\sample_data\\DICOM\\digest_article"; 

    auto* dicom = new SpicyTech::Dicom();
    dicom->LoadFile( directory ); 
    //dicom->TestVti("F:\\z2025_1\\Dicom\\DecafPV560\\headsq.vti"); 
    
    delete dicom;

    std::cout << "[SpicyTech] Done!!" << std::endl;
    return 0;
}
/*
    F:\z2025_1\ParaView600\bin\paraview.exe .\Actions\Tests\Dicom\test2.vti
*/