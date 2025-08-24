#include "Sources/vtktools4.hpp"


//--------------------------------------------------------------------------||--//
//--------------------------------------------------------------------------||--//
template <typename T>
T* GetCppArray3(vtkDataArray *vtk_array, int* rows, int* cols)
{
auto start = std::chrono::high_resolution_clock::now();

  cols[0] = 0;
  rows[0] = 0;
  T* raw_ptr = nullptr; 

  if(vtk_array) 
  { 
    int nDims = vtk_array->GetNumberOfComponents(); //assert(nDims==1); 
    int nRows = vtk_array->GetNumberOfTuples();

    cols[0] = nDims;
    rows[0] = nRows;

    raw_ptr = static_cast<T*>(vtk_array->GetVoidPointer(0));
    //std::copy(raw_ptr, raw_ptr + nDims * nRows, cpp_array.begin());
  }

//auto start = chrono::high_resolution_clock::now();
auto end = std::chrono::high_resolution_clock::now();
auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
std::cout << "\t - [GetCppArray3] Execution time: " << duration.count() / 1000.0 << " seg" << std::endl;
//std::cout << "=======================================================================================\n"; 

  return raw_ptr;
}


template <typename T>
T* ArrayGet3(vtkPointData* pd, std::string key, int& length)
{
  T* property = nullptr;  

  if( pd->HasArray(key.c_str()) ) 
  {
    vtkDataArray* array = pd->GetArray(key.c_str());
    
    int n_rows = -1; 
    int n_cols = -1; 
    property = GetCppArray3<T>(array, &n_rows, &n_cols); 
    length = n_rows * n_cols; 

    std::cout << "\t    [ArrayGet3] key:'"<< array->GetName() <<"' n_rows : "<< n_rows <<" n_cols: "<< n_cols <<"\n";  
  } 

  return property; 
}


//--------------------------------------------------------------------------||--//
vtkDataObject* PReaderSerial(const std::string& filename) 
{
    std::cout << "[PReaderSerial] Reading file: '" << filename << "' \n";
    std::string ext = filename.substr(filename.find_last_of('.') + 1);
    vtkDataObject* output = nullptr;

    if (ext == "vtp") {
        vtkXMLPolyDataReader* reader = vtkXMLPolyDataReader::New();
        reader->SetFileName(filename.c_str());
        reader->Update();
        output = reader->GetOutput();
        if (output) output->Register(nullptr); // ?? 
        reader->Delete();
    }
    else if (ext == "vti") {
        vtkXMLImageDataReader* reader = vtkXMLImageDataReader::New();
        reader->SetFileName(filename.c_str());
        reader->Update();
        output = reader->GetOutput();
        if (output) output->Register(nullptr);
        reader->Delete();
    }
    else if (ext == "vtu") {
        vtkXMLUnstructuredGridReader* reader = vtkXMLUnstructuredGridReader::New();
        reader->SetFileName(filename.c_str());
        reader->Update();
        output = reader->GetOutput();
        if (output) output->Register(nullptr);
        reader->Delete();
    }
    else {
        std::cerr << "[PReaderSerial] Unsupported file extension: " << ext << "\n";
        return nullptr;
    }

    if (output)
        std::cout << "[PReaderSerial] Loaded object type: '" << output->GetClassName() << "' \n";
    else
        std::cerr << "[PReaderSerial] Failed to read data.\n";

    return output;
}

//--------------------------------------------------------------------------||--//
std::vector<double> GetGeometricCenter(vtkDataObject* obj)
{
    std::vector<double> center(3);

    auto data = vtkDataSet::SafeDownCast(obj);
    assert(data);
    //if(!data) return center; 

    double bounds[6];
    data->GetBounds(bounds); // xmin, xmax, ymin, ymax, zmin, zmax

    center[0] = 0.5 * (bounds[0] + bounds[1]);
    center[1] = 0.5 * (bounds[2] + bounds[3]);
    center[2] = 0.5 * (bounds[4] + bounds[5]);
    return center;
}


std::vector<double> GetBounds(vtkDataObject* obj)
{
  std::vector<double> bounds(6);

  auto data = vtkDataSet::SafeDownCast(obj);
  assert(data);

  data->GetBounds(bounds.data()); // xmin, xmax, ymin, ymax, zmin, zmax
  return bounds; 
}


void CenterImageAtOrigin(vtkImageData* image)
{
    if (!image) return;

    int dims[3];
    double spacing[3];

    image->GetDimensions(dims);
    image->GetSpacing(spacing);

    double origin[3];
    origin[0] = -0.5 * spacing[0] * (dims[0] - 1);
    origin[1] = -0.5 * spacing[1] * (dims[1] - 1);
    origin[2] = -0.5 * spacing[2] * (dims[2] - 1);

    image->SetOrigin(origin);
}


vtkDataObject* CenterAtOrigin(vtkDataObject* obj)
{
    if (!obj) 
    {
      std::cout << "[CenterAtOrigin] '!obj' \n";
      return nullptr;
    }

    //std::cout << "[CenterAtOrigin] '"<< obj->GetClassName() << "' ... ";

    // Handle vtkImageData: set origin directly
    if (auto image = vtkImageData::SafeDownCast(obj)) 
    {
        int dims[3];
        double spacing[3];
        image->GetDimensions(dims);
        image->GetSpacing(spacing);

        double origin[3] = {
            -0.5 * spacing[0] * (dims[0] - 1),
            -0.5 * spacing[1] * (dims[1] - 1),
            -0.5 * spacing[2] * (dims[2] - 1)
        };

        image->SetOrigin(origin);

        //std::cout << " ok! \n";
        return image;
    }
    // Handle vtkDataSet: use transform
    else if (auto data = vtkDataSet::SafeDownCast(obj)) 
    {
        double bounds[6];
        data->GetBounds(bounds);

        double center[3] = {
            0.5 * (bounds[0] + bounds[1]),
            0.5 * (bounds[2] + bounds[3]),
            0.5 * (bounds[4] + bounds[5])
        };

        vtkTransform* transform = vtkTransform::New();
        transform->Translate(-center[0], -center[1], -center[2]);

        vtkTransformFilter* tf = vtkTransformFilter::New();
        tf->SetInputData(obj);
        tf->SetTransform(transform);
        tf->Update();

        //std::cout << "ok! \n";
        return tf->GetOutputDataObject(0);
    }
    else
    {
      std::cerr << "[CenterAtOrigin] Something wrong ... \n";      
      return nullptr;
    }

    //return obj;
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
//--------------------------------------------------------------------------| Contour3 |--//
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


float NormalizeToRange(float x, float new_min, float new_max, float arr_min, float arr_max)
{
    float scaled = (x - arr_min) / (arr_max - arr_min);
    float mapped = scaled * (new_max - new_min) + new_min;
    return mapped;
}


//----------------------------------------------------------------------------------|  |--//
float* GetVertices(vtkPolyData* vtp, int& n_rows, int& n_cols)
{
	if(vtp == nullptr) return nullptr; 

	vtkPoints* pts = vtp->GetPoints(); 
	return GetCppArray3<float>( pts->GetData(), &n_rows, &n_cols ); 
}


long long* GetTriangles(vtkPolyData* vtp, int& n_indices, long long& n_raw)
{
	if (vtp == nullptr) return nullptr;

	vtkCellArray* cells = vtp->GetPolys();
	if (cells == nullptr) return nullptr;

	n_indices = vtp->GetNumberOfCells();

	vtkIdTypeArray* data = cells->GetData();
	if (data == nullptr) return nullptr;

	if (data->GetNumberOfTuples() != vtp->GetNumberOfCells() * 4) {
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


//--------------------------------------------------------------------------||--//
vtkDataObject* CutterPlane3(vtkImageData* volumeData, double center[3], double anatomicalView[9])
{
    // Create vtkImageReslice
    vtkImageReslice* resliceFilter = vtkImageReslice::New();
    resliceFilter->SetInputData(volumeData);

    // Set output to 2D slice
    resliceFilter->SetOutputDimensionality(2);

    // Set direction cosines for the axes (anatomical view)
    resliceFilter->SetResliceAxesDirectionCosines(
        anatomicalView[0], anatomicalView[1], anatomicalView[2],
        anatomicalView[3], anatomicalView[4], anatomicalView[5],
        anatomicalView[6], anatomicalView[7], anatomicalView[8]);

    // Set origin of the slice (center)
    resliceFilter->SetResliceAxesOrigin(center);

    resliceFilter->Update();

    vtkDataObject* output = resliceFilter->GetOutputDataObject(0);

    return output;
}


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
}


//----------------------------------------------------------------------------------|  |--//
//------------------------------------------------------------------------------|  vti |--//
void VtpTest()
{
	// LogTemp: Warning: [SpicyTech] n_rows = 382824 n_cols = 3
	// LogTemp: Warning: [SpicyTech] NumberOfCells = 764550
	// LogTemp: Warning: [SpicyTech] n_triangles = 764550 n_raw = 3058200 -> n_raw == n_triangles * 4  

	std::string fname = "F:/z2025_1/Dicom/DecafPV560/contour.vtp"; 
	vtkDataObject* obj = PReaderSerial(fname); 
	vtkPolyData* vtp = static_cast<vtkPolyData*>(obj); 

	int NumberOfCells = vtp->GetNumberOfCells(); 
//	UE_LOG(LogTemp, Warning, TEXT("[SpicyTech] NumberOfCells = %d"), NumberOfCells);

	int n_rows, n_cols; 
	GetVertices(vtp, n_rows, n_cols); 
//	UE_LOG(LogTemp, Warning, TEXT("[SpicyTech] n_rows = %d n_cols = %d"), n_rows, n_cols);

    int n_triangles; 
    long long n_raw; 
    long long* triangles = GetTriangles(vtp, n_triangles, n_raw);
//	UE_LOG(LogTemp, Warning, TEXT("[SpicyTech] n_triangles = %d n_raw = %d"), n_triangles, n_raw);
}


//----------------------------------------------------------------------------------|  |--//
//TArray<float> 
float* VtiTest(int& width, int& height) 
{
	std::string fname = "F:/z2025_1/Dicom/DecafPV560/domain.vti"; 

	vtkImageData* vti = ReadVTIFile(fname); 
	CenterImageAtOrigin(vti); 

	double center[3]; 
	vti->GetCenter( center ); 
//UE_LOG(LogTemp, Warning, TEXT("[SpicyTech] center : (%f, %f, %f)"), center[0], center[1], center[2]);

	double bounds[6]; // [xmin, xmax, ymin, ymax, zmin, zmax]
    vti->GetBounds( bounds );
//UE_LOG(LogTemp, Warning, TEXT("[SpicyTech] xmin,xmax : (%f, %f)"), bounds[0], bounds[1]);
//UE_LOG(LogTemp, Warning, TEXT("[SpicyTech] ymin,ymax : (%f, %f)"), bounds[2], bounds[3]);
//UE_LOG(LogTemp, Warning, TEXT("[SpicyTech] zmin,zmax : (%f, %f)"), bounds[4], bounds[5]);

	float vmin =  0.0;
	float vmax =  1.0;  	

	float x0 = 0.5; 
	float x = NormalizeToRange(x0, bounds[0], bounds[1], vmin, vmax); 
//UE_LOG(LogTemp, Warning, TEXT("[CreateContour] x0 : %f -> %f"), x0, x);

	float y0 = 0.5f;
	float y = NormalizeToRange(y0, bounds[2], bounds[3], vmin, vmax);
//UE_LOG(LogTemp, Warning, TEXT("[CreateContour] y0 : %f -> %f"), y0, y);

	float z0 = 0.5f;
	float z = NormalizeToRange(z0, bounds[4], bounds[5], vmin, vmax);
//UE_LOG(LogTemp, Warning, TEXT("[CreateContour] z0 : %f -> %f"), z0, z);


  double anatomicalView[9] = {1, 0,  0, 0, 1,  0, 0, 0, 1}; // Axial. dimensions : (512, 512, 1) ??  
	//double anatomicalView[9] = {1, 0,  0, 0, 0, -1, 0, 1, 0}; // Coronal. dimensions: (512, 429, 1) 
  //double anatomicalView[9] = {0, 0, -1, 1, 0,  0, 0, 1, 0}; // Sagittal. dimensions: (429, 512, 1) 
	double orig[3] = {center[0] + x, center[1] + y, center[2] + z}; 
	vtkDataObject* obj = CutterPlane3(vti, orig, anatomicalView);
	vtkImageData* cutter = static_cast<vtkImageData*>(obj);  

	int dimensions[3];		
	cutter->GetDimensions(dimensions); 
//UE_LOG(LogTemp, Warning, TEXT("[CreateContour] dimensions: x=%d, y=%d, z=%d"), dimensions[0], dimensions[1], dimensions[2]);

	int length = 0; 
	std::string key = "ImageScalars";  
	vtkPointData* pd = cutter->GetPointData(); 

	float *buffer = ArrayGet3<float>(pd, key, length); 
	//std::vector<float> buffer = ArrayGet2<float>(pd, key); length = buffer.size();
	//std::vector<float> buffer; ArrayGet(pd, key, buffer); length = buffer.size(); 
//UE_LOG(LogTemp, Warning, TEXT("[CreateContour] length : %d "), length);

	//TArray<double> View; View.SetNumZeroed(length);	for(int i=0; i< length; i++) View[i] = buffer[i];  

	width = dimensions[0]; 
	height = dimensions[1]; 

	//TArrayView<double> View(buffer, length); // crash ... 
	//TArray<float> View; View.Append(buffer, length); // crash ...
	//return View; 
	return buffer; 
}



//----------------------------------------------------------------------------------|  |--//
vtkImageData* ReduceImage(vtkImageData*& inputImage, int factorX, int factorY, int factorZ)
{
    if (!inputImage)
        return nullptr;

    vtkImageShrink3D* shrinkFilter = vtkImageShrink3D::New();
    shrinkFilter->SetInputData(inputImage);

    // Set shrink factors for each axis
    shrinkFilter->SetShrinkFactors(factorX, factorY, factorZ);

    // Use averaging when reducing
    shrinkFilter->AveragingOn();

    shrinkFilter->Update();

    vtkImageData* reducedImage = vtkImageData::New();
    reducedImage->DeepCopy(shrinkFilter->GetOutput());

    shrinkFilter->Delete();

    return reducedImage; // caller is responsible for deleting this
}



//----------------------------------------------------------------------------------|  |--//
std::vector<float> IterateImageDataArray(
    vtkImageData* image, 
    const std::string& key, 
    double range[2], 
    int dims[3], 
    int& numComponents
)
{
    std::vector<float> data;
    if (!image) return data;

    vtkPointData* pd = image->GetPointData();
    vtkDataArray* array = pd->GetArray(key.c_str());
    if (!array) return data;

	array->GetRange( range );  

    image->GetDimensions(dims); // dims[0]=X, dims[1]=Y, dims[2]=Z
    numComponents = array->GetNumberOfComponents();
    
    data.resize(dims[0] * dims[1] * dims[2] * numComponents); 
    for (int z = 0; z < dims[2]; ++z)
    {
        for (int y = 0; y < dims[1]; ++y)
        {
            for (int x = 0; x < dims[0]; ++x)
            {
                vtkIdType idx = x + y * dims[0] + z * dims[0] * dims[1];

                // Access each component of the voxel
                for (int c = 0; c < numComponents; ++c)
                {
                    double value = array->GetComponent(idx, c);
                    data[idx * numComponents + c] = value;
                }
            }
        }
    }
    
    return data; 
}


//--------------------------------------------------------------------------||--//
// Copies vtkDataArray into a std::vector<float> in DataTiles order (X->Y->Z with flipped Y)
bool ExportArrayToDataTiles(vtkImageData* image, const std::string& key, std::vector<float>& outArray)
{
    if (!image) return false;

    vtkPointData* pd = image->GetPointData();
    vtkDataArray* array = pd->GetArray(key.c_str());
    if (!array) return false;

    int dims[3];
    image->GetDimensions(dims); // dims[0]=X, dims[1]=Y, dims[2]=Z
    int numComponents = array->GetNumberOfComponents();
    int totalVoxels = dims[0] * dims[1] * dims[2];

    outArray.resize(totalVoxels * numComponents);

    for (int z = 0; z < dims[2]; ++z)
    {
        for (int y = 0; y < dims[1]; ++y)
        {
            int flippedY = (dims[1] - 1) - y; // Y-flip for DataTiles

            for (int x = 0; x < dims[0]; ++x)
            {
                vtkIdType vtkIdx = x + y * dims[0] + z * dims[0] * dims[1];
                int dataTilesIdx = x + flippedY * dims[0] + z * dims[0] * dims[1];

                for (int c = 0; c < numComponents; ++c)
                {
                    outArray[dataTilesIdx * numComponents + c] = static_cast<float>(array->GetComponent(vtkIdx, c));
                }
            }
        }
    }

    return true;
}




//--------------------------------------------------------------------------||--//
vtkImageData* ExtractVOIWithDefaults(
    vtkImageData* input,
    int xmin, int xmax,
    int ymin, int ymax,
    int zmin, int zmax)
{
    int dims[3];
    input->GetDimensions(dims);

    if (xmin < 0) xmin = 0;
    if (xmax < 0) xmax = dims[0] - 1;

    if (ymin < 0) ymin = 0;
    if (ymax < 0) ymax = dims[1] - 1;

    if (zmin < 0) zmin = 0;
    if (zmax < 0) zmax = dims[2] - 1;

    vtkExtractVOI* extract = vtkExtractVOI::New();
    extract->SetInputData(input);
    extract->SetVOI(xmin, xmax, ymin, ymax, zmin, zmax);
    extract->Update();

    vtkImageData* output = vtkImageData::New();
    output->ShallowCopy(extract->GetOutput());

    extract->Delete();
    return output;
}



//--------------------------------------------------------------------------||--//
//--------------------------------------------------------------------------||--//