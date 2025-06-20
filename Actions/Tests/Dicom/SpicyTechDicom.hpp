#pragma once
//#ifndef DICOM_TOOLS_H  
//#define DICOM_TOOLS_H 

#include <string>
#include <vtkImageData.h>
//#include "vtktools.hpp" // fails!!

template <class vtkGridType> class VtkGrid; 


//--------------------------------------------------------------------------||--//
//--------------------------------------------------------------------------||--//
namespace SpicyTech 
{
    class Dicom
    {
        public :
            ~Dicom(); 
             Dicom(); 

            void TestVti(std::string directory); 
            void LoadFile(std::string directory); 

            void CutCreate(); 

        private :
            std::string fname; 
            VtkGrid<vtkImageData> *mesh; 
    }; // Dicom

} // namespace


//#endif // DICOM_TOOLS_H
//--------------------------------------------------------------------------||--//
//--------------------------------------------------------------------------||--//
/*
CHECK : 
    vtkErrorMacro( << "There was a problem retrieving data from: " << file );

    F:\z2025_1\Dicom\DecafPV560\SRC\VTK\IO\Image\vtkDICOMImageReader.cxx + 
*/