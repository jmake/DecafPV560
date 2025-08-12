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
        version = VersionShow(); 
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


    void VtkTest::_LoadFile()
    {
        std::string fname; 
        std::vector<int> dimensions;
        std::vector<float> spacing; 
        NiftiReader(fname, dimensions, spacing);  
    }


} // VtkTest, namespace SpicyTech2
 

//--------------------------------------------------------------------------||--//
//-------------------------------------------------------------------| Nifti |--//
/*
vtkImageData* CreateImageFromBuffer(float* voxels, const std::string& key,
                                    const std::vector<int>& dimensions,
                                    const std::vector<float>& spacing)
{
    int nx = dimensions[0];
    int ny = dimensions[1];
    int nz = dimensions[2];

    vtkFloatArray* vtk_array = vtkFloatArray::New();
    vtk_array->SetNumberOfComponents(1);
    vtk_array->SetNumberOfTuples(nx * ny * nz);
    vtk_array->SetVoidArray(voxels, nx * ny * nz, 1);
    vtk_array->SetName(key.c_str());

    vtkImageData* image = vtkImageData::New();
    image->SetDimensions(nx, ny, nz);
    image->SetSpacing(spacing[0], spacing[1], spacing[2]);
    image->GetPointData()->SetScalars(vtk_array);

    vtk_array->Delete();

    return image;
}


template <class T>
void GetRange(std::vector<T> property, std::vector<T>& range)
{
    std::pair<std::vector<T>::const_iterator, std::vector<T>::const_iterator> result;  

    result = std::minmax_element(property.begin(), property.end());
    range = {*result.first, *result.second};

    std::cout <<"\t [GetRange] range["<< key << "] : (" << range[0] <<", "<< range[1] <<") \n";
    //std::copy(range.data(), range.data() + range.size(), _range);  
}



template <class T>
std::vector<T> GetRange1(const T* property, int length)
{

    if (property == nullptr || length <= 0) {
        std::cerr << "\t[GetRange] Invalid input.\n";
        return {};
    }

    auto result = std::minmax_element(property, property + length);
    std::vector<T> range = { *result.first, *result.second };

    std::cout << "\t[GetRange] range : (" << range[0] << ", " << range[1] << ")\n";
    return range;
}

namespace SpicyTech3 {

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

        range = GetRange1<float>(data_ptr, data_size);

        key = "voxels"; 
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


    void* Nifti::GetVti() 
    {
        vti = CreateImageFromBuffer(data_ptr, 
                                    key,
                                    dimensions,
                                    spacing); 
        return vti; 
    }


    void Nifti::SaveVti(std::string fname)
    {
        this->GetVti(); 

        if(vti == nullptr) return ;
        PWriterSerial(static_cast<vtkImageData*>(vti), fname); 
    }


    std::string Nifti::GetKeyName()
    {
        return key; 
    }


    std::vector<float> Nifti::GetRange() 
    {

        //if(vti == nullptr) return ;
        //vtkPointData *pd = vti->GetPointData(); 
        //int length = 0; 
        //double* ptr = nullptr;
        //ptr = ArrayGet3<double>(pd, key, length); 
        //GetRange(property, range);
        return range; 
    }

} // Nifti, namespace SpicyTech3
*/

//--------------------------------------------------------------------------| Contour3 |--//
/*
std::string SuggestTypeName()
{
    if (sizeof(vtkIdType) == sizeof(int))
        return "int";
    else if (sizeof(vtkIdType) == sizeof(long))
        return "long";
    else if (sizeof(vtkIdType) == sizeof(long long))
        return "long long";
    else
        return "vtkIdType (platform-specific)";
}


namespace SpicyTech2 {

    Contour3::~Contour3() 
    {
    }


    Contour3::Contour3() 
    {
        vti = nullptr; 
        vtp = nullptr; 
    }
    

    void Contour3::Finish() 
    {
        if(vtp)
        {
            static_cast<vtkPolyData*>(vtp)->Delete(); 
            vtp = nullptr; 
        } 
    }

    void Contour3::SetVti(void* obj, std::string key, std::vector<float> range, double maxCells)
    {
        this->vti = obj; //static_cast<vtkImageData*>(obj);
        this->key = key; 
        this->range = range; 
        this->maxCells = maxCells; 

        std::cout << "\t+[Contour3] VtiSet : '" << static_cast<vtkPolyData*>(vti)->GetClassName() <<"' Ready! \n";
    }


    void Contour3::Update(double u0, bool centered)
    {
        if(vti == nullptr) return; 

        if(vtp) static_cast<vtkPolyData*>(vtp)->Delete(); 

        double threshold = MapValue(u0, 1e-6, 1.0, range[0], range[1]); 

        bool getscalars = true; 
        vtkDataObject* obj = GetContour(static_cast<vtkImageData*>(vti), key, threshold, getscalars); 

        if(centered) obj = CenterAtOrigin(obj); 

        //vtp = static_cast<vtkPolyData*>(obj); 
        vtp = obj; 
    }


    void Contour3::SaveVtp(std::string fname)
    {
        if(vtp == nullptr) return ;

        PWriterSerial(static_cast<vtkPolyData*>(vtp), fname); 
    }


    float* Contour3::GetVertices(int& n_rows, int& n_cols)
    {
        if(vtp == nullptr) return nullptr; 

        vtkPoints* pts = static_cast<vtkPolyData*>(vtp)->GetPoints(); 
        return GetCppArray3<float>(pts->GetData(), &n_rows, &n_cols); 
    }


    //template<typename T> fails!!
    long long* Contour3::GetTriangles(int& n_indices, long long& n_raw)
    {
        if (vtp == nullptr) return nullptr;

        vtkCellArray* cells = static_cast<vtkPolyData*>(vtp)->GetPolys();
        if (cells == nullptr) return nullptr;

        n_indices = static_cast<vtkPolyData*>(vtp)->GetNumberOfCells();

        vtkIdTypeArray* data = cells->GetData();
        if (data == nullptr) return nullptr;

        if (data->GetNumberOfTuples() != static_cast<vtkPolyData*>(vtp)->GetNumberOfCells() * 4) {
            std::cerr << "[Contour3] Assertion failed: tuple count does not match expected layout\n";
            std::abort(); // o std::exit(EXIT_FAILURE);
        }

        vtkIdType* raw = data->GetPointer(0);
        n_raw = data->GetNumberOfTuples();
        //vtkIdType size = data->GetNumberOfTuples();
        //n_raw = static_cast<long long>(size);

        if (sizeof(vtkIdType) == sizeof(long long)) {
            return reinterpret_cast<long long*>(raw);
        } else {
            std::cerr << "\t[GetTriangles] Unsafe cast vtkIdType* to " 
                      << typeid(long long).name() << "* -> "
                      << "Suggestion: use '" << SuggestTypeName() << "' as template parameter instead.\n";
            return nullptr;
        }
    }

} // Contour3, namespace SpicyTech2
*/

//--------------------------------------------------------------------------||--//
//--------------------------------------------------------------------------||--//