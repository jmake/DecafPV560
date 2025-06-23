#include <string>
#include <iostream>

#include <vtkVersion.h>
#include <vtkSmartPointer.h>
#include <vtkDICOMImageReader.h>


#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkDataArray.h>
#include <vtkPointData.h>

#include "vtktools.hpp" 
#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>


#include "SpicyTechDicom.hpp"

#include "nifti2_io.h"  
#include "laynii_lib.h"


//--------------------------------------------------------------------------||--//


//--------------------------------------------------------------------------||--//
vtkSmartPointer<vtkImageData> SaveVTIFromNifti(nifti_image* nii)
{
    std::cout << "[SaveVTIFromNifti] ...\n";

    const uint32_t nt = nii->nt;
    const uint32_t nx = nii->nx;
    const uint32_t ny = nii->ny;
    const uint32_t nz = nii->nz;

    const uint32_t nr_voxels = nz * ny * nx;

    nifti_image* nii_domain = copy_nifti_as_float32(nii);
    float* nii_domain_data = static_cast<float*>(nii_domain->data);

    // Create VTK float array (column-major)
    vtkSmartPointer<vtkFloatArray> vtk_data = vtkSmartPointer<vtkFloatArray>::New();
    vtk_data->SetNumberOfComponents(1);
    vtk_data->SetNumberOfTuples(nr_voxels);

    for (int t = 0; t < nt; ++t)
        for (int z = 0; z < nz; ++z)
            for (int y = 0; y < ny; ++y)
                for (int x = 0; x < nx; ++x) 
                {
                    int idx = x + nx * (y + ny * (z + nz * t));
                    float val = nii_domain_data[idx]; 
                    vtk_data->SetValue(idx++, val);
                }

    // Create vtkImageData
    vtkSmartPointer<vtkImageData> image = vtkSmartPointer<vtkImageData>::New();

    const float dX = nii->pixdim[1];
    const float dY = nii->pixdim[2];
    const float dZ = nii->pixdim[3];

    image->SetDimensions(nx, ny, nz);
    image->SetSpacing(dX, dY, dZ);
    image->GetPointData()->SetScalars(vtk_data);

    PWriterSerial(image, "nifti"); 

    std::cout << "[SaveVTIFromNifti] Done!!\n";
    return image;
}

//--------------------------------------------------------------------------||--//
void IterateNiftiData(nifti_image* nii1) 
{
    const uint32_t size_x = nii1->nx;
    const uint32_t size_y = nii1->ny;
    const uint32_t size_z = nii1->nz;

    const uint32_t end_x = size_x - 1;
    const uint32_t end_y = size_y - 1;
    const uint32_t end_z = size_z - 1;

    const uint32_t nr_voxels = size_z * size_y * size_x;

    const float dX = nii1->pixdim[1];
    const float dY = nii1->pixdim[2];
    const float dZ = nii1->pixdim[3];

    nifti_image* nii_domain = copy_nifti_as_int32(nii1);
    int32_t* nii_domain_data = static_cast<int32_t*>(nii_domain->data);
    // Binarize
    for (uint32_t i = 0; i != nr_voxels; ++i) {
        if (*(nii_domain_data + i) != 0) {
            *(nii_domain_data + i) = 1;
        } else {
            *(nii_domain_data + i) = 0;
        }
    }

    std::cout << "[IterateNiftiData] "<< nr_voxels <<"  \n";
/*    
    int nx = nii->nx, ny = nii->ny, nz = nii->nz, nt = nii->nt;
    int nvox = nii->nvox;

    int idx = -1 ;
    float* data = static_cast<float*>(nii->data);
    for (int t = 0; t < nt; ++t)
        for (int z = 0; z < nz; ++z)
            for (int y = 0; y < ny; ++y)
                for (int x = 0; x < nx; ++x) 
                {
                    idx = x + nx * (y + ny * (z + nz * t));
                    float val = data[idx];
//std::cout << idx <<" /  "<< z <<" "<< y <<" "<< x <<" "<< nvox << " "<< val <<" \n";

                }
*/
}


//--------------------------------------------------------------------------||--//
int NiftiReader(std::string fin_1) 
{
    /*
        - HAVE_ZLIB ->  zlib.h, zd.lib 
        https://github.com/madler/zlib
    */
    nifti_set_debug_level(2); 

    nifti_image* nii = nifti_image_read(fin_1.c_str(), 1);
    if (!nii) {
        fprintf(stderr, "[NiftiReader] ** failed to read NIfTI from '%s'\n", fin_1.c_str());
        return 2;
    }

    log_nifti_descriptives(nii); 

    //nii->data[i]; 
    IterateNiftiData(nii); 
    SaveVTIFromNifti(nii); 

    //log_nifti_descriptives(nii); 

    return 1; 
}


//--------------------------------------------------------------------------||--//



//--------------------------------------------------------------------------||--//
//--------------------------------------------------------------------------||--//
void VersionShow()
{
    std::cout << "[SpicyTech] VTK version: " << vtkVersion::GetVTKVersion() << std::endl;
}

//--------------------------------------------------------------------------||--//
vtkSmartPointer<vtkImageData> 
ReadDICOMSeries(const std::string& path) 
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


//--------------------------------------------------------------------------||--//


//--------------------------------------------------------------------------||--//

template <class vtkGridType> 
class VtkGrid   
{
    public :
    ~VtkGrid()
    {        
    }

    VtkGrid()
    {
        this->nDim    = -1 ;
        this->nCells  = -1 ;  
        this->nPts    = -1 ;  
        this->obj     = nullptr;

        //Warning("vtk.log"); 
    }


    vtkGridType* MeshGet()
    {
        return obj;
    }


    void MeshSave(std::string fname)
    {
        PWriterSerial(obj, fname); 
    }


    void MeshSet(vtkGridType* vtu)
    {
        nArrays = vtu->GetPointData()->GetNumberOfArrays();
        nCells  = vtu->GetNumberOfCells();
        nPts    = vtu->GetNumberOfPoints();
        obj     = vtu;

        // DicomTestImages4\series-000001 -> dims (512, 512, 361)
        std::cout<<" -    nPts:"<< nPts <<" \n"; // 94,633,984
        std::cout<<" -  nCells:"<< nCells <<" \n"; // 94,003,560
        std::cout<<" - nArrays:"<< nArrays <<" \n"; // 94,003,560

        PrintPointDataArrays( vtu->GetPointData() ); 
    }


    void PlaneGet() 
    {
        std::vector<double> normal = {1.0, 0.0, 0.0}; 

        std::vector<double> orig = GetGeometricCenter(obj); 
        PrintVector(orig); 

        vtkDataObject *cutter = CutterPlane(obj, orig, normal); 
        //vtkDataObject *cutter2 = Cutter(obj, GetFuntionPlane({2.0,0.0,0.01},{1.0,0.0,0.0}) ); assert(cutter2);

        std::vector<std::vector<double>> coords = GetCoords(cutter);
        std::cout << "\t [cutter] n_coords : "<< coords.size() <<" \n";

        std::vector<unsigned char> cellTypes;
        std::vector<std::vector<vtkIdType>> cellVertices;
        GetCellsList(cutter, cellVertices, cellTypes);  

        std::string fname = "cutter"; 
        PWriterSerial(cutter, fname); 
    }


    void SurfaceGet()
    {
        vtkPointData* pointData = obj->GetPointData(); 
        std::vector<std::string> names = GetArrayNames(pointData); 

        std::string key = "DICOMImage"; 
        if( Contains(names, key) )
        {
            vtkDataArray* array = GetPointDataArray(pointData, key);  

            std::vector<double> range(2);
            array->GetRange(range.data()); 
            std::cout << "\t range["<< key << "] : (" << range[0] <<", "<< range[1] <<") \n";
            
            double threshold = (range[0] + range[1]) * 0.5;
            vtkDataObject* contour = GetContour(obj, key, threshold); 

            std::vector<std::vector<double>> coords = GetCoords(contour);
            std::cout << "\t n_coords["<< key << "] : "<< coords.size() <<" \n";

            std::vector<unsigned char> cellTypes;
            std::vector<std::vector<vtkIdType>> cellVertices;
            GetCellsList(contour, cellVertices, cellTypes);  

            std::string fname = "contour"; 
            PWriterSerial(contour, fname); 
        }
        else 
        {
            PrintVector(names); 
        }
    }


    private: 
        int nDim ;
        int nCells ;
        int nPts ;
        int nArrays; 

        vtkGridType* obj; 

}; // VtkGrid


//--------------------------------------------------------------------------||--//
//--------------------------------------------------------------------------||--//
namespace SpicyTech {

    Dicom::~Dicom()
    {
        delete mesh; 
    }


    Dicom::Dicom()
    {
        VersionShow(); 

        mesh = new VtkGrid<vtkImageData>();
    }


    void Dicom::LoadNifti(std::string directory) 
    {
        NiftiReader(directory); 
    }


    void Dicom::LoadDicom(std::string directory) 
    {
        vtkSmartPointer<vtkImageData> vti; 
        vti = ReadDICOMSeries(directory); 

        mesh->MeshSet(vti); 
        mesh->MeshSave("test1"); 

        mesh->PlaneGet(); 
        mesh->SurfaceGet(); 
    }


    void Dicom::CutCreate() 
    {

    }


    void Dicom::TestVti(std::string directory) 
    {
        VtkGrid<vtkImageData> *m = new VtkGrid<vtkImageData>();
        vtkImageData* vti = ReadVTIFile(directory); 
        m->MeshSet(vti);
        delete m; 
    }


} // SpicyTech

//--------------------------------------------------------------------------||--//
//--------------------------------------------------------------------------||--//