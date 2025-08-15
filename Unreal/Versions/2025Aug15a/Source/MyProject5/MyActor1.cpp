#include "MyActor1.h"

#include "StaticTest.h"
#include "ExternalLibrary.h"

//#include "Sources/vtktools2.hpp"
#include "Sources/vtktools4.hpp"


//----------------------------------------------------------------------------------|  |--//
//-------------------------------------------------------------------------------| vtp |--//
//void Vti2UEMesh(std::string fname, TArray<FVector>& Vertices, TArray<int32>& Triangles)
void Vti2UEMesh(vtkPolyData* vtp, TArray<FVector>& Vertices, TArray<int32>& Triangles)
{
	//// n_rows = 382824 (n_cols = 3)
	//// n_triangles = 764550 (n_raw = 3058200)
	//std::string fname = "F:/z2025_1/Dicom/DecafPV560/contour.vtp"; 
	//vtkDataObject* obj = PReaderSerial(fname); 
	//vtkPolyData* vtp = static_cast<vtkPolyData*>(obj); 

	//obj = CenterAtOrigin(vtp); 
	//vtp = static_cast<vtkPolyData*>(obj);

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
void LibrariesTest(); 
TArray<float> GenerateChessboardData(int32 W, int32 H, int32 NumSquaresX, int32 NumSquaresY); 

UMaterial* CreateDebugMaterial(); 
UMaterialInstanceDynamic* ApplyDebugMaterial(UProceduralMeshComponent* mesh, UMaterial* material, const FLinearColor& Color = FLinearColor::Green); 


APixelTextureDisplay* TexturecreateTest(
						TArrayView<float> Data, 
						int width, int height, 
						float spacingx, float spacingy, 
						UWorld* World, 
						std::string fname
					)  
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

		Display->Init(width, height, spacingx, spacingy);
		Display->Apply(Data, 1.0f);
		Display->Save(fname); 
	}	

	return Display; 
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


void CreateIsoSurface(vtkPolyData* vtp, UProceduralMeshComponent* mesh, UWorld* World)
{
	TArray<int32> Triangles;
    TArray<FVector> Vertices;
	Vti2UEMesh(vtp, Vertices, Triangles); 

    // Dummy empty arrays
    TArray<FVector> Normals;
    TArray<FVector2D> UV0;
    TArray<FLinearColor> VertexColors;
    TArray<FProcMeshTangent> Tangents;

	mesh->ClearAllMeshSections();
    mesh->CreateMeshSection_LinearColor(
		0, Vertices, Triangles,
		Normals, UV0, VertexColors, Tangents, 
		true,   // bCreateCollision — set to true to enable collision
		true   // bCalculateNormals
    );

	UMaterialInterface* DefaultMat = UMaterial::GetDefaultMaterial(EMaterialDomain::MD_Surface);
}


void TestIsoSurface(UProceduralMeshComponent* mesh, UWorld* World)
{
	std::string fname = "F:/z2025_1/Dicom/DecafPV560/contour.vtp"; 
	vtkDataObject* obj = PReaderSerial(fname); 
	//vtkPolyData* vtp = static_cast<vtkPolyData*>(obj); 

	vtkPolyData* vtp = nullptr; 
	obj = CenterAtOrigin( static_cast<vtkPolyData*>(obj) ); 
	vtp = static_cast<vtkPolyData*>(obj);

	//UProceduralMeshComponent* mesh = nullptr;
    //mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("GeneratedMesh"));
	//mesh->ClearAllMeshSections();
    //mesh->DestroyComponent();
	CreateIsoSurface(vtp, mesh, World); 
} 

//----------------------------------------------------------------------------------|  |--//
//---------------------------------------------------------------------| CutterManager |--//
void CutterManager::Finish() 
{
	if(cutter)
	{
		cutter->Delete(); 
		cutter = nullptr; 
	}

	if (texture && !texture->IsPendingKillPending()) texture->Destroy();	
}


void CutterManager::SetVti(vtkImageData* vti_ptr, UWorld* uworld) 
{
	//vtkImageData* vti = ReadVTIFile(fname); 
	//CenterImageAtOrigin(vti); 

	double center[3]; 
	vti_ptr->GetCenter( center ); 

	double bounds[6]; // [xmin, xmax, ymin, ymax, zmin, zmax]
    vti_ptr->GetBounds( bounds );

	int dimensions[3];
	vti_ptr->GetDimensions(dimensions); 

	this->cutter = nullptr; 
	this->domain = vti_ptr; 
	this->world = uworld; 
}


void  CutterManager::CreateTextureAsync(
										APixelTextureDisplay* actor, 
										std::string name, 
										double anatomicalView[9], 
										float x0, float y0, float z0, 
										TFunction<void(APixelTextureDisplay*)> After
									) 
{
	if(domain == nullptr) return ;

	struct FCreateResult
	{
		float* Buffer;
		int32 Width;
		int32 Height;
		float SpacingX;
		float SpacingY;
	};

	auto Promise = MakeShared< TPromise<FCreateResult> >();
	TFuture<FCreateResult> Future = Promise->GetFuture();

    Async(EAsyncExecution::Thread, [this, Promise, anatomicalView, x0, y0, z0]()
    {
    	FCreateResult Result;
		Result.Width = -1;
		Result.Height = -1;
		Result.SpacingX = -1.0f;
		Result.SpacingY = -1.0f;

    	Result.Buffer = Create(anatomicalView, x0, y0, z0, Result.Width, Result.Height, Result.SpacingX, Result.SpacingY);
		Promise->SetValue( Result );
	});

    Future.Next([this, &actor, name, After](const FCreateResult& Result)
    {
        AsyncTask(ENamedThreads::GameThread, [this, &actor, name, After, Result]()
        {
        	UE_LOG(LogTemp, Warning, TEXT("[CreateTextureAsync] Width: %d Height: %d SpX: %f SpY: %f"), Result.Width, Result.Height, Result.SpacingX, Result.SpacingY);

			TArray<float> data;  
			data.Append(Result.Buffer, Result.Width * Result.Height);
			UE_LOG(LogTemp, Warning, TEXT("[CreateTextureAsync] Num : %d"), data.Num() );

			APixelTextureDisplay* display = nullptr; 
			display = world->SpawnActor<APixelTextureDisplay>(APixelTextureDisplay::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
			display->Init(Result.Width, Result.Height, Result.SpacingX, Result.SpacingY);
			display->Apply(data, 1.0f);
			//display->Save(fname); 
			//display->SetActorLabel(UTF8_TO_TCHAR(name.c_str())); // Segmentation 
			UTexture2D* texture2D = display->DynamicTexture; 

if(texture2D == nullptr)
{
	UE_LOG(LogTemp, Warning, TEXT("[CreateTextureAsync] fails !!") );
	return ;
}

UE_LOG(LogTemp, Warning, TEXT("[CreateTextureAsync] Size: %d X %d"), texture2D->GetSizeX(), texture2D->GetSizeY());

			After( display ); 
		});
    });
/*
	// Create Anatomical View (domain -> vti)
	int width = -1, height = -1; 
	float spacingx = -1.0, spacingy = -1.0; 
	float* buffer = Create(anatomicalView, x0, y0, z0, width, height, spacingx, spacingy); 

	// vti -> texture2D
	TArray<float> data;  
	data.Append(buffer, width * height);
	UE_LOG(LogTemp, Warning, TEXT("[CreateTextureAsync] width : %d, height : %d, Num : %d"), width, height, data.Num() );

	APixelTextureDisplay* display = nullptr; 
	display = world->SpawnActor<APixelTextureDisplay>(APixelTextureDisplay::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
	if (display == nullptr) return ; 

	display->Init(width, height, spacingx, spacingy);
	display->Apply(data, 1.0f);
	//display->Save(fname); 

	UE_LOG(LogTemp, Warning, TEXT("[CreateTextureAsync] 'APixelTextureDisplay' created!") );

	UTexture2D* texture2D = display->DynamicTexture; 
	if (texture2D == nullptr) return ; 
*/
	//return TexturecreateTest(data, width, height, spacingx, spacingy, world, fname); 
}


APixelTextureDisplay* CutterManager::CreateAxialTexture(float x0, float y0, float z0, std::string fname)
{
	double Axial[9] = {1, 0,  0, 0, 1,  0, 0, 0, 1}; 
	return CreateTexture(Axial, x0, y0, z0, fname); 
} 


APixelTextureDisplay* CutterManager::CreateCoronalTexture(float x0, float y0, float z0, std::string fname)
{
	double Coronal[9] = {1, 0,  0, 0, 0, -1, 0, 1, 0};
	return CreateTexture(Coronal, x0, y0, z0, fname); 
} 


APixelTextureDisplay* CutterManager::CreateSagittalTexture(float x0, float y0, float z0, std::string fname)
{
  	double Sagittal[9] = {0, 0, -1, 1, 0,  0, 0, 1, 0}; 
	return CreateTexture(Sagittal, x0, y0, z0, fname); 
} 


APixelTextureDisplay* CutterManager::CreateTexture(double anatomicalView[9], float x0, float y0, float z0, std::string fname) 
{
	if(domain == nullptr) return nullptr;

	int width = -1, height = -1; 
	float spacingx = -1.0, spacingy = -1.0; 
	float* buffer = Create(anatomicalView, x0, y0, z0, width, height, spacingx, spacingy); 

	TArray<float> data;  

	data.Empty(); 
	data.Append(buffer, width * height);

UE_LOG(LogTemp, Warning, TEXT("[CreateTexture] width : %d, height : %d, Num : %d"), width, height, data.Num() );

	//if (texture && !texture->IsPendingKillPending()) texture->Destroy(); 
	texture = TexturecreateTest(data, width, height, spacingx, spacingy, world, fname); 
	return texture; 
}


float* CutterManager::CreateAxial(float x0, float y0, float z0, int& width, int& height, float& sizeX, float& sizeY)
{
	double Axial[9] = {1, 0,  0, 0, 1,  0, 0, 0, 1}; 
	return Create(Axial, x0, y0, z0, width, height, sizeX, sizeY); 
} 


float* CutterManager::CreateCoronal(float x0, float y0, float z0, int& width, int& height, float& sizeX, float& sizeY)
{
	double Coronal[9] = {1, 0,  0, 0, 0, -1, 0, 1, 0};
	return Create(Coronal, x0, y0, z0, width, height, sizeX, sizeY); 
} 


float* CutterManager::CreateSagittal(float x0, float y0, float z0, int& width, int& height, float& sizeX, float& sizeY)
{
  	double Sagittal[9] = {0, 0, -1, 1, 0,  0, 0, 1, 0}; 
	return Create(Sagittal, x0, y0, z0, width, height, sizeX, sizeY); 
} 


float* CutterManager::Create(
								double anatomicalView[9], 
							 	float x0, float y0, float z0, 
							 	int& width, int& height, 
								float& sizeX, float& sizeY 
							)
{
	float* buffer = nullptr; 

	if(domain == nullptr) return buffer;

	if(cutter)
	{
		cutter->Delete();
		cutter = nullptr; 
	}

	double bounds[6]; // [xmin, xmax, ymin, ymax, zmin, zmax]
    domain->GetBounds( bounds );

	float vmin = 0.0;
	float vmax = 1.0; 

	//float x0 = 0.5; 
	float x = NormalizeToRange(x0, bounds[0], bounds[1], vmin, vmax); 
UE_LOG(LogTemp, Warning, TEXT("[CutterManager] x0 : %f -> %f"), x0, x);

	//float y0 = 0.5f;
	float y = NormalizeToRange(y0, bounds[2], bounds[3], vmin, vmax);
UE_LOG(LogTemp, Warning, TEXT("[CutterManager] y0 : %f -> %f"), y0, y);

	//float z0 = 0.5f;
	float z = NormalizeToRange(z0, bounds[4], bounds[5], vmin, vmax);
UE_LOG(LogTemp, Warning, TEXT("[CutterManager] z0 : %f -> %f"), z0, z);

	double center[3]; 
	domain->GetCenter( center ); 

	double orig[3] = {center[0] + x, center[1] + y, center[2] + z}; 
	vtkDataObject* obj = CutterPlane3(domain, orig, anatomicalView);
	//vtkImageData* 
	cutter = static_cast<vtkImageData*>(obj);  

	int dimensions[3];
	cutter->GetDimensions(dimensions); 
	width = dimensions[0]; 
	height = dimensions[1];
UE_LOG(LogTemp, Warning, TEXT("[CreateContour] dimensions: x=%d, y=%d, z=%d"), dimensions[0], dimensions[1], dimensions[2]);

	double spacing[3];
	cutter->GetSpacing(spacing);  
	sizeX = spacing[0]; 
	sizeY = spacing[1]; 
UE_LOG(LogTemp, Warning, TEXT("[CreateContour] spacing: x=%.2f, y=%.2f, z=%.2f"), spacing[0], spacing[1], spacing[2]);

	std::string key = "ImageScalars";  
	vtkPointData* pd = cutter->GetPointData(); 

	int length = -1; 
	buffer = ArrayGet3<float>(pd, key, length); 
	return buffer; 
}


//----------------------------------------------------------------------------------|  |--//
//-----------------------------------------------------------------------------| Actor |--//
AMyActor1::~AMyActor1()
{
/*
	for (auto* c : cutters)
	{
		//c->Finish(); // vector deleting destructor...
		c = nullptr; 
	} 
	cutters.clear(); 
*/
	if(vti_ptr)
	{
		vti_ptr->Delete(); 
		vti_ptr = nullptr; 
	}

	if (textureActor && !textureActor->IsPendingKillPending()) textureActor->Destroy();
} 


AMyActor1::AMyActor1()
{
	PrimaryActorTick.bCanEverTick = true; // -> 'Tick' every frame.  

	DebugMaterial = CreateDebugMaterial(); 

    Mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("GeneratedMesh"));
    RootComponent = Mesh;
    Mesh->bUseAsyncCooking = true;

	VtpTest(); 
}


void AMyActor1::BeginPlay()
{
	Super::BeginPlay();

	//LibrariesTest();  // :) 
	//TextureTest(); // -> chessboard

	//LoadVtiFile(); 
	//CreateSourface(0.5); 
	//CreateAxial(0.5); 
} 


void AMyActor1::LoadVtiFile(FString& filename) 
{
	if(vti_ptr)
	{ 
		vti_ptr->Delete();
		vti_ptr = nullptr; 
	} 

	//std::string fname = "F:/z2025_1/Dicom/DecafPV560/domain.vti"; 
	vti_ptr = ReadVTIFile( TCHAR_TO_UTF8(*filename) ); 
	CenterImageAtOrigin(vti_ptr); 

	range = std::vector<double>(2, std::numeric_limits<double>::max()); 
	vtkPointData* pd = vti_ptr->GetPointData(); 
  	vtkDataArray* array = pd->GetArray("voxels");
    array->GetRange(range.data()); 
    //std::cout <<"[RangeGet] range["<< key << "] : (" << range[0] <<", "<< range[1] <<") \n";

	UE_LOG(LogTemp, Warning, TEXT("[LoadVtiFile] ok!"));
}


void AMyActor1::CreateTextureAsync(
											APixelTextureDisplay* actor, 
											double view[9], 
											double center[3], 
											TFunction<void(APixelTextureDisplay*)> After
										)
{
	if(vti_ptr == nullptr) return ; 

	CutterManager *cutter = new CutterManager(); 
	cutter->SetVti(vti_ptr, GetWorld()); 
	/*
	double    Axial[9] = {1, 0,  0, 0, 1,  0, 0, 0, 1}; 
	double   center[3] = {0.5, 0.5, value};

	double  Coronal[9] = {1, 0,  0, 0, 0, -1, 0, 1, 0};
	double   center[3] = {0.5, value, 0.5};

	double Sagittal[9] = {0, 0, -1, 1, 0,  0, 0, 1, 0}; 
	double   center[3] = {value, 0.5, 0.5};
	*/

//	if (actor && !actor->IsPendingKillPending()) actor->Destroy();

	cutter->CreateTextureAsync(nullptr, "Dummy", view, center[0], center[1], center[2], After); 
}


UTexture2D* AMyActor1::CreateAxial(float value) 
{
	if(vti_ptr == nullptr) return nullptr; 

	FString filename = FString::Printf(TEXT("axial%.0f.png"), value * 1000);

	CutterManager *cutter = new CutterManager(); 
	cutter->SetVti(vti_ptr, GetWorld()); 

	if (textureActor && !textureActor->IsPendingKillPending()) textureActor->Destroy();
	textureActor = cutter->CreateAxialTexture(0.5, 0.5, value,  TCHAR_TO_UTF8(*filename)); 

	if (textureActor == nullptr) return nullptr; 

    UE_LOG(LogTemp, Warning, TEXT("[CreateAxial] value : %f (%s) "), value, *filename);
	return textureActor->DynamicTexture; 
}


void AMyActor1::CreateCoronal(float value) 
{
	if(vti_ptr == nullptr) return ; 

	FString filename = FString::Printf(TEXT("coronal%.0f.png"), value * 1000);

	CutterManager *cutter = new CutterManager(); 
	cutter->SetVti(vti_ptr, GetWorld()); 

	if (textureActor && !textureActor->IsPendingKillPending()) textureActor->Destroy();
	textureActor = cutter->CreateCoronalTexture(0.5, value, 0.5,  TCHAR_TO_UTF8(*filename)); 

    UE_LOG(LogTemp, Warning, TEXT("[CreateCoronal] value : %f (%s) "), value, *filename);
}


void AMyActor1::CreateSagittal(float value) 
{
	if(vti_ptr == nullptr) return ; 

	FString filename = FString::Printf(TEXT("sagittal%.0f.png"), value * 1000);

	CutterManager *cutter = new CutterManager(); 
	cutter->SetVti(vti_ptr, GetWorld()); 

	if (textureActor && !textureActor->IsPendingKillPending()) textureActor->Destroy();
	textureActor = cutter->CreateSagittalTexture(value, 0.5, 0.5, TCHAR_TO_UTF8(*filename)); 

    UE_LOG(LogTemp, Warning, TEXT("[CreateSagittal] value : %f (%s) "), value, *filename);
}


void AMyActor1::CreateSourface(float value, TFunction<void()> After)
{
	if(vti_ptr == nullptr) return ; 

    auto Promise = MakeShared< TPromise<vtkPolyData*> >();
    TFuture< vtkPolyData* > Future = Promise->GetFuture();

    Async(EAsyncExecution::Thread, [this, value, Promise]()
    {
		bool getscalars = true; 
		std::string key = "voxels"; 
		float threshold = NormalizeToRange(value, range[0], range[1], 0.0, 1.0); 

		vtkDataObject* obj = nullptr;  
		obj = GetContour(vti_ptr, key, threshold, getscalars); 
		obj = CenterAtOrigin( static_cast<vtkPolyData*>(obj) ); 

		vtkPolyData* vtp = nullptr; 
		vtp = static_cast<vtkPolyData*>(obj);
		Promise->SetValue( vtp );
    });


    Future.Next([this, value, After](vtkPolyData* vtp)
    {
        AsyncTask(ENamedThreads::GameThread, [this, value, vtp, After]()
        {
			int NumberOfCells = vtp->GetNumberOfCells(); 
			if(NumberOfCells > 2e6) return ; 

			Mesh->ClearAllMeshSections();
			CreateIsoSurface(vtp, Mesh, GetWorld()); 

			DynMaterialInstance = ApplyDebugMaterial(Mesh, DebugMaterial, DebugColor);

			FVector center(500.0, 0.0, 100.0); 
			CenterMeshAt(Mesh, center);

			DrawMeshBounds(Mesh); 

			UE_LOG(LogTemp, Warning, TEXT("[CreateSourface] value : %f "), value);

			After(); 
		});
    });
/*
	//TestIsoSurface(Mesh, GetWorld()); 
	vtkPolyData* vtp = nullptr; 
	vtkDataObject* obj = nullptr;  

	float threshold = NormalizeToRange(value, range[0], range[1], 0.0, 1.0); 

	bool getscalars = true; 
	std::string key = "voxels"; 
	obj = GetContour(vti_ptr, key, threshold, getscalars); 
	obj = CenterAtOrigin( static_cast<vtkPolyData*>(obj) ); 
	vtp = static_cast<vtkPolyData*>(obj);

	int NumberOfCells = vtp->GetNumberOfCells(); 
	if(NumberOfCells > 2e6) return ; 

	Mesh->ClearAllMeshSections();
	CreateIsoSurface(vtp, Mesh, GetWorld()); 

	DynMaterialInstance = ApplyDebugMaterial(Mesh, DebugMaterial, DebugColor);

	CenterMeshAt(Mesh, FVector(-500.0, 0.0, 100.0));

	DrawMeshBounds(Mesh); 
*/
} 


void AMyActor1::TextureTest() 
{
	int width = 128 * 2; 
	int height = 128 * 1; 

	int NumSquaresX = 2; 
	int NumSquaresY = NumSquaresX * 2;

	TArray<float> data;  

	data = GenerateChessboardData(width, height, NumSquaresX, NumSquaresY); 
	TexturecreateTest(data, width, height, 1.0, 1.0, GetWorld(), "chessboard.png"); 
	/*
	float* buffer = VtiTest(width, height); 	
	data.Empty(); 
	data.Append(buffer, width * height);

	UE_LOG(LogTemp, Warning, TEXT("[VtiTest] width : %d, height : %d, Num : %d"), width, height, data.Num() );

	TexturecreateTest(data, width, height, GetWorld(), "test1.png"); 
	*/
}


void AMyActor1::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    if (DynMaterialInstance)
    {
        DynMaterialInstance->SetVectorParameterValue(FName("Color"), DebugColor);
    }	
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


//----------------------------------------------------------------------------------|  |--//
//----------------------------------------------------------------------------------|  |--//
UMaterial* CreateDebugMaterial()
{
    static ConstructorHelpers::FObjectFinder<UMaterial> Mat(TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
    if (Mat.Succeeded())
    {
        return Mat.Object;
    }
	
	return nullptr;
}


UMaterialInstanceDynamic* ApplyDebugMaterial(UProceduralMeshComponent* mesh, UMaterial* material, const FLinearColor& Color)
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



//----------------------------------------------------------------------------------|  |--//
//----------------------------------------------------------------------------------|  |--//
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


//----------------------------------------------------------------------------------|  |--//
//----------------------------------------------------------------------------------|  |--//
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


void TestCubeMesh(UProceduralMeshComponent* mesh, UWorld* World)
{
	// camera position -> FVector(-200, 0, 0) ?? 
	TArray<int32> Triangles;
    TArray<FVector> Vertices;

	float size = 100.0f; 
	CreateCubeMesh(size, FVector(-5 * size, 0, size*0.5), Vertices, Triangles);
	DebugMeshNormals(World, Vertices, Triangles); 
} 


//----------------------------------------------------------------------------------|  |--//
//----------------------------------------------------------------------------------|  |--//
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