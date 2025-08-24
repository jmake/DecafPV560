#include "VtkInterface.h"


VtkInterface::VtkInterface()
{
}

VtkInterface::~VtkInterface()
{
}


//----------------------------------------------------------------------------------|  |--//
//----------------------------------------------------------------------------------|  |--//
template<typename RETURN_TYPE>
void SetAsyncFuncs(TFunction<RETURN_TYPE()> Task, TFunction<void(RETURN_TYPE)> After)
{
    auto Promise = MakeShared<TPromise<RETURN_TYPE>>();
    TFuture<RETURN_TYPE> Future = Promise->GetFuture();

    Async(EAsyncExecution::Thread, [Task, Promise]()
    {
        RETURN_TYPE Result = Task();
        Promise->SetValue(Result);
    });

    Future.Next([After](const RETURN_TYPE& Result)
    {
        AsyncTask(ENamedThreads::GameThread, [After, Result]()
        {
            After(Result);
        });
    });
}

//----------------------------------------------------------------------------------|  |--//
void Vti2AnatomicalBufferAsync(
                                vtkImageData* domain, 
                                double anatomicalView[9], 
                                float x0, 
                                float y0, 
                                float z0, //,vtkImageData*& cutter
                                std::function<void(AnatomicalProperties)> PostProcess 
                                //TFunction<void()> PostProcess 
) 
{
    // Init 
    auto Promise = MakeShared<TPromise<AnatomicalProperties>>();
    TFuture<AnatomicalProperties> Future = Promise->GetFuture();

    // Task 
    Async(EAsyncExecution::Thread, [Promise, domain, anatomicalView, x0, y0, z0]()
    {
	    AnatomicalProperties buffer = {};

        buffer.data = Vti2AnatomicalBuffer(
                                            domain, 
                                            anatomicalView, 
                                            x0, y0, z0, 
                                            buffer.vti_ptr, 
                                            buffer.width, 
                                            buffer.height, 
                                            buffer.spacingX, 
                                            buffer.spacingY 
                                        ); 
        
        Promise->SetValue(buffer);
    });    

    // After 
    Future.Next([PostProcess](const AnatomicalProperties& result)
    {
        AsyncTask(ENamedThreads::GameThread, [result,PostProcess]() 
        {
            vtkImageData* cutter = result.vti_ptr; 
            if(cutter == nullptr) 
            {
                UE_LOG(LogTemp, Error, TEXT("[PostProcess] 'cutter' fails !!") );
                return ; 
            } 

		    int numberOfCells = cutter->GetNumberOfCells(); 
            UE_LOG(LogTemp, Warning, TEXT("[Vti2AnatomicalBufferAsync] width : %d, height : %d numberOfCells: %d"), result.width, result.height, numberOfCells);

            PostProcess(result); 
			UE_LOG(LogTemp, Warning, TEXT("[Vti2AnatomicalBufferAsync] 'PostProcess' Executed !!") );
        });
    });
}


UTexture2D* AnatomicalBuffer2UTexture2D(AnatomicalProperties result, UWorld* world, APixelTextureDisplay*& display)
{
    //TArray<float> data;  data.Append(result.data, result.width * result.height);
    TArray<float> data(result.data, result.width * result.height);
    UE_LOG(LogTemp, Warning, TEXT("[CreateTextureAsync] Num : %d"), data.Num() );

    if (display && !display->IsActorBeingDestroyed())
    {
        display->Destroy();
        display = nullptr; 

        UE_LOG(LogTemp, Error, TEXT("[CreateTextureAsync] Removed! ") );
    }

    //APixelTextureDisplay* display = nullptr; 
    display = world->SpawnActor<APixelTextureDisplay>(APixelTextureDisplay::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
    display->Init(result.width, result.height, result.spacingX, result.spacingY);
    display->Apply(data, 1.0f);
    //display->Save(fname); 
    //display->SetActorLabel(UTF8_TO_TCHAR(name.c_str())); // Segmentation 
    UTexture2D* texture2D = display->DynamicTexture; 

    if(texture2D == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CreateTextureAsync] fails !!") );
        return nullptr;
    }

    UE_LOG(LogTemp, Warning, TEXT("[CreateTextureAsync] Size: %d X %d"), texture2D->GetSizeX(), texture2D->GetSizeY());
    return texture2D; 
}


//----------------------------------------------------------------------------------|  |--//
//----------------------------------------------------------------------------------|  |--//
void Vti2UEMesh(
                SurfaceProperties buffer, 
                TArray<FVector>& Vertices, 
                TArray<int32>& Triangles
            )
{
    // Vertices 
	Vertices.Reset();
	for(int i=0; i < buffer.n_rows; i++)
	{
		//for(int j=0; j<n_cols; j++, k++) float var = points[k]; 
		float x = buffer.points[i * buffer.n_cols + 0]; 
		float y = buffer.points[i * buffer.n_cols + 1]; 
		float z = buffer.points[i * buffer.n_cols + 2]; 	
		Vertices.Add( FVector(x,y,z) );
	}

	int32 nVertices = Vertices.Num();
	UE_LOG(LogTemp, Warning, TEXT("[Vti2UEMesh] nVertices = %d "), nVertices);

    // Triangles 
	Triangles.Reset();
	for (int i = 0; i < buffer.n_triangles; i++) 
	{
		int t0 = buffer.triangles[i * 4 + 0]; 
		int t1 = buffer.triangles[i * 4 + 1]; 
		int t2 = buffer.triangles[i * 4 + 2]; 
		int t3 = buffer.triangles[i * 4 + 3]; 
		Triangles.Add(t1);
		Triangles.Add(t2);
		Triangles.Add(t3);
	}

	int32 nTriangles = Triangles.Num();
	UE_LOG(LogTemp, Warning, TEXT("[Vti2UEMesh] nTriangles = %d (%d) "), nTriangles, buffer.n_triangles * 3);
}


//----------------------------------------------------------------------------------|  |--//
//----------------------------------------------------------------------------------|  |--//
void LoadVtiFile(
	const std::string& fname, 
	vtkImageData*& vti_ptr, 
	std::vector<double>& range
) 
{
	if(vti_ptr)
	{ 
		vti_ptr->Delete();
		vti_ptr = nullptr; 
	} 

	//vti_ptr = ReadVTIFile( TCHAR_TO_UTF8(*filename) ); 
	vti_ptr = ReadVTIFile(fname); 
	CenterImageAtOrigin(vti_ptr); 

	range = std::vector<double>(2, std::numeric_limits<double>::max()); 
	vtkPointData* pd = vti_ptr->GetPointData(); 
  	vtkDataArray* array = pd->GetArray("voxels");
    array->GetRange(range.data()); 
    //std::cout <<"[RangeGet] range["<< key << "] : (" << range[0] <<", "<< range[1] <<") \n";
	UE_LOG(LogTemp, Warning, TEXT("[LoadVtiFile] range : (%f,%f)"), range[0], range[1]);

	int dimensions[3];
	vti_ptr->GetDimensions(dimensions); 
	UE_LOG(LogTemp, Warning, TEXT("[LoadVtiFile] dimensions : (%d,%d,%d)"), dimensions[0], dimensions[1], dimensions[2] );
	//UE_LOG(LogTemp, Warning, TEXT("[LoadVtiFile] ok!"));
}


//----------------------------------------------------------------------------------|  |--//
float* Vti2AnatomicalBuffer(
								vtkImageData* domain, 
								double anatomicalView[9], 
								float x0, 
								float y0, 
								float z0, 
								vtkImageData*& cutter, 
								int& width, 
								int& height, 
								float& spacingX, 
								float& spacingY 
							)
{
	// Domain parameters  
	if(domain == nullptr) 
	{
		UE_LOG(LogTemp, Warning, TEXT("[Vti2AnatomicalView] 'domain' fails!") );
		return nullptr;
	}

	int dimensions2[3];
	domain->GetDimensions(dimensions2); 
	UE_LOG(LogTemp, Warning, TEXT("[Vti2AnatomicalView] VTI Dimensions : (%d,%d,%d)"), dimensions2[0], dimensions2[1], dimensions2[2] );

	double center[3]; 
	domain->GetCenter( center ); 
	UE_LOG(LogTemp, Warning, TEXT("[Vti2AnatomicalView] VTI Center : (%f,%f,%f)"), center[0], center[1], center[2] );

	double bounds[6]; // [xmin, xmax, ymin, ymax, zmin, zmax]
    domain->GetBounds( bounds );

	float vmin = 0.0;
	float vmax = 1.0; 
	float x = NormalizeToRange(x0, bounds[0], bounds[1], vmin, vmax); 
	UE_LOG(LogTemp, Warning, TEXT("[Vti2AnatomicalView] x0 : %f -> %f"), x0, x);

	float y = NormalizeToRange(y0, bounds[2], bounds[3], vmin, vmax);
	UE_LOG(LogTemp, Warning, TEXT("[Vti2AnatomicalView] y0 : %f -> %f"), y0, y);

	float z = NormalizeToRange(z0, bounds[4], bounds[5], vmin, vmax);
	UE_LOG(LogTemp, Warning, TEXT("[Vti2AnatomicalView] z0 : %f -> %f"), z0, z);

	UE_LOG(LogTemp, Warning, TEXT("[Vti2AnatomicalView] View1 : (%f,%f,%f)"), anatomicalView[0], anatomicalView[1], anatomicalView[2] );
	UE_LOG(LogTemp, Warning, TEXT("[Vti2AnatomicalView] View2 : (%f,%f,%f)"), anatomicalView[3], anatomicalView[4], anatomicalView[5] );
	UE_LOG(LogTemp, Warning, TEXT("[Vti2AnatomicalView] View3 : (%f,%f,%f)"), anatomicalView[6], anatomicalView[7], anatomicalView[8] );

	double orig[3] = {center[0] + x, center[1] + y, center[2] + z}; 
	vtkDataObject* obj = CutterPlane3(domain, orig, anatomicalView);

	// Creating Slicer 
	if(cutter)
	{
		cutter->Delete();
		cutter = nullptr; 
	}

	cutter = static_cast<vtkImageData*>(obj);  

	int dimensions[3];
	cutter->GetDimensions(dimensions); 
	width = dimensions[0]; 
	height = dimensions[1];
	UE_LOG(LogTemp, Warning, TEXT("[Vti2AnatomicalView] dimensions: x=%d, y=%d, z=%d"), dimensions[0], dimensions[1], dimensions[2]);

	double spacing[3];
	cutter->GetSpacing(spacing);  
	spacingX = spacing[0]; 
	spacingY = spacing[1]; 
	UE_LOG(LogTemp, Warning, TEXT("[Vti2AnatomicalView] spacing: x=%.2f, y=%.2f, z=%.2f"), spacing[0], spacing[1], spacing[2]);

	int length = -1; 
	std::string key = "ImageScalars";  
	vtkPointData* pd = cutter->GetPointData(); 
	float* buffer = ArrayGet3<float>(pd, key, length); 

	UE_LOG(LogTemp, Warning, TEXT("[Vti2AnatomicalView] length : %d "), length);
	return buffer; 
}


//----------------------------------------------------------------------------------|  |--//
void Vti2SurfaceBuffer( 
						vtkImageData* domain, 
						float value, 
						float valueMin, 
						float valueMax, 
						vtkPolyData*& surface, 
						float*& points, 
						int& n_rows, 
						int& n_cols, 
						long long*& triangles, 
						int& n_triangles  // ->  n_triangles == n_raw / 4 !!
					) 
{
	// Domain parameters  
	if(domain == nullptr) 
	{
		UE_LOG(LogTemp, Warning, TEXT("[Vti2Surface] 'domain' fails!") );
		return ;
	}

	// Creating surface 
	std::string key = "voxels"; 
	float threshold = NormalizeToRange(value, valueMin, valueMax, 0.0, 1.0); 

	bool getscalars = true; 

	vtkDataObject* obj = nullptr;  
	obj = GetContour(domain, key, threshold, getscalars); 
	obj = CenterAtOrigin( static_cast<vtkPolyData*>(obj) ); 

	if(surface)
	{
		surface->Delete();
		surface = nullptr; 
	}	
	surface = static_cast<vtkPolyData*>(obj);

	// Extracting data 
	int NumberOfCells = surface->GetNumberOfCells(); 
	int NumberOfCellsMax = 2e6; 
	if(NumberOfCells > NumberOfCellsMax) return ; 

	//int n_rows, n_cols; 
	//float* 
	points = GetVertices(surface, n_rows, n_cols); 
	UE_LOG(LogTemp, Warning, TEXT("[Vti2Surface] n_rows = %d n_cols = %d"), n_rows, n_cols);

	//int n_triangles; 
	long long n_raw; 
	//long long* 
	triangles = GetTriangles(surface, n_triangles, n_raw);
	UE_LOG(LogTemp, Warning, TEXT("[Vti2Surface] n_triangles = %d x 4 -> n_raw = %d"), n_triangles, n_raw);

    if( (n_triangles * 4) != n_raw)
    {
	    UE_LOG(LogTemp, Error, TEXT("[Vti2Surface] 'n_triangles == n_raw' fails !!"));
        return ; 
    }

/*
	Vertices.Reset();
	for(int i=0; i<n_rows; i++)
	{
		//for(int j=0; j<n_cols; j++, k++) float var = points[k]; 
		float x = points[i * n_cols + 0]; 
		float y = points[i * n_cols + 1]; 
		float z = points[i * n_cols + 2]; 	
		Vertices.Add( FVector(x,y,z) );
	}

	int32 nVertices = Vertices.Num();
	UE_LOG(LogTemp, Warning, TEXT("[SpicyTech] nVertices = %d "), nVertices);

	Triangles.Reset();
	for (int i = 0; i < n_triangles; ++i) 
	{
		int t0 = triangles[i * 4 + 0]; 
		int t1 = triangles[i * 4 + 1]; 
		int t2 = triangles[i * 4 + 2]; 
		int t3 = triangles[i * 4 + 3]; 
		Triangles.Add(t1);
		Triangles.Add(t2);
		Triangles.Add(t3);
	}

	int32 nTriangles = Triangles.Num();
	UE_LOG(LogTemp, Warning, TEXT("[SpicyTech] nTriangles = %d (%d) "), nTriangles, n_triangles * 3);
*/
}

//----------------------------------------------------------------------------------|  |--//

