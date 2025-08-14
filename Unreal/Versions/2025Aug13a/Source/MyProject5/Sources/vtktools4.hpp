#pragma once 

#include <vector>
#include <unordered_map>
#include <iostream>
#include <string>

#include <iostream>
#include <string>

#include <map>
#include <chrono>
#include <chrono>
#include <string>
#include <iostream> // cin, cout, endl, cerr
#include <vector>   // vector

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
#include <vtkXMLStructuredGridWriter.h>

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



#include <vtkTriangleFilter.h>

#include <vtkPointSet.h>
#include <vtkDataObject.h>
#include <vtkDataArray.h>

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

#include <vtkSmartPointer.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>
#include <vtkDataObject.h>
#include <vtkDataSet.h>

#include <vtkPoints.h>


#include <vtkDataObject.h>
#include <vtkXMLReader.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkXMLImageDataReader.h>
#include <vtkXMLUnstructuredGridReader.h>

#include <vtkFlyingEdgesPlaneCutter.h>

#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkImageReslice.h>
#include <vtkMatrix4x4.h>
#include <vtkMath.h>
#include <vtkDataObject.h>

#include <vtkPointDataToCellData.h>

#include <vtkVersion.h>
#include <vtkSmartPointer.h>
#include <vtkDICOMImageReader.h>

#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkDataArray.h>
#include <vtkPointData.h>

#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>



template <typename T>
T* GetCppArray3(vtkDataArray *vtk_array, int* rows, int* cols);

template <typename T>
T* ArrayGet3(vtkPointData* pd, std::string key, int& length);

vtkDataObject* PReaderSerial(const std::string& filename);

std::vector<double> GetGeometricCenter(vtkDataObject* obj);
std::vector<double> GetBounds(vtkDataObject* obj);

void CenterImageAtOrigin(vtkImageData* image);

vtkDataObject* CenterAtOrigin(vtkDataObject* obj); 
vtkImageData* ReadVTIFile(const std::string& path);

template float* GetCppArray3<float>(vtkDataArray*, int*, int*);
template float* ArrayGet3<float>(vtkPointData* pd, std::string key, int& length); 
//--------------------------------------------------------------------------||--//
//--------------------------------------------------------------------------||--//


std::string SuggestTypeName(); 
long long* GetTriangles(vtkPolyData* vtp, int& n_indices, long long& n_raw); 
float* GetVertices(vtkPolyData* vtp, int& n_rows, int& n_cols); 

vtkDataObject* CutterPlane3(vtkImageData* volumeData, double center[3], double anatomicalView[9]); 

float NormalizeToRange(float x, float new_min, float new_max, float arr_min = 0.0f, float arr_max = 1.0f); 

float* VtiTest(int& width, int& height); 

void VtpTest();

vtkDataObject* GetContour(vtkDataObject *input, std::string key, double threshold, bool scalars); 

//--------------------------------------------------------------------------||--//
//--------------------------------------------------------------------------||--//