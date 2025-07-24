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

    //--------------------------------------------------------------------------||--//
    class VtkTest 
    {
        public :
        ~VtkTest(); 

        VtkTest(); 

        void NumpyPrint(double* data, int size); // -> SEE : configuration.i
        std::vector<double> GetData();  

    }; 

} // namespace SpicyTech2

//--------------------------------------------------------------------------||--//
namespace SpicyTech2 {

    //--------------------------------------------------------------------------||--//
    class Nifti 
    {
        public :
        ~Nifti(){}; 

        Nifti(){}; 

        void Finish(); 
        void LoadFile(std::string fname); 

        float* GetBuffer();
        int GetBufferSize();

        std::vector<int> GetDimensions(); 
        std::vector<float> GetSpacing(); 

        float GetValue(int i, int j, int k, int l); 

        private : 
        int data_size = 0; 
        float* data_ptr = nullptr; 

        std::vector<float> spacing; 
        std::vector<int> dimensions; 

    }; 

} // namespace SpicyTech2


//--------------------------------------------------------------------------||--//
//--------------------------------------------------------------------------||--//