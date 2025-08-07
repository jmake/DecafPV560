#pragma once
//#ifndef DICOM_TOOLS_H  
//#define DICOM_TOOLS_H 

#include <chrono>
#include <vector>
#include <string>
#include <cstdint>


//--------------------------------------------------------------------------||--//
//--------------------------------------------------------------------------||--//

namespace SpicyTech2 {

#ifdef SPICYTECH_API
    class __declspec(dllexport) VtkTest 
#else
    class __declspec(dllimport) VtkTest
#endif
    {
        public :
        ~VtkTest(); 

        VtkTest(); 

        void NumpyPrint(double* data, int size); // -> SEE : configuration.i
        std::vector<double> GetData();  

        void _LoadFile(); 

        std::string GetVersion()
        {
            return version; 
        } 

        std::string version; 
    }; 

} // namespace SpicyTech2


//--------------------------------------------------------------------------||--//
//-------------------------------------------------------------------| Nifti |--//
/*
namespace SpicyTech3 {

#ifdef SPICYTECH_API
    class __declspec(dllexport) Nifti 
#else
    class __declspec(dllimport) Nifti
#endif
    {
        public :
        ~Nifti(){}; 

        Nifti(){vti = nullptr;}; 

        void Finish(); 
        void LoadFile(std::string fname); 

        float* GetBuffer();
        int GetBufferSize();

        std::vector<int> GetDimensions(); 
        std::vector<float> GetSpacing(); 

        float GetValue(int i, int j, int k, int l); 

        void* GetVti();  
        void SaveVti(std::string fname); 
        
        std::string GetKeyName(); 

        std::vector<float> GetRange(); 

        private : 
        int data_size = 0; 

        float* data_ptr = nullptr; 

        std::vector<float> range; 
        std::vector<float> spacing; 
        std::vector<int> dimensions;

        std::string key;  

        void* vti; 
    }; 

} // namespace SpicyTech3 
*/

//--------------------------------------------------------------------------||--//
//----------------------------------------------------------------| Contour3 |--//
/*
namespace SpicyTech2 {

#ifdef SPICYTECH_API
    class __declspec(dllexport) Contour3 
#else
    class __declspec(dllimport) Contour3
#endif
    {
        public :
        ~Contour3(); 

        Contour3(); 

        void Finish(); 

        void SetVti(void* obj, std::string key, std::vector<float> range, double maxCells); 
        void Update(double threshold, bool centered); 
        //void GetProperty(float* array); 
        void GetRange(); 

        //template<typename T> T* GetTriangles(int& n_indices);
        long long* GetTriangles(int& n_indices, long long& n_raw);

        void SaveVtp(std::string fname); 

        float* GetVertices(int& n_rows, int& n_cols); 

        private : 
        void* vtp; // vtkPolyData
        void* vti; // vtkImageData

        double maxCells; 

        std::string key;   
        std::vector<float> range; 

        std::vector<int>   indices; 
        std::vector<float> vertices; 
        //std::vector<float> property; 
    }; 

} // Contour3, namespace SpicyTech2  

*/
//--------------------------------------------------------------------------||--//





//--------------------------------------------------------------------------||--//
//--------------------------------------------------------------------------||--//