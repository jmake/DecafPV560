#pragma once
//#ifndef DICOM_TOOLS_H  
//#define DICOM_TOOLS_H 

#include <chrono>
#include <vector>
#include <string>
#include <vtkImageData.h>
#include <vtkPoints.h>  // Add this

#include <cstdint>

//#include "vtktools.hpp" // fails!!

template <class vtkGridType> class VtkGrid; 

//int NiftiReader(std::string fin_1); 

//--------------------------------------------------------------------------||--//
//--------------------------------------------------------------------------||--//
namespace SpicyTech {

    
class DataHolder {
public:
    DataHolder(size_t size);
    ~DataHolder();

    float* GetDataPtr();
    size_t GetDataSize() const;

    uintptr_t GetDataAddress();

private:
    float* data_;
    size_t size_;
};


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
        bool  SetObj(vtkDataObject* obj, std::string key, bool extract=true); 

        void Finish(); 
        void Save(std::string fname); 

        void GeometryUpdate(); 
        void PointDataUpdate(std::string key); 

        void BoundsGet(double* arrayDouble1); 
        void RangeGet(std::string key, float* arrayFloat1);
        void RangeGet(float* arrayFloat1);
        void DimensionsGet(int* arrayInt1);
        void GeometryGet(int* arrayInt1, float* arrayFloat1, float* arrayFloat2); 

        std::vector<float> range; 
        std::vector<int>   indices; 
        std::vector<float> vertices; 
        std::vector<float> property; 

        protected:
            vtkDataObject* obj; 

        private : 
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
        bool LoadFile(std::string directory, std::string key, bool centered=false, bool extract=true); 
    };  


    //--------------------------------------------------------------------------||--//
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
        bool Update(float nx, float ny, float nz, bool centered=false); 
        bool Update(float x, float y, float z, float nx, float ny, float nz, bool centered=false); 

        private : 
            std::string key; 
            std::vector<double> orig;   
            std::vector<double> normal;  
            
            vtkImageData* vti; 
    }; 


    //--------------------------------------------------------------------------||--//
    class CutterManager2 : public VtkManager 
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

        void VtiSet(void* obj, std::string key);
        bool Update(float nx, float ny, float nz, bool centered=false); 
        bool Update(float x, float y, float z, float nx, float ny, float nz, bool centered=false); 

void GetOrigin(double* arrayDouble1);
void GetSpacing(double* arrayDouble1);
void GetProperty(float* arrayFloat1); 
int  PointsByAxis(int* arrayInt1);

uintptr_t GetDataAddress(int* arrayInt1); // fails in Unity 
uintptr_t GetDataAddress2(int* arrayInt1); // fails in Unity 

        private : 
        std::string key; 
        std::vector<double> orig;   
        std::vector<double> normal;  
        
        vtkImageData* vti; 
    }; 


    //--------------------------------------------------------------------------||--//
    class Contour2 : public VtkManager 
    {
        public : 
        void Finish()
        {
            VtkManager::Finish();
            if(vti) vti = nullptr; 
        }

        void Tick() 
        {
            start = std::chrono::high_resolution_clock::now();
        }

        void Tack(std::string msg) 
        {
            //auto start = std::chrono::high_resolution_clock::now();
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            std::cout << "\t - ["<< msg <<"] Execution time: " << duration.count() / 1000.0 << " seg" << std::endl;
            //std::cout << "=======================================================================================\n"; 
        }

        void VtiSet(void* obj, std::string key, float* arrayFloat1, double maxCells);
        bool Update(double threshold, bool centered=false); 
        void GetProperty(float* array); 

        private : 
        double maxCells; 
        std::string key;         
        vtkImageData* vti; 

        std::chrono::time_point<std::chrono::high_resolution_clock> start;
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