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

#include <vtkDataObject.h>
#include <vtkCellType.h>
#include <vtkCell.h>
#include <vtkIdList.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPolyData.h>

#include <vtkSmartPointer.h>
//vtkSmartPointer<vtkContourFilter> contourFilter = vtkSmartPointer<vtkContourFilter>::New(); // SEGMENTATION!!


#include <vector>
#include <unordered_map>
#include <iostream>
#include <string>

#include <vtkTriangleFilter.h>

#include <vtkPointSet.h>
#include <vtkDataObject.h>
#include <vtkDataArray.h>
#include <vector>

#include <vtkKMeansStatistics.h>

#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkPoints.h>
#include <vtkTable.h>
#include <vtkDoubleArray.h>
#include <vtkKMeansStatistics.h>
#include <vtkVariantArray.h>
#include <vtkIntArray.h>
#include <vtkFieldData.h>
#include <vtkPointData.h>
#include <vtkNew.h>
#include <vtkMath.h>

//--------------------------------------------------------------------------||--//


//--------------------------------------------------------------------------||--//
//--------------------------------------------------------------------------||--//
vtkImageData* RunKMeansOnImageDataPoints(vtkImageData* image, int numClusters)
{
  std::cout << "[RunKMeansOnImageDataPoints] ...";

  if (!image || numClusters < 1) {
      std::cerr << "[RunKMeansOnImageDataPoints] Invalid input." << std::endl;
      return nullptr;
  }

  int dims[3];
  double spacing[3];
  double origin[3];
  image->GetDimensions(dims);
  image->GetSpacing(spacing);
  image->GetOrigin(origin);

  vtkNew<vtkTable> table;
  vtkNew<vtkDoubleArray> xArr;
  vtkNew<vtkDoubleArray> yArr;
  vtkNew<vtkDoubleArray> zArr;

  xArr->SetName("X");
  yArr->SetName("Y");
  zArr->SetName("Z");

  table->AddColumn(xArr);
  table->AddColumn(yArr);
  table->AddColumn(zArr);

  for (int z = 0; z < dims[2]; ++z)
      for (int y = 0; y < dims[1]; ++y)
          for (int x = 0; x < dims[0]; ++x)
          {
              double px = origin[0] + x * spacing[0];
              double py = origin[1] + y * spacing[1];
              double pz = origin[2] + z * spacing[2];

              vtkNew<vtkVariantArray> row;
              row->InsertNextValue(px);
              row->InsertNextValue(py);
              row->InsertNextValue(pz);
              table->InsertNextRow(row);
          }

  vtkNew<vtkKMeansStatistics> kmeans;
  kmeans->SetInputData(vtkStatisticsAlgorithm::INPUT_DATA, table);
  kmeans->SetColumnStatus("X", 1);
  kmeans->SetColumnStatus("Y", 1);
  kmeans->SetColumnStatus("Z", 1);
  kmeans->RequestSelectedColumns();
  kmeans->SetAssessOption(true);
  kmeans->SetDefaultNumberOfClusters(numClusters);
  kmeans->Update();

  vtkTable* assessedTable = kmeans->GetOutput(1);
  vtkAbstractArray* labels = assessedTable->GetColumnByName("kmeans cluster");

  if (!labels) {
      std::cerr << "[RunKMeansOnImageDataPoints] Failed to get cluster labels." << std::endl;
      return nullptr;
  }

  vtkNew<vtkIntArray> clusterLabels;
  clusterLabels->SetName("KMeansLabels");
  clusterLabels->SetNumberOfComponents(1);
  clusterLabels->SetNumberOfTuples(labels->GetNumberOfTuples());

  for (vtkIdType i = 0; i < labels->GetNumberOfTuples(); ++i) {
      clusterLabels->SetValue(i, labels->GetVariantValue(i).ToInt());
  }

  image->GetPointData()->AddArray(clusterLabels);

  std::cout << "[RunKMeansOnImageDataPoints] Done!!";
  return image;
}


//--------------------------------------------------------------------------||--//
void RunKMeansOnScalarArray(vtkDataObject* obj, const std::string& arrayName, int numClusters)
{
    std::cout << "[RunKMeansOnScalarArray] Clustering array: " << arrayName << "...\n";

    vtkPolyData* poly = vtkPolyData::SafeDownCast(obj);
    if (!poly || !poly->GetPointData())
    {
        std::cerr << "[RunKMeansOnScalarArray] Invalid vtkPolyData or missing PointData.\n";
        return;
    }

    vtkDataArray* dataArray = poly->GetPointData()->GetArray(arrayName.c_str());
    if (!dataArray)
    {
        std::cerr << "[RunKMeansOnScalarArray] Array '" << arrayName << "' not found.\n";
        return;
    }

    vtkIdType nPoints = dataArray->GetNumberOfTuples();
    if (nPoints == 0)
    {
        std::cerr << "[RunKMeansOnScalarArray] Array has no data.\n";
        return;
    }

    // Build VTK table with 1 column from the selected array
    vtkNew<vtkTable> table;
    vtkNew<vtkDoubleArray> scalarColumn;
    scalarColumn->SetName(arrayName.c_str());
    scalarColumn->SetNumberOfTuples(nPoints);

    for (vtkIdType i = 0; i < nPoints; ++i)
        scalarColumn->SetValue(i, dataArray->GetComponent(i, 0));

    table->AddColumn(scalarColumn);

    // Set up KMeans
    vtkNew<vtkKMeansStatistics> kmeans;
    kmeans->SetInputData(vtkStatisticsAlgorithm::INPUT_DATA, table);
    kmeans->SetColumnStatus(arrayName.c_str(), 1);
    kmeans->RequestSelectedColumns();
    kmeans->SetAssessOption(true);
    kmeans->SetDefaultNumberOfClusters(numClusters);
    kmeans->Update();

    vtkTable* result = kmeans->GetOutput();
    vtkAbstractArray* clusterColumn = result->GetColumnByName("kmeans");
    if (!clusterColumn)
    {
        std::cerr << "[RunKMeansOnScalarArray] Missing 'kmeans' column in output.\n";
        return;
    }

    // Create and assign cluster label array
    vtkNew<vtkIntArray> clusterLabels;
    clusterLabels->SetName("ClusterLabels");
    clusterLabels->SetNumberOfComponents(1);
    clusterLabels->SetNumberOfTuples(nPoints);

    for (vtkIdType i = 0; i < nPoints; ++i)
        clusterLabels->SetValue(i, static_cast<int>(clusterColumn->GetVariantValue(i).ToInt()));

    poly->GetPointData()->AddArray(clusterLabels);
    poly->GetPointData()->SetActiveScalars("ClusterLabels");

    std::cout << "[RunKMeansOnScalarArray] Done.\n";
}


void RunKMeansOnPolyDataWithLabels(vtkDataObject* obj, int numClusters)
{
  std::cout << "[RunKMeansOnPolyDataWithLabels] ...";

    vtkPolyData* poly = vtkPolyData::SafeDownCast(obj);
    if (!poly || !poly->GetPoints())
    {
        std::cerr << "[RunKMeans] Invalid vtkPolyData or missing points.\n";
        return;
    }

    vtkPoints* pts = poly->GetPoints();
    vtkIdType nPoints = pts->GetNumberOfPoints();

    vtkNew<vtkTable> table;
    vtkNew<vtkDoubleArray> arrX;
    vtkNew<vtkDoubleArray> arrY;
    vtkNew<vtkDoubleArray> arrZ;

    arrX->SetName("X");
    arrY->SetName("Y");
    arrZ->SetName("Z");

    arrX->SetNumberOfTuples(nPoints);
    arrY->SetNumberOfTuples(nPoints);
    arrZ->SetNumberOfTuples(nPoints);

    for (vtkIdType i = 0; i < nPoints; ++i)
    {
        double p[3];
        pts->GetPoint(i, p);
        arrX->SetValue(i, p[0]);
        arrY->SetValue(i, p[1]);
        arrZ->SetValue(i, p[2]);
    }

    table->AddColumn(arrX);
    table->AddColumn(arrY);
    table->AddColumn(arrZ);

    vtkNew<vtkKMeansStatistics> kmeans;
    kmeans->SetInputData(vtkStatisticsAlgorithm::INPUT_DATA, table);
    kmeans->SetColumnStatus("X", 1);
    kmeans->SetColumnStatus("Y", 1);
    kmeans->SetColumnStatus("Z", 1);
    kmeans->RequestSelectedColumns();
    kmeans->SetAssessOption(true);
    kmeans->SetDefaultNumberOfClusters(numClusters);
    kmeans->Update();

    vtkTable* result = kmeans->GetOutput();

    // Extract "kmeans" column and map to vtkIntArray
    vtkAbstractArray* clusterColumn = result->GetColumnByName("kmeans");
    if (!clusterColumn)
    {
        std::cerr << "\n[RunKMeans] Missing 'kmeans' column in result.\n";
        return;
    }

    vtkNew<vtkIntArray> clusterLabels;
    clusterLabels->SetName("ClusterLabels");
    clusterLabels->SetNumberOfComponents(1);
    clusterLabels->SetNumberOfTuples(nPoints);

    for (vtkIdType i = 0; i < nPoints; ++i)
    {
        clusterLabels->SetValue(i, static_cast<int>(clusterColumn->GetVariantValue(i).ToInt()));
    }

    // Assign to point data
    poly->GetPointData()->AddArray(clusterLabels);
    poly->GetPointData()->SetActiveScalars("ClusterLabels");

  std::cout << "[RunKMeansOnPolyDataWithLabels] Done!!";
}


//--------------------------------------------------------------------------||--//
vtkDataObject* TriangulateGet(vtkDataObject* obj)
{
    vtkTriangleFilter* triangleFilter = vtkTriangleFilter::New();
    triangleFilter->SetInputData(obj);
    triangleFilter->Update();

    return triangleFilter->GetOutputDataObject(0);
}



//--------------------------------------------------------------------------||--//
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
//--------------------------------------------------------------------------||--//
std::vector<int> GetFlatCellIndices(vtkDataObject* obj, int& nCells)
{
    vtkPolyData* poly = vtkPolyData::SafeDownCast(obj);
    std::vector<int> flatIndices;

    if (!poly)
    {
        nCells = 0; 
        std::cerr << "[GetFlatCellIndices] Error: input is not vtkPolyData.\n";
        return flatIndices;
    }

    nCells = poly->GetNumberOfCells();
    for (vtkIdType i = 0; i < nCells; ++i)
    {
        vtkSmartPointer<vtkIdList> ids = vtkSmartPointer<vtkIdList>::New();
        poly->GetCellPoints(i, ids);

        for (vtkIdType j = 0; j < ids->GetNumberOfIds(); ++j)
        {
            flatIndices.push_back(static_cast<int>(ids->GetId(j)));
        }
    }

    return flatIndices;
}



//--------------------------------------------------------------------------||--//
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


vtkDataObject* CutterCreate(vtkDataObject* Obj, vtkImplicitFunction* function)
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
  vtkDataObject *cutter = CutterCreate(obj, plane); 
  assert(cutter);
  return cutter;
}


//--------------------------------------------------------------------------||--//
//--------------------------------------------------------------------------||--//
vtkDataObject* GetContour(vtkDataObject *input, std::string key, double threshold, bool scalars)
{
  vtkContourFilter* contourFilter = vtkContourFilter::New();
  contourFilter->SetInputData( input );
  contourFilter->SetValue(0,threshold);  // IsoSurface(0) = threshold
  contourFilter->SetInputArrayToProcess(0, 0, 0, 0, key.c_str() );

  contourFilter->GenerateTrianglesOn();
  if(!scalars) contourFilter->ComputeScalarsOff();
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
//----------------------------------------|  FROM : E04_BOOST/nek5k01_01.cxx |--//
template <typename T>
std::vector<T>
GetCppArray(vtkDataArray *vtk_array, int* rows=NULL, int* cols=NULL, std::string order="C")
{
  //assert(vtk_array);
  std::vector<T> cpp_array;  

  if(vtk_array) 
  { 
    int nDims = vtk_array->GetNumberOfComponents(); //assert(nDims==1); 
    int nRows = vtk_array->GetNumberOfTuples();

    if(cols) cols[0] = nDims;
    if(rows) rows[0] = nRows;

  //std::vector<double> cpp_array(nRows * nDims,0.0);
    cpp_array = std::vector<T>(nRows * nDims, 0.0);   

    /*  C++ Order 
    |--- nDims ---| _ _
    a1  b1 ... y1 z1  |  
    a2  b2 ... y2 z2  |  
    ...              nRows  ==>> [a1 b1 ... y1 z1 ... an bn ... yn zn]    
    an  bn ... yn zn _|_         |---------  nDims * nRows ----------|
    */
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
    if(order == "F")
    for(int j=0,k=0; j<nDims; j++)
      for(int i=0; i<nRows; i++) cpp_array[k++] = vtk_array->GetComponent(i,j);
  }

  return cpp_array;
}


//--------------------------------------------------------------------------||--//
/*
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
    array = GetCppArray<double>(dataArray, &nPts, &nCols, "C");
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

  return array;
}
*/

//--------------------------------------------------------------------------||--//
//--------------------------------------------------------------------------||--//
/*
class Analysis
{
  public :
  Analysis(){}
 ~Analysis(){}

  void SetPoints(vtkPoints *points)
  {
    vtkDataArray *dataArray = points->GetData();
    Arrays["Coords"] = GetCppArray<double>(dataArray, &rows, &cols);
    ArrayNames.push_back("Coords");
  }

  void SetArray(vtkPointData  *pointData, std::string key)
  {
    GetArrayNames(pointData);
    FindArrayName(key);

    vtkDataArray *dataArray = pointData->GetArray(key.c_str());
    Arrays[key] = GetCppArray<double>(dataArray);
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
    assert( icol + 1 <= ncols ); 

    double *begin = Array.data() + rows * (icol+0);
    double *end   = Array.data() + rows * (icol+1);

    std::vector<double> array(rows, std::numeric_limits<double>::max() ); 
    for(int i=0; i<rows; i++) array[i] = begin[i];//  Array.data() + rows * icol + i;
    return array;  
  }


  protected :
  std::vector<std::string> ArrayNames;
  std::map<std::string, std::vector<double> > Arrays;

  std::map<std::string, std::vector<double> >::iterator It;

  int rows, cols;

};
*/
/*
  Analysis analysis = Analysis();
  analysis.SetPoints( polyData->GetPoints() );
  analysis.SetArray(  polyData->GetPointData(), "Pressure");
  //analysis.SetArray(  polyData->GetPointData(), "Velocity");
*/

//--------------------------------------------------------------------------||--//
//--------------------------------------------------------------------------||--//
void VersionShow()
{
    std::cout << "[SpicyTech] VTK version: " << vtkVersion::GetVTKVersion() << std::endl;
}


void PrintSelf(vtkDataObject *vtkDataObject)
{
  vtkDataObject->PrintSelf(std::cout,vtkIndent(2));
}


void VtkWarning( std::string fname )
{
  vtkFileOutputWindow *outwin = vtkFileOutputWindow::New();
  outwin->SetFileName( fname.c_str() );    
  outwin->SetInstance(outwin);
}


//--------------------------------------------------------------------------||--//
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
/*
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
*/
class ExtractorSlicer  
{
    public :
    ~ExtractorSlicer()
    {
      if(vtp) 
      {
        vtp->Delete();
        vtp = nullptr; 
      } 

      if(array) 
      {
        array = nullptr; 
      } 

      if(vti) 
      {
        vti->Delete(); // ??
        vti = nullptr; 
      } 

      indices.clear(); 
      property.clear(); 
      vertices.clear(); 
    }


    ExtractorSlicer()
    {
      vti = nullptr; 
      vtp = nullptr; 
      array = nullptr; 

      indices.clear(); 
      property.clear(); 
      vertices.clear(); 
    }


    void Create(vtkDataObject *obj, std::string prop)
    {
      vti = static_cast<vtkImageData*>(obj);        
      key = prop; 

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
      vtkDataObject *cutter = CutterPlane(vti, orig, normal); 
      //RunKMeansOnPolyDataWithLabels(cutter, 2); 

      vtp = static_cast<vtkPolyData*>(cutter);

      vtkPointData* pointData = vtp->GetPointData(); 

      std::vector<std::string> names;  
      names = GetArrayNames(pointData); 

      if( Contains(names, key) )
      {
        //RunKMeansOnScalarArray(vtp, key, 2); 
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

    vtkPolyData* vtp; 
    vtkImageData* vti; 
    vtkDataArray* array; 

    std::vector<int> indices; 
    std::vector<float> property; 
    std::vector<float> vertices; 
}; 


//--------------------------------------------------------------------------||--//
std::vector<double> RangeGet(vtkImageData *obj, std::string key) 
{
  std::vector<double> range(2, std::numeric_limits<double>::max()); 

  vtkPointData* pd = obj->GetPointData(); 
  std::vector<std::string> names = GetArrayNames(pd); 

  if( Contains(names, key) )
  {
    vtkDataArray* ar = GetPointDataArray(pd, key);  
    ar->GetRange(range.data()); 
    std::cout <<"[RangeGet] range["<< key << "] : (" << range[0] <<", "<< range[1] <<") \n";
  }

  return range; 
}


class ExtractorContour
{
    public :
    ~ExtractorContour()
    {
      if(vtp) 
      {
        vtp->Delete();
        vtp = nullptr; 
      } 

      if(array) 
      {
        array = nullptr; 
      } 

      if(vti) 
      {
        vti = nullptr; 
      } 

      indices.clear(); 
      property.clear(); 
      vertices.clear(); 
    }


    ExtractorContour()
    {
      vti = nullptr; 
      vtp = nullptr; 
      array = nullptr; 

      indices.clear(); 
      property.clear(); 
      vertices.clear(); 

      range.clear(); 
    }


    void Create(vtkDataObject *obj, std::string prop)
    {
      key = prop; 
      vti = static_cast<vtkImageData*>(obj); 

      range = RangeGet(vti, prop); 

      double threshold = (range[0] + range[1]) * 0.5;
      __Update__(threshold);
    }


    void Update(double threshold)
    {
      __Update__(threshold);
    } 


    void __Update__(double threshold)
    {
      bool getscalars = true; 
      vtkDataObject* contour = GetContour(vti, key, threshold, getscalars); 
      //vtkDataObject* trias = TriangulateGet(contour); 

      vtp = static_cast<vtkPolyData*>( contour );
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
      if( array )
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
      if( vtp )
      {
        PWriterSerial(vtp, fname); 
      }        
    }

    void IndicesGet(int* array) 
    {
      std::copy(indices.begin(), indices.end(), array);
    }

    
    std::vector<int> indices; 
    std::vector<float> property; 
    std::vector<float> vertices; 

    private : 
    std::string key;  
    std::vector<double> range;

    vtkPolyData* vtp; 
    vtkImageData* vti; 
    vtkDataArray* array; 
}; 


//--------------------------------------------------------------------------||--//
//--------------------------------------------------------------------------||--//



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