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

#include "SpicyTechDicom.hpp"


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
//namespace SpicyTech {

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

//} // SpicyTech


//--------------------------------------------------------------------------||--//
//--------------------------------------------------------------------------||--//