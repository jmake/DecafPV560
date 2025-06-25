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
vtkImageData* SaveVTIFromNifti(nifti_image* nii)
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
    vtk_data->SetName("voxel");

    for (int t = 0; t < nt; ++t)
        for (int z = 0; z < nz; ++z)
            for (int y = 0; y < ny; ++y)
                for (int x = 0; x < nx; ++x) 
                {
                    int idx = x + nx * (y + ny * (z + nz * t));
                    float val = nii_domain_data[idx]; 
                    vtk_data->SetValue(idx++, val);
                }

    vtkImageData* image = vtkImageData::New(); 

    const float dX = nii->pixdim[1];
    const float dY = nii->pixdim[2];
    const float dZ = nii->pixdim[3];

    image->SetDimensions(nx, ny, nz);
    image->SetSpacing(dX, dY, dZ);
    image->GetPointData()->SetScalars(vtk_data);


    //RunKMeansOnImageDataPoints(image, 2); 
    PWriterSerial(image, "nifti"); 

    std::cout << "[SaveVTIFromNifti] Done!!\n";
    return image;
}


vtkImageData* NiftiReader(std::string fin_1) 
{
    // flag 'HAVE_ZLIB' ->  zlib.h, zd.lib https://github.com/madler/zlib
    nifti_set_debug_level(2); 

    nifti_image* nii = nifti_image_read(fin_1.c_str(), 1);
    if (!nii) {
        fprintf(stderr, "[NiftiReader] ** failed to read NIfTI from '%s'\n", fin_1.c_str());
        return nullptr;
    }

    log_nifti_descriptives(nii); 
    return SaveVTIFromNifti(nii); 
}

//--------------------------------------------------------------------------||--//
//--------------------------------------------------------------------------||--//



//--------------------------------------------------------------------------||--//
//--------------------------------------------------------------------------||--//
template <class vtkGridType> 
class VtkGrid   
{
    public :
    ~VtkGrid()
    {
        if(obj)
        {
            obj->Delete(); 
            obj = nullptr; 
        } 

        if(slicer)
        {
            delete slicer; 
            slicer = nullptr; 
        }

        if(contour)
        {
            delete contour; 
            contour = nullptr; 
        }
    }


    VtkGrid()
    {
        this->nDim    = -1 ;
        this->nCells  = -1 ;  
        this->nPts    = -1 ;  
        this->obj     = nullptr;

        this->slicer = new ExtractorSlicer(); 
        this->contour = new ExtractorContour(); 
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

        std::cout<<" -    nPts:"<< nPts <<" \n"; 
        std::cout<<" -  nCells:"<< nCells <<" \n"; 
        std::cout<<" - nArrays:"<< nArrays <<" \n"; 
        PrintPointDataArrays( vtu->GetPointData() ); 

        obj = vtu;
    }


    void CutterCreate(std::string key) 
    {
        slicer->Create(obj, key); 
    } 


    void CutterSave(std::string fname)
    {
        slicer->Save(fname); 
    }


    void CutterUpdate(float x, float y, float z, float nx, float ny, float nz)
    {
        std::vector<double> n0 = {nx, ny, nz}; 
        std::vector<double> r0 = {x, y, z};
        slicer->Update(n0); 
    }


    void ContourCreate(std::string key) 
    {
        contour->Create(obj, key); 
    } 


    void ContourSave(std::string fname)
    {
        contour->Save(fname); 
    }


    void ContourUpdate(float threshold)
    {
        contour->Update(threshold); 
    }


    std::vector<int> ContourIndices()
    {
        return contour->indices;
    } 

    std::vector<float> ContourVertices()
    {
        return contour->vertices;
    } 

    std::vector<float> ContourProperty()
    {
        return contour->property;
    } 


    /*
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
    */

    private: 
    int nDim;
    int nPts;
    int nCells;
    int nArrays; 

    vtkGridType* obj; 
    ExtractorSlicer* slicer; 
    ExtractorContour* contour; 

}; // VtkGrid


//--------------------------------------------------------------------------||--//
namespace SpicyTech {

    Nifti::~Nifti()
    {
        if(manager)
        {
            delete manager; 
        }
    }


    Nifti::Nifti()
    {
        VersionShow(); 

        manager = new VtkGrid<vtkImageData>();
    }


    void Nifti::LoadFile(std::string directory) 
    {
        vtkImageData* vti = NiftiReader(directory); 
        manager->MeshSet(vti); 
    }


    void Nifti::CutCreate(std::string key) 
    {
        if(!manager) return; 
        manager->CutterCreate(key); 
    }


    void Nifti::CutSave(std::string fname) 
    {
        if(!manager) return; 
        manager->CutterSave(fname); 
    }


    void Nifti::CutUpdate(float x, float y, float z, float nx, float ny, float nz)
    {
        if(!manager) return; 
        manager->CutterUpdate(x,y,z, nx,ny,nz); 
    }


    void Nifti::ContourCreate(std::string key) 
    {
        if(!manager) return; 
        manager->ContourCreate(key); 
    }


    void Nifti::ContourSave(std::string fname) 
    {
        if(!manager) return; 
        manager->ContourSave(fname); 
    }


    void Nifti::ContourUpdate(float threshold)
    {
        if(!manager) return; 
        manager->ContourUpdate(threshold); 
    }

    
    void Nifti::ContourDimensions(int* sizes)
    {
        if(!manager) return; 
        sizes[0] = manager->ContourIndices().size(); 
        sizes[1] = manager->ContourVertices().size(); 
        sizes[2] = manager->ContourProperty().size(); 
    } 


    void Nifti::ContourGeometry(int* indices, float* vertices, float* property)
    {
        if(!manager) return; 

        const auto& src1 = manager->ContourIndices();
        std::copy(src1.data(), src1.data() + src1.size(), indices);  

        const auto& src2 = manager->ContourVertices();
        std::copy(src2.data(), src2.data() + src2.size(), vertices);  

        const auto& src3 = manager->ContourProperty();
        std::copy(src3.data(), src3.data() + src3.size(), property);  

    } 


} // SpicyTech


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


    void Dicom::LoadFile(std::string directory) 
    {
        vtkSmartPointer<vtkImageData> vti; 
        vti = ReadDICOMSeries(directory); 

        mesh->MeshSet(vti); 
        mesh->MeshSave("test1"); 

        //mesh->PlaneGet(); 
        //mesh->SurfaceGet(); 
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