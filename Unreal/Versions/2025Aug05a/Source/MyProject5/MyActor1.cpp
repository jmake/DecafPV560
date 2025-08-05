// Fill out your copyright notice in the Description page of Project Settings.

#include "MyActor1.h"
//#include "Kismet/GameplayStatics.h"

#include "StaticTest.h"
#include "ExternalLibrary.h"
#include <vtkImageData.h> 
#include "Sources/vtktools2.hpp"


//----------------------------------------------------------------------------------|  |--//
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


float* GetVertices(vtkPolyData* vtp, int& n_rows, int& n_cols)
{
	if(vtp == nullptr) return nullptr; 

	vtkPoints* pts = vtp->GetPoints(); 
	return GetCppArray3<float>( pts->GetData(), &n_rows, &n_cols ); 
}



//----------------------------------------------------------------------------------|  |--//
//------------------------------------------------------------------------------|  vti |--//
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


float NormalizeToRange(float x, float new_min, float new_max, float arr_min = 0.0f, float arr_max = 1.0f)
{
    float scaled = (x - arr_min) / (arr_max - arr_min);
    float mapped = scaled * (new_max - new_min) + new_min;
    return mapped;
}


TArray<float> VtiTest(int& width, int& height) 
{
	std::string fname = "F:/z2025_1/Dicom/DecafPV560/domain.vti"; 

	vtkImageData* vti = ReadVTIFile(fname); 
	CenterImageAtOrigin(vti); 

	double center[3]; 
	vti->GetCenter( center ); 
	UE_LOG(LogTemp, Warning, TEXT("[SpicyTech] center : (%f, %f, %f)"), center[0], center[1], center[2]);

	double bounds[6]; // [xmin, xmax, ymin, ymax, zmin, zmax]
    vti->GetBounds( bounds );
	UE_LOG(LogTemp, Warning, TEXT("[SpicyTech] xmin,xmax : (%f, %f)"), bounds[0], bounds[1]);
	UE_LOG(LogTemp, Warning, TEXT("[SpicyTech] ymin,ymax : (%f, %f)"), bounds[2], bounds[3]);
	UE_LOG(LogTemp, Warning, TEXT("[SpicyTech] zmin,zmax : (%f, %f)"), bounds[4], bounds[5]);

	float vmin =  0.0;
	float vmax =  1.0;  	

	float x0 = 0.5; 
	float x = NormalizeToRange(x0, bounds[0], bounds[1], vmin, vmax); 
	UE_LOG(LogTemp, Warning, TEXT("[CreateContour] x0 : %f -> %f"), x0, x);

	float y0 = 0.5f;
	float y = NormalizeToRange(y0, bounds[2], bounds[3], vmin, vmax);
	UE_LOG(LogTemp, Warning, TEXT("[CreateContour] y0 : %f -> %f"), y0, y);

	float z0 = 0.5f;
	float z = NormalizeToRange(z0, bounds[4], bounds[5], vmin, vmax);
	UE_LOG(LogTemp, Warning, TEXT("[CreateContour] z0 : %f -> %f"), z0, z);


    double anatomicalView[9] = {1, 0,  0, 0, 1,  0, 0, 0, 1}; // Axial. dimensions : (512, 512, 1) ??  
	//double anatomicalView[9] = {1, 0,  0, 0, 0, -1, 0, 1, 0}; // Coronal. dimensions: (512, 429, 1) 
    //double anatomicalView[9] = {0, 0, -1, 1, 0,  0, 0, 1, 0}; // Sagittal. dimensions: (429, 512, 1) 
	double orig[3] = {center[0] + x, center[1] + y, center[2] + z}; 
	vtkDataObject* obj = CutterPlane3(vti, orig, anatomicalView);
	vtkImageData* cutter = static_cast<vtkImageData*>(obj);  

	int dimensions[3];		
	cutter->GetDimensions(dimensions); 
	UE_LOG(LogTemp, Warning, TEXT("[CreateContour] dimensions: x=%d, y=%d, z=%d"), dimensions[0], dimensions[1], dimensions[2]);

	int length = 0; 
	std::string key = "ImageScalars";  
	vtkPointData* pd = cutter->GetPointData(); 

	float *buffer = ArrayGet3<float>(pd, key, length); 
	//std::vector<float> buffer = ArrayGet2<float>(pd, key); length = buffer.size();
	//std::vector<float> buffer; ArrayGet(pd, key, buffer); length = buffer.size(); 
	UE_LOG(LogTemp, Warning, TEXT("[CreateContour] length : %d "), length);

	//TArray<double> View; View.SetNumZeroed(length);	for(int i=0; i< length; i++) View[i] = buffer[i];  

	width = dimensions[0]; 
	height = dimensions[1]; 

	//TArrayView<double> View(buffer, length); // crash ... 
	TArray<float> View; View.Append(buffer, length); // crash ...
	return View; 
}


//----------------------------------------------------------------------------------|  |--//
//-------------------------------------------------------------------------------| vtp |--//
void VtpTest()
{
	// LogTemp: Warning: [SpicyTech] n_rows = 382824 n_cols = 3
	// LogTemp: Warning: [SpicyTech] NumberOfCells = 764550
	// LogTemp: Warning: [SpicyTech] n_triangles = 764550 n_raw = 3058200 -> n_raw == n_triangles * 4  

	std::string fname = "F:/z2025_1/Dicom/DecafPV560/contour.vtp"; 
	vtkDataObject* obj = PReaderSerial(fname); 
	vtkPolyData* vtp = static_cast<vtkPolyData*>(obj); 

	int NumberOfCells = vtp->GetNumberOfCells(); 
	UE_LOG(LogTemp, Warning, TEXT("[SpicyTech] NumberOfCells = %d"), NumberOfCells);

	int n_rows, n_cols; 
	GetVertices(vtp, n_rows, n_cols); 
	UE_LOG(LogTemp, Warning, TEXT("[SpicyTech] n_rows = %d n_cols = %d"), n_rows, n_cols);

    int n_triangles; 
    long long n_raw; 
    long long* triangles = GetTriangles(vtp, n_triangles, n_raw);
	UE_LOG(LogTemp, Warning, TEXT("[SpicyTech] n_triangles = %d n_raw = %d"), n_triangles, n_raw);
}


//----------------------------------------------------------------------------------|  |--//
void Vti2Mesh(TArray<FVector>& Vertices, TArray<int32>& Triangles)
{
	// n_rows = 382824 (n_cols = 3)
	// n_triangles = 764550 (n_raw = 3058200)
	std::string fname = "F:/z2025_1/Dicom/DecafPV560/contour.vtp"; 
	vtkDataObject* obj = PReaderSerial(fname); 
	vtkPolyData* vtp = static_cast<vtkPolyData*>(obj); 

	obj = CenterAtOrigin(vtp); 
	vtp = static_cast<vtkPolyData*>(obj);

	int NumberOfCells = vtp->GetNumberOfCells(); 
	UE_LOG(LogTemp, Warning, TEXT("[SpicyTech] NumberOfCells = %d"), NumberOfCells);

	int n_rows, n_cols; 
	float* points = GetVertices(vtp, n_rows, n_cols); 
	UE_LOG(LogTemp, Warning, TEXT("[SpicyTech] n_rows = %d n_cols = %d"), n_rows, n_cols);

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


	int n_triangles; 
	long long n_raw; 
	long long* triangles = GetTriangles(vtp, n_triangles, n_raw);
	UE_LOG(LogTemp, Warning, TEXT("[SpicyTech] n_triangles = %d n_raw = %d"), n_triangles, n_raw / 4);

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
} 



//----------------------------------------------------------------------------------|  |--//
//--------------------------------------------------------------------------------| UE |--//

void LibrariesTest() 
{
	auto* externalLibrary = new ExternalLibrary(); 
	float dynamicResult = externalLibrary->Sqrt(2);
	
	FString UEMessage = FString("DynamicResult:") + FString::SanitizeFloat(dynamicResult);
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 9999, FColor::Green, UEMessage); 	

	auto* staticTest = new StaticTest(); 
	float staticResult = staticTest->Sqrt(3);
	FString UEMessage2 = FString("    StaticResult:") + FString::SanitizeFloat(staticResult);
	if (GEngine) GEngine->AddOnScreenDebugMessage(0, 9999, FColor::Blue, UEMessage2, true); 	
}


TArray<float> GenerateChessboardData(int32 W, int32 H, int32 NumSquaresX, int32 NumSquaresY)
{
    TArray<float> Data;
    Data.SetNumZeroed(W * H);

    // Compute square size in pixels
    int32 SquareWidth = W / NumSquaresX;
    int32 SquareHeight = H / NumSquaresY;

    for (int32 Y = 0; Y < H; Y++)
    {
        for (int32 X = 0; X < W; X++)
        {
            int32 SquareX = X / SquareWidth;
            int32 SquareY = Y / SquareHeight;

            // Alternate colors like a chessboard: black (0.0) and white (1.0)
            bool bIsWhite = (SquareX + SquareY) % 2 == 0;
            Data[Y * W + X] = bIsWhite ? 1.0f : 0.0f;
        }
    }

    return Data;
}


TArray<float> GeneratePerlinData(int32 W, int32 H, float Time, float Scale)
{
    TArray<float> Data;
    Data.SetNumZeroed(W * H);

    for (int32 Y = 0; Y < H; Y++)
    {
        for (int32 X = 0; X < W; X++)
        {
            float FX = (float)X * Scale + Time;
            float FY = (float)Y * Scale + Time;
            float Value = FMath::PerlinNoise2D(FVector2D(FX, FY));
            Data[Y * W + X] = (Value + 1.0f) * 0.5f;
        }
    }

    return Data;
}


void TexturecreateTest(TArrayView<float> Data, int width, int height, UWorld* World)  
{
	FVector Position = FVector(-500.0, 100.0, 100.0); 
	FRotator Rotation = FRotator(-90.0, 0.0, 0.0); 

	float Time = World->GetTimeSeconds();

	APixelTextureDisplay* Display = nullptr; 
	Display = World->SpawnActor<APixelTextureDisplay>(APixelTextureDisplay::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
	if (Display)
	{
		Display->ActorRotation = Rotation; 
		Display->ActorPosition = Position; 

		Display->Init(width, height);
		Display->Apply(Data, 1.0f);
		Display->Save(); 
	}	
}


void CenterMeshAt(UProceduralMeshComponent* mesh, const FVector& center)
{
    if (!mesh) return;

    FBox Bounds = mesh->CalcBounds(mesh->GetComponentTransform()).GetBox();
    FVector MeshCenter = Bounds.GetCenter();

    FVector Offset = center - MeshCenter;
    mesh->AddLocalOffset(Offset);
}


void DrawMeshBounds(UProceduralMeshComponent* mesh)
{
    if (!mesh || !mesh->GetWorld()) return;

    const FBoxSphereBounds Bounds = mesh->CalcBounds(mesh->GetComponentTransform());
    DrawDebugBox(
        mesh->GetWorld(),
        Bounds.Origin,
        Bounds.BoxExtent,
        FColor::Red,
        true,     // persistent
        999.0f,    // duration
        0,        // depth priority
        2.0f      // line thickness
    );

    UE_LOG(LogTemp, Warning, TEXT("Bounds: Origin=%s, Extent=%s"),
        *Bounds.Origin.ToString(), *Bounds.BoxExtent.ToString());
}


UMaterial* CreateDebugMaterial()
{
    static ConstructorHelpers::FObjectFinder<UMaterial> Mat(TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
    if (Mat.Succeeded())
    {
        return Mat.Object;
    }
	
	return nullptr;
}


UMaterialInstanceDynamic* ApplyDebugMaterial(UProceduralMeshComponent* mesh, UMaterial* material, const FLinearColor& Color = FLinearColor::Green)
{
    if (!mesh || !material) return nullptr;

    // Create a dynamic material instance from the given material
    UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(material, mesh);
    if (DynMat)
    {
        DynMat->SetVectorParameterValue(FName("Color"), Color); //FLinearColor::Green);
        mesh->SetMaterial(0, DynMat);
		return DynMat; 
	}

	return nullptr; 
}


void CreateCubeMesh(float size, const FVector& center, TArray<FVector>& Vertices, TArray<int32>& Triangles)
{
    Vertices.Reset();
    Triangles.Reset();

    const float h = size * 0.5f;

    const FVector P[8] = {
        center + FVector(-h, -h, -h), // 0
        center + FVector( h, -h, -h), // 1
        center + FVector( h,  h, -h), // 2
        center + FVector(-h,  h, -h), // 3
        center + FVector(-h, -h,  h), // 4
        center + FVector( h, -h,  h), // 5
        center + FVector( h,  h,  h), // 6
        center + FVector(-h,  h,  h)  // 7
    };

    for (int i = 0; i < 8; ++i) Vertices.Add(P[i]);

	const int32 F[36] = {
		0, 1, 2,  0, 2, 3, // Bottom face
		4, 6, 5,  4, 7, 6, // Top face
		0, 5, 1,  0, 4, 5, // Front face
		1, 6, 2,  1, 5, 6, // Right face
		2, 7, 3,  2, 6, 7, // Back face
		3, 4, 0,  3, 7, 4 // Left face
	};

	// wound counter-clockwise (CCW)
	for (int i = 0; i < 36; ++i) Triangles.Add(F[i]);
}


void DebugMeshNormals(UWorld* World, const TArray<FVector>& Vertices, const TArray<int32>& Triangles)
{
    if (!World) return;

    for (int32 i = 0; i < Triangles.Num(); i += 3)
    {
        const FVector& A = Vertices[Triangles[i]];
        const FVector& B = Vertices[Triangles[i + 1]];
        const FVector& C = Vertices[Triangles[i + 2]];

        FVector Normal = -FVector::CrossProduct(B - A, C - A).GetSafeNormal();
        FVector Center = (A + B + C) / 3;

		float scale = 50.0f; 
        DrawDebugLine(World, Center, Center + Normal * scale, FColor::Green, false, 1000.0f, 0, 2.5f);
    }

    UE_LOG(LogTemp, Warning, TEXT("DebugMeshNormals: %d triangle normals drawn."), Triangles.Num() / 3);
}


void CreateMesh(UProceduralMeshComponent* mesh, UWorld* World)
{
	/*
	// camera position -> FVector(-200, 0, 0) ?? 
	TArray<int32> Triangles;
    TArray<FVector> Vertices;

	float size = 100.0f; 
	CreateCubeMesh(size, FVector(-5 * size, 0, size*0.5), Vertices, Triangles);
	DebugMeshNormals(World, Vertices, Triangles); 
	*/
	TArray<int32> Triangles;
    TArray<FVector> Vertices;
	Vti2Mesh(Vertices, Triangles); 

    // Dummy empty arrays
    TArray<FVector> Normals;
    TArray<FVector2D> UV0;
    TArray<FLinearColor> VertexColors;
    TArray<FProcMeshTangent> Tangents;

    mesh->CreateMeshSection_LinearColor(
		0, Vertices, Triangles,
		Normals, UV0, VertexColors, Tangents, 
		true,   // bCreateCollision — set to true to enable collision
		true   // bCalculateNormals
    );

	UMaterialInterface* DefaultMat = UMaterial::GetDefaultMaterial(EMaterialDomain::MD_Surface);
	//mesh->SetMaterial(0, DefaultMat);
	/*
    UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(DefaultMat, mesh);
	DynMat->SetVectorParameterValue(FName("BaseColor"), FLinearColor::Green);
	mesh->SetMaterial(0, DynMat);
	*/
}




//----------------------------------------------------------------------------------|  |--//
//-----------------------------------------------------------------------------| Actor |--//
AMyActor1::AMyActor1()
{
	PrimaryActorTick.bCanEverTick = true; // Set this actor to call Tick() every frame.  

	DebugMaterial = CreateDebugMaterial(); 

    Mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("GeneratedMesh"));
    RootComponent = Mesh;
    Mesh->bUseAsyncCooking = true;
}


// Called when the game starts or when spawned
void AMyActor1::BeginPlay()
{
	Super::BeginPlay();

	LibrariesTest(); 

	IsoSourface(); 

	CutterAxial(); 
} 



void AMyActor1::IsoSourface()
{	
	VtpTest(); 

	CreateMesh(Mesh, GetWorld()); 
	DynMaterialInstance = ApplyDebugMaterial(Mesh, DebugMaterial, DebugColor);

	CenterMeshAt(Mesh, FVector(-500.0, 0.0, 100.0));

	DrawMeshBounds(Mesh); 
} 


void AMyActor1::CutterAxial() 
{
	int width = 128 * 2; 
	int height = 128 * 1; 

	int NumSquaresX = 2; 
	int NumSquaresY = NumSquaresX * 2;

	TArray<float> data;  

	data = GenerateChessboardData(width, height, NumSquaresX, NumSquaresY); 
	TexturecreateTest(data, width, height, GetWorld()); 

	data = VtiTest(width, height); 	
	UE_LOG(LogTemp, Warning, TEXT("[VtiTest] width : %d, height : %d, Num : %d"), width, height, data.Num() );

	TexturecreateTest(data, width, height, GetWorld()); 
}


void AMyActor1::Test1()
{
	UE_LOG(LogTemp, Warning, TEXT("Test1"));
} 


void AMyActor1::Test2(float value)
{
    UE_LOG(LogTemp, Warning, TEXT("Test2 value : %f "), value );
} 


void AMyActor1::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    if (DynMaterialInstance)
    {
        DynMaterialInstance->SetVectorParameterValue(FName("Color"), DebugColor);
    }	

	//UpdateActorTransformIfChanged(); 

}


void AMyActor1::UpdateActorTransformIfChanged()
{
    if (!ActorPosition.Equals(LastPosition) || !ActorRotation.Equals(LastRotation))
    {
        SetActorLocation(ActorPosition);
        SetActorRotation(ActorRotation);

        LastPosition = ActorPosition;
        LastRotation = ActorRotation;
    }
}


/*
1) 
	ERROR : 
		... Cannot open include file: 'SpicyWrapper.h': ...
	SOLUTION : 
		In 'Source\MyProject5\MyProject5.Build.cs' add 'SpicyWrapper' -> 

		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", 
			"SpicyWrapper"
		});

2) 
	In 'Plugins\SpicyWrapper\Source\SpicyWrapper\SpicyWrapper.Build.cs' add -> 

	public SpicyWrapper(ReadOnlyTargetRules Target) : base(Target)
	{
        Type = ModuleType.External;
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "include"));
        PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "lib", "MyMathLib.lib"));
	}

*/