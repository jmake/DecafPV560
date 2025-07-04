#pragma once
//#ifndef DICOM_TOOLS_H  
//#define DICOM_TOOLS_H 

#include <vector>
#include <string>
#include <vtkImageData.h>
#include <vtkPoints.h>  // Add this

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

            void LoadFile(std::string directory, bool centered); 
            void VtiSave(std::string fname);  

            void CutCreate(std::string key); 
            void CutSave(std::string key); 
            void CutUpdate(float x, float y, float z, float nx, float ny, float nz); 
            void CutDimensions(int* arrayInt1);
            void CutGeometry(int* arrayInt1, float* arrayFloat1, float* arrayFloat2); 


            void ContourCreate(std::string key, float maxCells); 
            void ContourSave(std::string key); 
            void ContourUpdate(float u0, float umin, float umax); 

            void ContourDimensions(int* arrayInt1);
            void ContourGeometry(int* arrayInt1, float* arrayFloat1, float* arrayFloat2); 

        private :
            std::string fname; 
            VtkGrid<vtkImageData> *manager; 
    }; // Nifti



    class VtkManager 
    {
        public :
        ~VtkManager(){}; 
        VtkManager(){}; 

        void Init();

        void* GetObj(); 
        bool  SetObj(vtkDataObject* obj, std::string key); 

        void Finish(); 
        void Save(std::string fname); 

        void GeometryUpdate(); 
        void PointDataUpdate(std::string key); 

        void RangeGet(float* arrayFloat1);
        void DimensionsGet(int* arrayInt1);
        void GeometryGet(int* arrayInt1, float* arrayFloat1, float* arrayFloat2); 

        std::vector<float> range; 
        std::vector<int>   indices; 
        std::vector<float> vertices; 
        std::vector<float> property; 

        private : 
            vtkDataObject* obj; 
            vtkPointData* pointData; 
            vtkPoints* points;

    }; 


    class VtkLoader : public VtkManager 
    {
        public : 
        bool LoadFile(std::string directory, std::string key); 
    }; 


    class NifftiLoader : public VtkManager 
    {
        public : 
        bool LoadFile(std::string directory, std::string key); 
    };  


    class CutterManager : public VtkManager 
    {
        public : 
        void Finish()
        {
            VtkManager::Finish();

            key.clear(); 
            orig.clear(); 
            normal.clear(); 

            if(vti) vti = nullptr; 
        }

/*
        void Reset() 
        {
            VtkManager::Finish();

            key.clear(); 
            orig.clear(); 
            normal.clear(); 
        }
*/

        void VtiSet(void* obj, std::string key);
        bool Update(float nx, float ny, float nz); 
        bool Update(float x, float y, float z, float nx, float ny, float nz); 

        private : 
            std::string key; 
            std::vector<double> orig;   
            std::vector<double> normal;  
            
            vtkImageData* vti; 
    }; 


} // namespace


//#endif // DICOM_TOOLS_H
//--------------------------------------------------------------------------||--//
//--------------------------------------------------------------------------||--//
/*
CHECK : 
    vtkErrorMacro( << "There was a problem retrieving data from: " << file );

    F:\z2025_1\Dicom\DecafPV560\SRC\VTK\IO\Image\vtkDICOMImageReader.cxx + 
*/