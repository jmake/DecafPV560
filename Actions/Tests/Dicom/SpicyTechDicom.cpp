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
//vtkSmartPointer<vtkImageData> 
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

    // Create vtkImageData
    //vtkSmartPointer<vtkImageData> image = vtkSmartPointer<vtkImageData>::New();
    vtkImageData* image = vtkImageData::New(); 

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
}


//--------------------------------------------------------------------------||--//
//vtkSmartPointer<vtkImageData> 
vtkImageData* NiftiReader(std::string fin_1) 
{
    /*
        - HAVE_ZLIB ->  zlib.h, zd.lib 
        https://github.com/madler/zlib
    */
    nifti_set_debug_level(2); 

    nifti_image* nii = nifti_image_read(fin_1.c_str(), 1);
    if (!nii) {
        fprintf(stderr, "[NiftiReader] ** failed to read NIfTI from '%s'\n", fin_1.c_str());
        //return 2;
    }

    log_nifti_descriptives(nii); 

    IterateNiftiData(nii); 
    return SaveVTIFromNifti(nii); 
    //vtkSmartPointer<vtkImageData> vti = SaveVTIFromNifti(nii); 
    //return vti; 
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
void _Cutter(
                    vtkDataObject *obj, 
                    std::vector<double> orig, 
                    std::vector<double> normal, 
                    std::string key
                 ) 
{
    // C# : 
    //      int[] triangles
    //  Vector3[] meshVertices = new Vector3[nbVertices];
    //
    //   indices -> [ v11,v12,...,v1M, v21,v22,....v2M, ..., vN1,vN2,...,vNM ];
    // triangles -> [ v11,v12,v13, v21,v22,v23, ..., vN1,vN2,vN3 ]; 
    //  vertices -> { {x1,y1,z1}, {x2,y2,z2}, ..., {xM,yM,zM} };
    // 
    normal = {1.0, 0.0, 0.0}; 
    orig = GetGeometricCenter(obj); 

    vtkDataObject *cutter = CutterPlane(obj, orig, normal); 
    vtkPolyData* vtp = static_cast<vtkPolyData*>(cutter);

    int n_rows = -1; 
    int n_cols = -1; 
    std::vector<float> vertices = GetCppArray<float>( vtp->GetPoints()->GetData(), &n_rows, &n_cols); 

    int n_indices = -1; 
    std::vector<int> indices = GetFlatCellIndices( vtp, n_indices );


    vtkPointData* pointData = vtp->GetPointData(); 
    std::vector<std::string> names = GetArrayNames(pointData); 

    //std::string key; 
    if( Contains(names, key) )
    {
        vtkDataArray* array = GetPointDataArray(pointData, key);  
        std::vector<float> vertices = GetCppArray<float>(array, &n_rows, &n_cols); 
    }

}


class Slicer  
{
    public :
    ~Slicer()
    {
        if(vtp) 
        {
            vtp->Delete();
            vtp = nullptr; 
        } 

        if(vti) 
        {
            vti->Delete();
            vti = nullptr; 
        } 
        
        indices.clear(); 
        property.clear(); 
        vertices.clear(); 
    }


    Slicer()
    {
        vti = nullptr; 
        vtp = nullptr; 
        array = nullptr; 

        indices = {}; 
        property = {}; 
        vertices = {}; 
    }


    void Create(vtkDataObject *obj, std::string prop)
    {
        vti = static_cast<vtkImageData*>(obj);        
        key = prop; 

        //std::vector<double> r0 = GetGeometricCenter(obj); 
        //std::vector<double> n0 = {1.0, 0.0, 0.0}; 

        orig = GetGeometricCenter(obj); 
        __Update__({1.0,0.0,0.0}, orig);
    }


    void Update(std::vector<double> n0)
    {
        __Update__(n0, orig);
    } 

    
    void __Update__(
            std::vector<double> normal, 
            std::vector<double> orig 
        )
    {
        //orig = r0; 
        //normal = n0; 

        vtkDataObject *cutter = CutterPlane(vti, orig, normal); 
        vtp = static_cast<vtkPolyData*>(cutter);

        vtkPointData* pointData = vtp->GetPointData(); 

        std::vector<std::string> names;  
        names = GetArrayNames(pointData); 

        if( Contains(names, key) )
        {
            array = GetPointDataArray(pointData, key);  
        }

        DataGet(); 
        GeometryGet(); 
    }


    void DataGet()
    {
        if( vtp && array )
        {
            int n_rows = -1; 
            int n_cols = -1; 

            property = GetCppArray<float>(array, &n_rows, &n_cols); 
            std::cout << "\t [Slicer] key:'"<< array->GetName() <<"' n_rows : "<< n_rows <<" n_cols: "<< n_cols <<"\n";        
        }
    }


    void GeometryGet()
    {
        // C# : 
        //      int[] triangles
        //  Vector3[] meshVertices = new Vector3[nbVertices];
        //
        //   indices -> [ v11,v12,...,v1M, v21,v22,....v2M, ..., vN1,vN2,...,vNM ];
        // triangles -> [ v11,v12,v13, v21,v22,v23, ..., vN1,vN2,vN3 ]; 
        //  vertices -> { {x1,y1,z1}, {x2,y2,z2}, ..., {xM,yM,zM} };
        // 
        if( vtp && array )
        {
            int n_rows = -1; 
            int n_cols = -1; 
            vertices = GetCppArray<float>( vtp->GetPoints()->GetData(), &n_rows, &n_cols); 
            std::cout << "\t [Slicer] n_rows : "<< n_rows <<" n_cols: "<< n_cols <<"\n";

            int n_indices = -1; 
            indices = GetFlatCellIndices( vtp, n_indices );
            std::cout << "\t [Slicer] n_indices : "<< n_indices <<" \n";
        }

    }    


    void Save(std::string fname)
    {
        if( vtp && array )
        {
            PWriterSerial(vtp, fname); 
        }        
    }


    private : 
    std::string key;  
    std::vector<double> orig;  
    //std::vector<double> normal;  

    vtkPolyData* vtp; 
    vtkImageData* vti; 
    vtkDataArray* array; 

    std::vector<int> indices; 
    std::vector<float> property; 
    std::vector<float> vertices; 
}; 


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
    }


    VtkGrid()
    {
        this->nDim    = -1 ;
        this->nCells  = -1 ;  
        this->nPts    = -1 ;  
        this->obj     = nullptr;

        this->slicer = new Slicer(); 
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


    void CutterCreate(std::string key) //, std::vector<double> orig, std::vector<double> normal) 
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
        //std::vector<double> r0 = {x, y, z};

        slicer->Update(n0); 
    }

/*
    void PlaneGet() 
    {
        std::vector<double> normal = {1.0, 0.0, 0.0}; 

        std::vector<double> orig = GetGeometricCenter(obj); 
        PrintVector(orig); 

        vtkDataObject *cutter = CutterPlane(obj, orig, normal); 

        std::vector<std::vector<double>> coords = GetCoords(cutter);
        std::cout << "\t [cutter] n_coords : "<< coords.size() <<" \n";

        std::vector<unsigned char> cellTypes;
        std::vector<std::vector<vtkIdType>> cellVertices;
        GetCellsList(cutter, cellVertices, cellTypes);  

        std::string fname = "cutter"; 
        PWriterSerial(cutter, fname); 
    }
*/

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

        Slicer* slicer; 
        vtkGridType* obj; 

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


} // SpicyTech


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