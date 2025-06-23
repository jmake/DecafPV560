//#pragma once 
#ifndef VTK_TOOLS_H  
#define VTK_TOOLS_H 

#include <iostream> // cin, cout, endl, cerr
#include <vector>   // vector
#include <map>

#include <assert.h>   
#include <algorithm>    // std::min_element, std::max_element, std::sort, std::find  

#include <vtkDataArray.h> 
#include <vtkPointData.h>
#include <vtkPoints.h> 

//#include <vtkMPIController.h>
#include <vtkAlgorithmOutput.h>

#include <vtkXMLPDataWriter.h>
#include <vtkXMLPPolyDataWriter.h>
#include <vtkXMLPUnstructuredGridWriter.h> 

#include <vtkCompositeDataIterator.h>
#include <vtkCompositeDataSet.h>
#include <vtkDataObject.h>

#include <vtkFileOutputWindow.h>

#include <vtkExtractGeometry.h>
#include <vtkImplicitFunction.h>

#include <vtkFileOutputWindow.h>

#include <vtkXMLWriter.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkXMLImageDataWriter.h>
#include <vtkXMLUnstructuredGridWriter.h>

#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkCompleteArrays.h>

#include <vtkBox.h>
#include <vtkPlane.h>
#include <vtkCutter.h>
#include <vtkContourFilter.h>

#include <vtkPolyData.h>
#include <vtkImageData.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>

#include <vtkXMLImageDataReader.h>

#include <vtkDataSet.h>
#include <vtkBoundingBox.h>


//--------------------------------------------------------------------------||--//
//--------------------------------------------------------------------------||--//
#include <vtkDataObject.h>
#include <vtkCellType.h>
#include <vtkCell.h>
#include <vtkIdList.h>
#include <vtkSmartPointer.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPolyData.h>

#include <vector>
#include <unordered_map>
#include <iostream>
#include <string>

std::string GetCellTypeName(unsigned char type)
{
    switch (type)
    {
    case VTK_TRIANGLE:    return "VTK_TRIANGLE";
    case VTK_QUAD:        return "VTK_QUAD";
    case VTK_TETRA:       return "VTK_TETRA";
    case VTK_PYRAMID:     return "VTK_PYRAMID";
    case VTK_WEDGE:       return "VTK_WEDGE";
    case VTK_HEXAHEDRON:  return "VTK_HEXAHEDRON";
    case VTK_POLYHEDRON:  return "VTK_POLYHEDRON";
    case VTK_VOXEL:       return "VTK_VOXEL";
    default:              return "???";
    }
}

void GetCellsList(vtkDataObject* obj,
                  std::vector<std::vector<vtkIdType>>& cellVertices,
                  std::vector<unsigned char>& cellTypes)
{
    vtkUnstructuredGrid* ugrid = vtkUnstructuredGrid::SafeDownCast(obj);
    vtkPolyData* poly = vtkPolyData::SafeDownCast(obj);

    if (!ugrid && !poly)
    {
        std::cout << "[GetCellsList] Unsupported vtkDataObject type.\n";
        return;
    }

    vtkIdType nCells = ugrid ? ugrid->GetNumberOfCells() : poly->GetNumberOfCells();

    std::unordered_map<unsigned char, int> typeCount;

    for (vtkIdType i = 0; i < nCells; ++i)
    {
        vtkSmartPointer<vtkIdList> ids = vtkSmartPointer<vtkIdList>::New();
        if (ugrid)
            ugrid->GetCellPoints(i, ids);
        else
            poly->GetCellPoints(i, ids);

        std::vector<vtkIdType> cell(ids->GetNumberOfIds());
        for (vtkIdType j = 0; j < ids->GetNumberOfIds(); ++j)
            cell[j] = ids->GetId(j);

        cellVertices.push_back(cell);

        unsigned char cellType = ugrid ? ugrid->GetCellType(i) : poly->GetCellType(i);
        cellTypes.push_back(cellType);

        typeCount[cellType]++;
    }

    std::cout << "[GetCellsList] Found Types: ";
    for (std::unordered_map<unsigned char, int>::const_iterator it = typeCount.begin(); it != typeCount.end(); ++it)
    {
        std::cout << GetCellTypeName(it->first) << " " << it->second  <<" ";
    }
    std::cout << std::endl;
}





//--------------------------------------------------------------------------||--//
#include <vtkPointSet.h>
#include <vtkDataObject.h>
#include <vtkDataArray.h>
#include <vector>

std::vector<std::vector<double>> GetCoords(vtkDataObject* obj)
{
    vtkPointSet* pointSet = vtkPointSet::SafeDownCast(obj);
    if (!pointSet) return {};

    vtkDataArray* data = pointSet->GetPoints()->GetData();
    if (!data) return {};

    vtkIdType numPoints = data->GetNumberOfTuples();
    int numComponents = data->GetNumberOfComponents();

    std::vector<std::vector<double>> coords(numPoints, std::vector<double>(numComponents));

    for (vtkIdType i = 0; i < numPoints; ++i)
        data->GetTuple(i, coords[i].data());

    return coords;
}



//--------------------------------------------------------------------------||--//
//--------------------------------------------------------------------------||--//
void VtkWarning( std::string fname )
{
  vtkFileOutputWindow *outwin = vtkFileOutputWindow::New();
  outwin->SetFileName( fname.c_str() );    
  outwin->SetInstance(outwin);
}


template <typename T>
void PrintVector(const std::vector<T>& vec)
{
    std::cout << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i];
        if (i + 1 < vec.size())
            std::cout << ", ";
    }
    std::cout << "]" << std::endl;
}


std::vector<double> GetGeometricCenter(vtkDataObject* obj)
{
    auto data = vtkDataSet::SafeDownCast(obj);
    assert(data);

    double bounds[6];
    data->GetBounds(bounds); // xmin, xmax, ymin, ymax, zmin, zmax

    std::vector<double> center(3);
    center[0] = 0.5 * (bounds[0] + bounds[1]);
    center[1] = 0.5 * (bounds[2] + bounds[3]);
    center[2] = 0.5 * (bounds[4] + bounds[5]);

    return center;
}


//--------------------------------------------------------------------------||--//
//--------------------------------------------------------------------------||--//
vtkImplicitFunction* GetFuntionPlane(std::vector<double> Orig, std::vector<double> Normal)
{
  vtkPlane *plane = vtkPlane::New();
  plane->SetOrigin(&Orig[0]);
  plane->SetNormal(&Normal[0]);
  return plane;
}


vtkDataObject* Cutter(vtkDataObject* Obj, vtkImplicitFunction* function)
{
  vtkCutter *cutter = vtkCutter::New();
  cutter->SetCutFunction( function );
  cutter->SetInputData( Obj );
  cutter->Update();
  return cutter->GetOutputDataObject(0);
}


vtkDataObject* CutterPlane(
                            vtkDataObject *obj, 
                            std::vector<double> orig, 
                            std::vector<double> normal
                          )
{
  vtkImplicitFunction* plane = GetFuntionPlane(orig, normal); 
  vtkDataObject *cutter = Cutter(obj, plane); 
  assert(cutter);
  return cutter;
}


//--------------------------------------------------------------------------||--//
//--------------------------------------------------------------------------||--//
vtkDataObject* GetContour(vtkDataObject *input, std::string key, double threshold)
{
//vtkSmartPointer<vtkContourFilter> contourFilter = vtkSmartPointer<vtkContourFilter>::New(); // SEGMENTATION!!
  vtkContourFilter* contourFilter = vtkContourFilter::New();
  contourFilter->SetInputData( input );
  contourFilter->SetValue(0,threshold);  // IsoSurface(0) = threshold
  contourFilter->SetInputArrayToProcess(0, 0, 0, 0, key.c_str() );

  contourFilter->GenerateTrianglesOn();
  contourFilter->ComputeScalarsOff();
  contourFilter->ComputeNormalsOff();
  contourFilter->Update();

  return contourFilter->GetOutputDataObject(0);
/*
  vtkDataObject *output = contourFilter->GetOutputDataObject(0); assert( output->IsA("vtkPolyData") );
  vtkPolyData   *poly   = vtkPolyData::SafeDownCast(output); assert(poly);

  poly->GetCellData()->RemoveArray("vtkOriginalCellIds");
  poly->GetFieldData()->RemoveArray("__CatalystChannel__");
  std::cout<<" GetNumberOfPoints "<< poly->GetNumberOfPoints() <<" \n";
 
  return poly;
*/
}

//--------------------------------------------------------------------------||--//
void PWriterSerial(vtkDataObject* Obj, const std::string& name) 
{
  std::cout << "[PWriterSerial] ...\n";

  assert(!Obj->IsA("vtkMultiBlockDataSet"));

  vtkXMLWriter* writer = nullptr;
  if (Obj->IsA("vtkPolyData")) writer = vtkXMLPolyDataWriter::New();
  else 
  if (Obj->IsA("vtkImageData")) writer = vtkXMLImageDataWriter::New();
  else 
  if (Obj->IsA("vtkUnstructuredGrid")) writer = vtkXMLUnstructuredGridWriter::New();

  if (writer) 
    std::cout << "[PWriterSerial] Saving '" << Obj->GetClassName() <<"' ...\n";
  else 
    std::cout << "[PWriterSerial] Unknown VTK data type: " << Obj->GetClassName() << std::endl;

  assert(writer);

  writer->SetDataModeToBinary();
  writer->SetInputData( Obj );

  std::ostringstream oss;
  oss << name << "." << writer->GetDefaultFileExtension();
  //std::cout << "\t [PWriterSerial] Saving '"<< oss.str() <<"' ... \n";  
  
  writer->SetFileName(oss.str().c_str());
  writer->Write();

  std::cout << "[PWriterSerial] '" << writer->GetFileName() << "' Saved!!\n";
  writer->Delete();
}



//--------------------------------------------------------------------------||--//
vtkImageData* ReadVTIFile(const std::string& path)
{
    //auto reader = vtkSmartPointer<vtkXMLImageDataReader>::New();
    vtkXMLImageDataReader *reader = vtkXMLImageDataReader::New(); 
    reader->SetFileName(path.c_str());
    reader->Update();

    vtkImageData* image = reader->GetOutput();
    std::cout << "GetNumberOfScalarComponents: " << image->GetNumberOfScalarComponents() << std::endl;

    double range[2];
    image->GetScalarRange(range);
    std::cout << "Scalar Range: [" << range[0] << ", " << range[1] << "]" << std::endl;

    int dims[3];
    image->GetDimensions(dims);
    std::cout << "Dimensions: [" << dims[0] << ", " << dims[1] << ", " << dims[2] << "]" << std::endl;

    return image;
}



//--------------------------------------------------------------------------||--//
//--------------------------------------------------------------------------||--//
std::vector<std::string> GetArrayNames(vtkPointData* pointData)
{
    std::vector<std::string> names;
    int numArrays = pointData->GetNumberOfArrays();

    for (int i = 0; i < numArrays; ++i) {
        const char* name = pointData->GetArrayName(i);
        if (name) {
            names.emplace_back(name);
        }
    }

    return names;
}


template <typename T>
bool Contains(const std::vector<T>& vec, const T& value)
{
    return std::find(vec.begin(), vec.end(), value) != vec.end();
}


vtkDataArray* GetPointDataArray(vtkPointData* pointData, const std::string& key) 
{
  int hasArray = pointData->HasArray(key.c_str());

  vtkDataArray* array = pointData->GetArray(key.c_str());

  if (!array) {
      std::cerr << "[GetPointDataArray] '" << key << "' not found!\n[GetPointDataArray] Available arrays:\n";
      for (int i = 0; i < pointData->GetNumberOfArrays(); ++i) {
          const char* name = pointData->GetArrayName(i);
          std::cerr << "\t'" << (name ? name : "(unnamed)") << "'\n";
      }
      exit(1);
  }

  return array;
}

//--------------------------------------------------------------------------||--//
void PrintPointDataArrays(vtkPointData* pointData) 
{
  int numArrays = pointData->GetNumberOfArrays();

  std::cout << "[PrintPointDataArrays] Found " << numArrays << " array(s):\n";
  for (int i = 0; i < numArrays; ++i) 
  {
      const char* name = pointData->GetArrayName(i);
      std::cout << "\t Array: '" << (name ? name : "(unnamed)") << "' \n";

      vtkDataArray* array = GetPointDataArray(pointData, name); 

      int nrows = array->GetNumberOfTuples(); 
      int ncols = array->GetNumberOfComponents();
      std::cout << "\t nrows: " << nrows <<" ndims:"<< ncols << "\n";

      std::vector<double> range(2);
      array->GetRange(range.data()); 
      std::cout << "\t range: (" << range[0] <<", "<< range[1] <<") \n";

      auto size = array->GetActualMemorySize(); 
      //std::cout << "\t size: " << size << "\n";

      for(int i=0; i< ncols; i++) // Tuples
      {
          std::vector<double> rows; 
          for(int j=0; j< nrows; j++) // Components
          {
              auto value = array->GetComponent(i,j); 
              rows.push_back(value); 
          }
          std::cout << "\t  idom: " << i <<" nrow: "<< rows.size() << "\n";
      }

      //ExtractArray(array);
  }
}



//--------------------------------------------------------------------------||--//
void PrintSelf(vtkDataObject *vtkDataObject)
{
  vtkDataObject->PrintSelf(std::cout,vtkIndent(2));
}


//--------------------------------------------------------------------------||--//
//--------------------------------------------------------------------------||--//
void PrintDataObjectName(vtkDataObject *vtkDataObject)
{
  std::cout<<" GetClassName:'"<< vtkDataObject->GetClassName() <<"' "<<std::endl;
}


vtkDataObject* ExtractBlock( vtkCompositeDataSet *composite ) 
{
  //vtkCompositeDataSet *composite = reader->GetOutput();
  vtkCompositeDataIterator* iter = composite->NewIterator();
  return iter->GetCurrentDataObject(); 
} 

//--------------------------------------------------------------------------||--//
//----------------------------------------|  FROM : E04_BOOST/nek5k01_01.cxx |--//
std::vector<double>
GetCppArray(vtkDataArray *vtk_array, int* rows=NULL, int* cols=NULL, std::string order="F")
{
  //assert(vtk_array);
  std::vector<double> cpp_array;  

  if(vtk_array) 
  { 
    int nDims = vtk_array->GetNumberOfComponents(); //assert(nDims==1); 
    int nRows = vtk_array->GetNumberOfTuples();

    if(cols) cols[0] = nDims;
    if(rows) rows[0] = nRows;

  //std::vector<double> cpp_array(nRows * nDims,0.0);
    cpp_array = std::vector<double>(nRows * nDims, 0.0);   

    /*  C++ Order 
    |--- nDims ---| _ _
    a1  b1 ... y1 z1  |  
    a2  b2 ... y2 z2  |  
    ...              nRows  ==>> [a1 b1 ... y1 z1 ... an bn ... yn zn]    
    an  bn ... yn zn _|_         |---------  nDims * nRows ----------|
    */
//    if(0)
    if(order == "C")
    for(int i=0,k=0; i<nRows; i++)
      for(int j=0; j<nDims; j++) cpp_array[k++] = vtk_array->GetComponent(i,j);

    /* Fotran order 
    |--- nDims ---| _ _
    a1  b1 ... y1 z1  |  
    a2  b2 ... y2 z2  |  
    ...              nRows  ==>> [a1 a2 ... an ...       z1 z2 ... zn]    
    a1n bn ... yn zn _|_         |---------  nDims * nRows ----------|
    */
//    if(1)
    if(order == "F")
    for(int j=0,k=0; j<nDims; j++)
      for(int i=0; i<nRows; i++) cpp_array[k++] = vtk_array->GetComponent(i,j);
  }

  return cpp_array;
}


//--------------------------------------------------------------------------||--//
std::vector<double> GetPointsArray(vtkDataObject *Obj, int print=1)
{
  // 
  // points  = [x1,y1,z1, x2,y2,z2, ..., xN,yN,zN]  
  //
  vtkPoints *points = NULL;
  if( Obj->IsA("vtkPolyData")         ) points = vtkPolyData::SafeDownCast(Obj)->GetPoints();
  if( Obj->IsA("vtkStructuredGrid")   ) points = vtkStructuredGrid::SafeDownCast(Obj)->GetPoints();  
  if( Obj->IsA("vtkUnstructuredGrid") ) points = vtkUnstructuredGrid::SafeDownCast(Obj)->GetPoints(); 
  assert(points); 

  int nPts=0, nCols=0;
  std::vector<double> array(0);
  if(points)
  {
    vtkDataArray *dataArray = points->GetData(); assert(dataArray);
    array = GetCppArray(dataArray, &nPts, &nCols, "C");
  }

  if(print)
  {
    for(int i=0,k=0; i<nPts; i++)
    {
      std::cout<<" "<< i <<" ["; 
      for(int j=0; j<nCols; j++, k++) std::cout<<" "<< array[k] ; 
      std::cout<<" ] \n"; 
    } 
  } 

//std::cout<<"[GetPointsArray] nPts:"<< nPts <<" \n";
  return array;
}



//--------------------------------------------------------------------------||--//

//--------------------------------------------------------------------------||--//
//--------------------------------------------------------------------------||--//
class Analysis
{
  public :
  Analysis(){}
 ~Analysis(){}

  void SetPoints(vtkPoints *points)
  {
    vtkDataArray *dataArray = points->GetData();
    Arrays["Coords"] = GetCppArray(dataArray, &rows, &cols);
    ArrayNames.push_back("Coords");
  }

  void SetArray(vtkPointData  *pointData, std::string key)
  {
    GetArrayNames(pointData);
    FindArrayName(key);

    vtkDataArray *dataArray = pointData->GetArray(key.c_str());
    Arrays[key] = GetCppArray(dataArray);
  }

  void GetArrayNames(vtkPointData *pointData)
  {
    if(ArrayNames.size()==1) // 'Coords' already added...
    {
      int nArrays = pointData->GetNumberOfArrays();
      for(int k=0; k<nArrays; k++) ArrayNames.push_back( pointData->GetArrayName(k) );
    }
  }

  bool FindArrayName(std::string key)
  {
    std::vector<std::string>::iterator it = std::find(ArrayNames.begin(), ArrayNames.end(), key);
    bool found = it != ArrayNames.end();
    if(!found)
    {
      std::cout<<"\t'"<< key <<"' Not Found!! \n" << std::endl;
      for(int k=0; k<ArrayNames.size(); k++) std::cout<<"\t  "<< k <<") '"<< ArrayNames[k] <<"' \n";
      exit(1);
    }
    return found;
  }

  void SaveArray(std::string fname, std::string key)
  {
      std::ofstream myfile;
      myfile.open (fname);
      assert( Arrays.find(key) != Arrays.end() );

      std::vector<double> Array( Arrays[key]  );

      size_t dims = (Array.size()==rows)?(1):( Array.size()/rows );

      std::vector<double*> ptr(dims,NULL);
      for(int i=0; i<dims; i++) ptr[i] = Array.data() + rows * i;

      for(int i=0; i<rows; i++)
      {
        myfile<< i  <<" ";
        for(int j=0; j<dims; j++)
        {
          myfile<< ptr[j][i] <<" ";
        }
        myfile<<"\n";
      }
      myfile<<" \n";

    myfile.close();
  }

  void PrintArrays(bool entire=true)
  {
    for(It=Arrays.begin(); It!=Arrays.end(); It++)
    {
      std::string name(It->first);
      std::vector<double> Array( It->second );

      size_t dims = (Array.size()==rows)?(1):( Array.size()/rows );
      std::cout<<"\t[PrintArrays] '"<< name << "': " << Array.size() <<" = "<< Array.size()/dims <<" x "<< dims <<"\n";

      std::vector<double*> ptr(dims,NULL);
      for(int i=0; i<dims; i++) ptr[i] = Array.data() + rows * i;

      if(entire) 
      for(int i=0; i<rows; i++)
      {
        std::cout<< i  <<") ";
        for(int j=0; j<dims; j++)
        {
          std::cout<< ptr[j][i] <<" ";
        }
        std::cout<<"\n";
      }
      std::cout<<" \n";

    } // for 
  }


  std::vector<double> GetComponent(std::string key, int icol)  
  {
    assert( FindArrayName(key) ); 

    std::vector<double> Array( Arrays[key] );  
    size_t ncols = (Array.size()==rows)?(1):( Array.size()/rows );
 /*
    std::cout<< "Array.size:" << Array.size() <<" \n";   
    std::cout<< "rows:" << rows <<" \n"; 
    std::cout<< "ncols:" << ncols <<" \n"; 
    std::cout<< "icol:" << icol <<" \n";
    std::cout<< "key:" <<  key <<" \n";
exit(0);
 */
    assert( icol + 1 <= ncols ); 

    double *begin = Array.data() + rows * (icol+0);
    double *end   = Array.data() + rows * (icol+1);

    std::vector<double> array(rows, std::numeric_limits<double>::max() ); 
    for(int i=0; i<rows; i++) array[i] = begin[i];//  Array.data() + rows * icol + i;
    return array;  
//    return std::vector<double>(begin,end); 
  }


  protected :
  std::vector<std::string> ArrayNames;
  std::map<std::string, std::vector<double> > Arrays;

  std::map<std::string, std::vector<double> >::iterator It;

  int rows, cols;

};
/*
  Analysis analysis = Analysis();
  analysis.SetPoints( polyData->GetPoints() );
  analysis.SetArray(  polyData->GetPointData(), "Pressure");
  //analysis.SetArray(  polyData->GetPointData(), "Velocity");
*/


//--------------------------------------------------------------------------||--//
//--------------------------------------------------------------------------||--//




//--------------------------------------------------------------------------||--//
//--------------------------------------------------------------------------||--//
/*
void GetVtkDataObjectMaxMin(vtkDataObject *dataObject)
{
  vtkMPIController *contr = vtkMPIController::New();
  contr->Initialize(NULL, NULL, 1); // initializedExternally == 1;
  int nranks = contr->GetNumberOfProcesses();
  int rank   = contr->GetLocalProcessId();

  assert( dataObject->IsA("vtkUnstructuredGrid") ); //dataObject->PrintSelf(std::cout,vtkIndent(2)); //exit(0);
  //PrintDataObjectName( dataObject );  

  Analysis analysis = Analysis(); // vtktools.hpp   

  if( dataObject->IsA("vtkUnstructuredGrid") )
    analysis.SetPoints( vtkUnstructuredGrid::SafeDownCast(dataObject)->GetPoints() );
  //analysis.PrintArrays(false);
  //analysis.SaveArray("x.y", "Coords");

  int idMin = -1; 
  int idMax = -1;
  std::vector<double>::iterator result;
  std::vector< std::vector<double> > range; 
  for(int i=0; i<3; i++)
  {
    std::vector<double> array( analysis.GetComponent("Coords",i) );
    result = std::min_element(array.begin(), array.end());
    idMin  = std::distance(array.begin(), result);

    result = std::max_element(array.begin(), array.end());
    idMax  = std::distance(array.begin(), result);

    range.push_back( {array[idMin], array[idMax]} );
  }

  for(int i=0; i<range.size(); i++)
    std::cout
           <<"\t[GetVtkDataObjectMaxMin] "
           << rank <<"."<< nranks <<":"
	   <<" R"<< i <<" = "
           <<"["<< range[i][0]   
           <<","<< range[i][1] <<"] "
           <<" \n";

}
*/

/*
void PWriter1(vtkAlgorithmOutput *GetOutputPort, std::string name, std::string type)
{
  std::string fname;
  vtkXMLPDataWriter *parallel_writer = NULL;

  if( type == "vtkUnstructuredGrid")
  {
    parallel_writer = vtkXMLPUnstructuredGridWriter::New();
    fname = name + ".pvtu";
  }
  if( type == "vtkPolyData")
  {
    parallel_writer = vtkXMLPPolyDataWriter::New();
    fname = name + ".pvtp";
  }

  vtkMPIController *contr = vtkMPIController::New(); 
  //contr->Initialize(); //&argc, &argv, 1);
  contr->Initialize(NULL, NULL, 1); // initializedExternally == 1;
  int nranks = contr->GetNumberOfProcesses();
  int rank   = contr->GetLocalProcessId();

  if(!rank)  
  std::cout
  <<"\t'"<< fname <<"' " 
  <<"rank:"<< rank 
  <<"/"<< nranks
  <<"\n";

  // Create the parallel writer and call some functions
//auto parallel_writer = vtkSmartPointer<vtkXMLPUnstructuredGridWriter>::New();
  //if( GetOutputPort->IsA("vtkUnstructuredGrid") ) parallel_writer = vtkXMLPUnstructuredGridWriter::New(); 
  //if( GetOutputPort->IsA("vtkPolyData") )    parallel_writer = vtkXMLPPolyDataWriter::New();   
  assert(parallel_writer); 
  parallel_writer->SetInputConnection( GetOutputPort );
  parallel_writer->SetController(contr);
  parallel_writer->SetFileName( fname.c_str() );
  parallel_writer->SetNumberOfPieces(nranks);
  parallel_writer->SetStartPiece(rank);
  parallel_writer->SetEndPiece(rank);
  parallel_writer->SetDataModeToBinary();
  parallel_writer->Update();
  parallel_writer->Write();

  contr->Finalize(1);
}
*/

//--------------------------------------------------------------------------||--//
//--------------------------------------------------------------------------||--//
#endif // VTK_TOOLS_H
// J. MIGUEL ZAVALA AKE. 2019AUG18. STOCKHOLM, SWEDEN.
/*
  NOTES :
    vtkAlgorithmOutput <-- GetOutputPort|SetInputConnection, from vtkAlgorithm  
    vtkDataObject      <-- GetOutputDataObject|SetInputData, from vtkAlgorithm 

  FROM : (2025Jun19)
    https://github.com/jmake/INSITU_ANALYSIS/tree/master/PAAKAT/EXAMPLES/CXX/E03_OCYL
*/ 