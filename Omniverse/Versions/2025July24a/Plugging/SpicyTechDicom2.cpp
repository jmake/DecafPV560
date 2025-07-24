#include "Logger.hpp" 

#include "SpicyTechDicom2.hpp"

#include "nifti2_io.h"  

#include "laynii_lib.h"

// SEE: https://examples.vtk.org/site/VTKBook/02Chapter2/
#include "vtktools2.hpp" 


//--------------------------------------------------------------------------||--//
//--------------------------------------------------------------------------||--//
float* NiftiReader(std::string fin_1, std::vector<int>& dimensions, std::vector<float>& spacing) 
{
    // Reading ... 
    nifti_set_debug_level(2); // flag 'HAVE_ZLIB' ->  zlib.h, zd.lib https://github.com/madler/zlib

    nifti_image* nii = nifti_image_read(fin_1.c_str(), 1);
    if (!nii) {
        fprintf(stderr, "[NiftiReader] ** failed to read NIfTI from '%s'\n", fin_1.c_str());
        return nullptr;
    }

    log_nifti_descriptives(nii); 


    // Extracting ... 
    const float dX = nii->pixdim[1];
    const float dY = nii->pixdim[2];
    const float dZ = nii->pixdim[3];
    spacing.push_back(dX); 
    spacing.push_back(dY); 
    spacing.push_back(dZ); 

    const uint32_t nt = nii->nt;
    const uint32_t nx = nii->nx;
    const uint32_t ny = nii->ny;
    const uint32_t nz = nii->nz;
    //const uint32_t nr_voxels = nz * ny * nx;
    dimensions.push_back(nx); 
    dimensions.push_back(ny); 
    dimensions.push_back(nz); 
    dimensions.push_back(nt); 

    // idx -> i + nx * (j + ny * (k + nz * l));
    nifti_image* nii_domain = copy_nifti_as_float32(nii);
    float* nii_domain_data = static_cast<float*>(nii_domain->data);
    
    return nii_domain_data;
}


//--------------------------------------------------------------------------||--//
//--------------------------------------------------------------------------||--//
namespace SpicyTech2 {

    //--------------------------------------------------------------------------||--//
    VtkTest::~VtkTest()
    {
    }; 

    VtkTest::VtkTest()
    {
        VersionShow(); 
    }; 


    void VtkTest::NumpyPrint(double* data, int size) // -> SEE : configuration.i
    {
        std::cout <<"[VtkTest] received : (";
        for (int i = 0; i < size; ++i) std::cout << data[i] <<",";
        std::cout <<") \n";
    }; 


    std::vector<double> VtkTest::GetData() 
    {
        std::vector<double> array = {0,1,2,3,4,5,6,7,8,9};
        return array; 
    }; 


} // namespace SpicyTech2

//--------------------------------------------------------------------------||--//
namespace SpicyTech2 {


    void Nifti::Finish() 
    {
        if(data_ptr)
        {
            data_ptr = nullptr; 
            data_size = 0; 
        }
    } 


    void Nifti::LoadFile(std::string fname)
    {
        std::cout <<"[Nifti] file : '"<< fname <<"' \n";

        spacing.clear(); 
        dimensions.clear(); 
        data_ptr = NiftiReader(fname, dimensions, spacing);  
        data_size = dimensions[0] * dimensions[1] * dimensions[2] * dimensions[3];

        std::cout <<"[Nifti] dimensions : ("
        << dimensions[0] <<","
        << dimensions[1] <<","
        << dimensions[2] <<","
        << dimensions[3] <<") -> "
        << data_size <<" \n";
    }


    int Nifti::GetBufferSize()
    {
        return data_size; 
    }


    float* Nifti::GetBuffer()
    {
        return data_ptr; 
    }


    float Nifti::GetValue(int i, int j, int k, int l)
    {
        // data_size = dimensions[0] * dimensions[1] * dimensions[2] * dimensions[3];
        int nx = dimensions[0]; 
        int ny = dimensions[1]; 
        int nz = dimensions[2]; 
        int nt = dimensions[3]; 
        int idx = i + nx * (j + ny * (k + nz * l));
        return data_ptr[idx]; 
    }


    std::vector<int> Nifti::GetDimensions()
    {
        return dimensions; 
    }


    std::vector<float> Nifti::GetSpacing()
    {
        return spacing; 
    }


} // namespace SpicyTech2


//--------------------------------------------------------------------------||--//
//--------------------------------------------------------------------------||--//