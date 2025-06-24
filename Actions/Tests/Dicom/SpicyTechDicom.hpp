#pragma once
//#ifndef DICOM_TOOLS_H  
//#define DICOM_TOOLS_H 

#include <string>
#include <vtkImageData.h>
//#include "vtktools.hpp" // fails!!

template <class vtkGridType> class VtkGrid; 

//int NiftiReader(std::string fin_1); 

//--------------------------------------------------------------------------||--//
//--------------------------------------------------------------------------||--//
namespace SpicyTech {
    
    class Dicom
    {
        public :
            ~Dicom(); 
             Dicom(); 

            void LoadFile(std::string directory); 
            void CutCreate(); 
            
            void TestVti(std::string directory); 

        private :
            std::string fname; 
            VtkGrid<vtkImageData> *mesh; 
    }; // Dicom


    class Nifti
    {
        public :
            ~Nifti(); 
             Nifti(); 

            void LoadFile(std::string directory); 
            void CutSave(std::string key); 
            void CutCreate(std::string key); 
            void CutUpdate(float x, float y, float z, float nx, float ny, float nz); 

        private :
            std::string fname; 
            VtkGrid<vtkImageData> *manager; 
    }; // Nifti


} // namespace


//#endif // DICOM_TOOLS_H
//--------------------------------------------------------------------------||--//
//--------------------------------------------------------------------------||--//
/*
CHECK : 
    vtkErrorMacro( << "There was a problem retrieving data from: " << file );

    F:\z2025_1\Dicom\DecafPV560\SRC\VTK\IO\Image\vtkDICOMImageReader.cxx + 
*/