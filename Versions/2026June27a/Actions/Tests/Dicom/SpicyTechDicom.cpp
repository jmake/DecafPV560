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
vtkImageData* Nifti2Vti(nifti_image* nii)
{
    std::cout << "[Nifti2Vti] ...\n";

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
    //PWriterSerial(image, fname); 

    std::cout << "[Nifti2Vti] Done!!\n";
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
    return Nifti2Vti(nii); 
}

//--------------------------------------------------------------------------||--//
//--------------------------------------------------------------------------||--//



//--------------------------------------------------------------------------||--//
//--------------------------------------------------------------------------||--//
/*
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


    void ContourUpdate(float u0, float umin, float umax)
    {
        contour->Update(u0, umin, umax); 
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


    private: 
    int nDim;
    int nPts;
    int nCells;
    int nArrays; 

    vtkGridType* obj; 
    ExtractorSlicer* slicer; 
    ExtractorContour* contour; 

}; // VtkGrid
*/
//--------------------------------------------------------------------------||--//
//--------------------------------------------------------------------------||--//
template <class vtkGridType>
class VtkCutterBase {
protected:
    ExtractorSlicer* slicer;
    vtkGridType* obj;

public:
    VtkCutterBase() : slicer(new ExtractorSlicer()), obj(nullptr) {}

    ~VtkCutterBase() {
        delete slicer;
    }

    void SetCutterObject(vtkGridType* o) { obj = o; }

    void CutterCreate(const std::string& key) {
        slicer->Create(obj, key);
    }

    void CutterSave(const std::string& fname) {
        slicer->Save(fname);
    }

    void CutterUpdate(float x, float y, float z, float nx, float ny, float nz) {
        std::vector<double> n0 = {nx, ny, nz}; 
        std::vector<double> r0 = {x, y, z};
        slicer->Update(n0);
    }

    
    std::vector<int>   CutterIndices() { return slicer->indices; }
    std::vector<float> CutterVertices() { return slicer->vertices; }
    std::vector<float> CutterProperty() { return slicer->property; }

};


//--------------------------------------------------------------------------||--//
template <class vtkGridType>
class VtkContourBase {
protected:
    ExtractorContour* contour;
    vtkGridType* obj;

public:
    VtkContourBase() : contour(new ExtractorContour()), obj(nullptr) {}

    ~VtkContourBase() {
        delete contour;
    }

    void SetContourObject(vtkGridType* o) { obj = o; }

    void ContourCreate(const std::string& key, float maxCells) {
        contour->Create(obj, key, maxCells);
    }

    void ContourSave(const std::string& fname) {
        contour->Save(fname);
    }

    void ContourUpdate(float u0, float umin, float umax) {
        contour->Update(u0, umin, umax);
    }

    std::vector<int> ContourIndices() { return contour->indices; }
    std::vector<float> ContourVertices() { return contour->vertices; }
    std::vector<float> ContourProperty() { return contour->property; }
};


//--------------------------------------------------------------------------||--//
template <class vtkGridType>
class VtkGrid : public VtkCutterBase<vtkGridType>, public VtkContourBase<vtkGridType> {
public:
    VtkGrid() {
        nDim = nPts = nCells = nArrays = -1;
        obj = nullptr;
    }

    ~VtkGrid() {
        if (obj) {
            obj->Delete();
            obj = nullptr;
        }
    }

    vtkGridType* MeshGet() { return obj; }

    void MeshSet(vtkGridType* vtu) {
        nArrays = vtu->GetPointData()->GetNumberOfArrays();
        nCells  = vtu->GetNumberOfCells();
        nPts    = vtu->GetNumberOfPoints();

        std::cout << " -    nPts: " << nPts << "\n";
        std::cout << " -  nCells: " << nCells << "\n";
        std::cout << " - nArrays: " << nArrays << "\n";

        PrintPointDataArrays(vtu->GetPointData());

        this->SetCutterObject(vtu);
        this->SetContourObject(vtu);
        obj = vtu;
    }

    void MeshSave(const std::string& fname) {
        PWriterSerial(obj, fname);
    }

protected:
    int nDim, nPts, nCells, nArrays;
    vtkGridType* obj;
};



//--------------------------------------------------------------------------||--//
namespace SpicyTech {

    ////////////////////////////////////////////////////
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


    void Nifti::LoadFile(std::string directory, bool centered) 
    {
        vtkImageData* vti = NiftiReader(directory); 

        if(centered) CenterImageAtOrigin(vti); 

        manager->MeshSet(vti); 
    }


    void Nifti::VtiSave(std::string fname) 
    {
        manager->MeshSave(fname); 
    }


    ////////////////////////////////////////////////////
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


    void Nifti::CutDimensions(int* sizes)
    {
        if(!manager) return; 
        sizes[0] = manager->CutterIndices().size(); 
        sizes[1] = manager->CutterVertices().size(); 
        sizes[2] = manager->CutterProperty().size(); 
    } 


    void Nifti::CutGeometry(int* indices, float* vertices, float* property)
    {
        if(!manager) return; 

        const auto& src1 = manager->CutterIndices();
        std::copy(src1.data(), src1.data() + src1.size(), indices);  

        const auto& src2 = manager->CutterVertices();
        std::copy(src2.data(), src2.data() + src2.size(), vertices);  

        const auto& src3 = manager->CutterProperty();
        std::copy(src3.data(), src3.data() + src3.size(), property);  
    } 

    ////////////////////////////////////////////////////
    void Nifti::ContourCreate(std::string key, float maxCells) 
    {
        if(!manager) return; 
        manager->ContourCreate(key, maxCells); 
    }


    void Nifti::ContourSave(std::string fname) 
    {
        if(!manager) return; 
        manager->ContourSave(fname); 
    }


    void Nifti::ContourUpdate(float u0, float umin, float umax)
    {
        if(!manager) return; 
        manager->ContourUpdate(u0, umin, umax); 
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
        //mesh->MeshSave("test1"); 
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