#include "Vti2VolumeTexture.h"


//----------------------------------------------------------------------------------|  |--//
TArray<float> DataTiles(
	TArray<float>& dataTotal,  
	int z, 
	TArray<float> dataLocal, 
	int widthLocal, 
	int heightLocal, 
	int widthTotal, 
	int heightTotal
) 
{
	//TArray<float> dataTotal; 
	//dataTotal.Init(0.0f, widthTotal * heightTotal);

	// Tiles per row (fits exactly because of multiples)
	int tilesPerRow = widthTotal / widthLocal;

	// Tile grid position
	int tileX = z % tilesPerRow;
	int tileY = z / tilesPerRow;

	// Offsets in the big array
	int offsetX = tileX * widthLocal;
	int offsetY = tileY * heightLocal;

	for (int yL = 0; yL < heightLocal; yL++)
	{
		for (int xL = 0; xL < widthLocal; xL++)
		{
			int indexL = yL * widthLocal + xL;
			float var = dataLocal[indexL];

			int xG = offsetX + xL;
			int yG = offsetY + yL;

			int indexG = yG * widthTotal + xG;

			dataTotal[indexG] = z; //var; // or z for debug
		}
	}

	return dataTotal;
}

/*
TArray<float> DataTiles(
						int z, 
						TArray<float> dataLocal, 
						int widthLocal, 
						int heightLocal, 
						int widthTotal, 
						int heightTotal 
					) 
{
	TArray<float> dataTotal; 
	dataTotal.Init(0.0f, widthTotal * heightTotal);

	for (int yL = 0; yL < heightLocal; yL++)
	{
		for (int xL = 0; xL < widthLocal; xL++)
		{
			//int indexL = yL * widthLocal + xL;  
			//float var = dataLocal[indexL];

//int indexG = xL + heightTotal * yL + widthLocal * heightLocal * z; // :(  
//int indexG = xL + widthTotal * yL + widthLocal * heightLocal * z; // :/ 
//int indexG = heightLocal * xL + yL + widthLocal * heightLocal * z; // :( 
//int indexG = heightTotal * xL + yL + widthLocal * heightLocal * z; // :( 

			if(indexG < dataTotal.Num() ) dataTotal[indexG] = z; 
		}
	} 

	return dataTotal; 
}
*/


//----------------------------------------------------------------------------------|  |--//
TArray<float> CreateDataTest(int width, int height)
{
	TArray<float> data; 
	data.Init(0.0f, width * height);

	int squareW = width / 2;
	int squareH = height / 2;

	int offsetX = (width - squareW) / 2;
	int offsetY = (height - squareH) / 2;

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			if (x >= offsetX && x < offsetX + squareW &&
				y >= offsetY && y < offsetY + squareH)
			{
				data[y * width + x] = 1.0f; // inside square
			}
			else
			{
				data[y * width + x] = 0.0f; // outside square
			}
		}
	}

	return data; 
}

//----------------------------------------------------------------------------------|  |--//
TArray<float> CreateDataNormal(int width0, int height0, float a)
{
	TArray<float> data;
	data.Init(0.0f, width0 * height0);

	// Center of the grid
	float cx =  width0 / 2.0f;
	float cy = height0 / 2.0f;

	// Maximum possible radius (corner of the grid)
	float rMax = FMath::Sqrt(cx * cx + cy * cy);
	float cutoff = rMax / 2.0f;

	// Exponential values at 0 and cutoff
	float f0 = FMath::Exp(-a * 0.0f);          // = 1.0
	float fCut = FMath::Exp(-a * cutoff);

	for (int y = 0; y < height0; y++)
	{
		for (int x = 0; x < width0; x++)
		{
			float dx = x - cx;
			float dy = y - cy;
			float r = FMath::Sqrt(dx * dx + dy * dy);

			float f = FMath::Exp(-a * r);

			// Normalize so that f(0)=1, f(cutoff)=0
			float value = (f - fCut) / (f0 - fCut);
			value = FMath::Clamp(value, 0.0f, 1.0f);

			data[y * width0 + x] = value;
		}
	}

	return data;
}



//----------------------------------------------------------------------------------|  |--//
void CreateRootComponent(AActor* Owner)
{
	if (!Owner->GetRootComponent())
	{
		USceneComponent* RootComp = NewObject<USceneComponent>(Owner, TEXT("RootSceneComponent"));
		RootComp->RegisterComponent();
		Owner->SetRootComponent(RootComp);

		UE_LOG(LogTemp, Warning, TEXT("[CreateRootComponent] Created RootComponent !!"));
	}
}


//----------------------------------------------------------------------------------|  |--//
UStaticMeshComponent* CreatePlaneComponent(
	AActor* Owner, 
	FVector Scale = FVector(1.0, 1.0, 1.0),
	FVector Location = FVector::ZeroVector, 
	FRotator Rotation = FRotator::ZeroRotator
) 
{
	UStaticMesh* PlaneMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (!PlaneMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CreatePlaneComponent] 'UStaticMesh' fails !!"));
		return nullptr; 
	}

    if (!Owner->GetRootComponent())
    {
		UE_LOG(LogTemp, Warning, TEXT("[CreatePlaneComponent] 'GetRootComponent' fails !!"));
		return nullptr; 
    }
	
    // Create a new StaticMeshComponent at runtime
	UStaticMeshComponent* RuntimeMeshComponent = nullptr; 
	RuntimeMeshComponent = NewObject<UStaticMeshComponent>(Owner);
    RuntimeMeshComponent->SetStaticMesh( PlaneMesh );
    RuntimeMeshComponent->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	RuntimeMeshComponent->RegisterComponent();

	//FVector Scale(sizex, sizey, 1.0);
    RuntimeMeshComponent->SetWorldScale3D( Scale );

	//FVector Location(0.0, 0.0, 0.0);
	RuntimeMeshComponent->SetRelativeLocation(Location);

	//FRotator Rotation(0.0,0.0,0.0); // (??,Z,??)
	RuntimeMeshComponent->SetRelativeRotation(Rotation);

    //SetActorLocation(FVector(300.0f, 0.0f, 100.0f));
    //SetActorRotation(FRotator(0.0f, 45.0f, 0.0f));	

	UE_LOG(LogTemp, Warning, TEXT("[CreatePlaneComponent] Done !!") ) ;
	return RuntimeMeshComponent; 
}


//----------------------------------------------------------------------------------|  |--//
void CreateDrawDebugBox(UStaticMeshComponent* Mesh, UWorld* World)
{
	FVector HalfSize = Mesh->GetStaticMesh()->GetBoundingBox().GetExtent(); // local unscaled
	FVector Scale = Mesh->GetComponentScale();
	HalfSize *= Scale; // apply component scale

	FVector Location = Mesh->GetComponentLocation();
	FQuat Rotation = Mesh->GetComponentQuat();
	FColor Color = FColor::Red;
	float Duration = 0.f;
	bool bPersistent = true;

	DrawDebugBox(World, Location, HalfSize, Rotation, Color, bPersistent, Duration, 0, 2.0f);
}


//----------------------------------------------------------------------------------|  |--//
UMaterialInstanceDynamic* CreateMaterialDynamic(
	AActor* Owner, 
    const std::string& MaterialPath,
    const std::string& VolumeTexParamName = ""
)
{
	//  - Basic Material : 
	//		static ConstructorHelpers::FObjectFinder<UMaterial> Mat(TEXT("/Engine/EngineMaterials/DefaultMaterial")); 
	//
	//  - Creating Material : 
	//		Content Drawer  -> Right Click 
	// 						-> Material (M_RuntimeTexture) 
	//     					-> Add a 'TextureSampleParameter2D' node -> Set parameter name "TextureParameter"
	//                  	-> Connect 'RGB' to 'Base Color' 
	// 
	//  - MaterialPath -> Right Click -> Copy reference

    // Load material asset
    FString PathFString(MaterialPath.c_str());
    UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, *PathFString);
    if (!Material)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CreateMaterialDynamic] Failed to load material: %s"), *PathFString);
        return nullptr;
    }

    // If a parameter name was provided, validate it
	UMaterialInstanceDynamic* DynMat = nullptr; 
    if (!VolumeTexParamName.empty())
    {
        DynMat = UMaterialInstanceDynamic::Create(Material, Owner); 
        if (DynMat)
        {
            UTexture* DummyTex = nullptr;
            FMaterialParameterInfo ParamInfo(FName(VolumeTexParamName.c_str()));
            if (DynMat->GetTextureParameterValue(ParamInfo, DummyTex))
            {
                UE_LOG(LogTemp, Warning, TEXT("[CreateMaterialDynamic] Parameter '%s' exists in material '%s'."),
                    *ParamInfo.Name.ToString(), *Material->GetName());
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("[CreateMaterialDynamic] Parameter '%s' NOT found in material '%s'."),
                    *ParamInfo.Name.ToString(), *Material->GetName());
            }
        }
    }

    return DynMat;
}


//----------------------------------------------------------------------------------|  |--//
UTexture2D* CreateDynamicTexture2D(AActor* Owner, int32 Width, int32 Height)
{
    UTexture2D* DynamicTexture = nullptr;
	DynamicTexture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);
    DynamicTexture->Filter = TF_Nearest;
    DynamicTexture->SRGB = false;
    DynamicTexture->UpdateResource();
	
    //DynamicMaterial->SetTextureParameterValue(FName("Texture"), DynamicTexture);
	UE_LOG(LogTemp, Warning, TEXT("[CreateDynamicTexture2D] Done !!") ) ;
	return DynamicTexture; 
}


//----------------------------------------------------------------------------------|  |--//
void UpdateDynamicTexture2D(UTexture2D* DynamicTexture, TArrayView<float> Data) 
{
    FLinearColor ColorMin = FLinearColor::White;
    FLinearColor ColorMax = FLinearColor::Black;

    float Min = TNumericLimits<float>::Max();
    float Max = TNumericLimits<float>::Lowest();
    for (float Val : Data)
    {
        Min = FMath::Min(Min, Val);
        Max = FMath::Max(Max, Val);
    }

    uint8* MipData = static_cast<uint8*>(DynamicTexture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE));
    for (int32 i = 0; i < Data.Num(); i++)
    {
        float T = FMath::GetMappedRangeValueClamped(FVector2D(Max, Min), FVector2D(0.f, 1.f), Data[i]);
        FColor Color = FMath::Lerp(ColorMin, ColorMax, T).ToFColor(false);

        MipData[i * 4 + 0] = Color.B;
        MipData[i * 4 + 1] = Color.G;
        MipData[i * 4 + 2] = Color.R;
        MipData[i * 4 + 3] = Color.A;
    }

    DynamicTexture->GetPlatformData()->Mips[0].BulkData.Unlock();
    DynamicTexture->UpdateResource();

	UE_LOG(LogTemp, Warning, TEXT("[UpdateDynamicTexture] Done !!") ) ;
}


//----------------------------------------------------------------------------------|  |--//
void SaveDynamicTexture2D(UTexture2D* Texture, std::string fname)
{
    //if (!Texture || !Texture->GetPlatformData() || Texture->GetPlatformData()->Mips.Num() == 0) return ;
    FTexture2DMipMap& Mip = Texture->GetPlatformData()->Mips[0];
    const void* Data = Mip.BulkData.LockReadOnly();

    const int32 Width = Mip.SizeX;
    const int32 Height = Mip.SizeY;
    const int32 BytesPerPixel = 4;

    TArray<uint8> RawData;
    RawData.AddUninitialized(Width * Height * BytesPerPixel);
    FMemory::Memcpy(RawData.GetData(), Data, RawData.Num());

    Mip.BulkData.Unlock();

    // Use PNG image wrapper
    IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
    TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

    FString FilePath = FPaths::ProjectSavedDir() / UTF8_TO_TCHAR(fname.c_str()); 
	if (ImageWrapper->SetRaw(RawData.GetData(), RawData.Num(), Width, Height, ERGBFormat::BGRA, 8))
    {
        const TArray64<uint8>& PNGData = ImageWrapper->GetCompressed(100);
        FFileHelper::SaveArrayToFile(PNGData, *FilePath);
    }

	UE_LOG(LogTemp, Warning, TEXT("[SaveDynamicTexture2D] '%s' Done !!"), *FilePath) ;
}



//----------------------------------------------------------------------------------|  |--//
//----------------------------------------------------------------------------------|  |--//
UProceduralMeshComponent* CreateProceduralBox(AActor* Owner, FVector BoxExtent = FVector(100, 100, 100))
{
    if (!Owner) return nullptr;

    // Create Procedural Mesh Component
    UProceduralMeshComponent* ProcMesh = NewObject<UProceduralMeshComponent>(Owner);
    ProcMesh->RegisterComponent();
    ProcMesh->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

    // Define vertices of a cube (BoxExtent is half-size)
    TArray<FVector> Vertices = {
        // Front face
        FVector(-BoxExtent.X, -BoxExtent.Y, -BoxExtent.Z),
        FVector(BoxExtent.X, -BoxExtent.Y, -BoxExtent.Z),
        FVector(BoxExtent.X, BoxExtent.Y, -BoxExtent.Z),
        FVector(-BoxExtent.X, BoxExtent.Y, -BoxExtent.Z),
        FVector(-BoxExtent.X, -BoxExtent.Y, BoxExtent.Z),
        FVector(BoxExtent.X, -BoxExtent.Y, BoxExtent.Z),
        FVector(BoxExtent.X, BoxExtent.Y, BoxExtent.Z),
        FVector(-BoxExtent.X, BoxExtent.Y, BoxExtent.Z)
    };

    // Cube triangles
    TArray<int32> Triangles = {
        // Bottom
        0,2,1, 0,3,2,
        // Top
        4,5,6, 4,6,7,
        // Front
        0,1,5, 0,5,4,
        // Back
        2,3,7, 2,7,6,
        // Left
        0,4,7, 0,7,3,
        // Right
        1,2,6, 1,6,5
    };

    // Normals (simple cube normals)
    TArray<FVector> Normals; Normals.Init(FVector::UpVector, Vertices.Num());

    // UVs (simple 0-1 mapping)
    TArray<FVector2D> UV0; UV0.Init(FVector2D(0,0), Vertices.Num());

    // Vertex Colors
    TArray<FLinearColor> VertexColors; VertexColors.Init(FLinearColor::White, Vertices.Num());

    // Tangents
    TArray<FProcMeshTangent> Tangents; Tangents.Init(FProcMeshTangent(1,0,0), Vertices.Num());

    // Create mesh section
    ProcMesh->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UV0, VertexColors, Tangents, true);

    // Enable collision (optional)
    ProcMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    return ProcMesh;
}


//----------------------------------------------------------------------------------|  |--//
float* VtiFile2Buffer(
	const std::string& fname, 
	vtkImageData*& vti_ptr, 
	int dimensions[3]
) 
{
	// Loading 
	if(vti_ptr)
	{ 
		vti_ptr->Delete();
		vti_ptr = nullptr; 
	} 

	vti_ptr = ReadVTIFile(fname); 
	CenterImageAtOrigin(vti_ptr); 

	//int dimensions[3];
	vti_ptr->GetDimensions(dimensions); 

	auto total = dimensions[0] * dimensions[1] * dimensions[2]; 
	UE_LOG(LogTemp, Warning, TEXT("[VtiFile2Buffer] dimensions : (%d,%d,%d) -> %d"), dimensions[0], dimensions[1], dimensions[2], total);

	double spacing[3];
	vti_ptr->GetSpacing(spacing);  
	UE_LOG(LogTemp, Warning, TEXT("[VtiFile2Buffer] spacing: x=%.2f, y=%.2f, z=%.2f"), spacing[0], spacing[1], spacing[2]);

	// Getting buffer  
	std::string key = "voxels";  

	vtkPointData* pd = vti_ptr->GetPointData(); 
  	vtkDataArray* array = pd->GetArray( key.c_str() );

	auto range = std::vector<double>(2, std::numeric_limits<double>::max()); 
	array->GetRange(range.data()); 
	UE_LOG(LogTemp, Warning, TEXT("[VtiFile2Buffer] range : (%f,%f)"), range[0], range[1]);


	int length = -1; 
	float* buffer = ArrayGet3<float>(pd, key, length); 
	UE_LOG(LogTemp, Warning, TEXT("[VtiFile2Buffer] length : %d "), length);

	return buffer; 
}


UVolumeTexture* CreateVolumeFromFloatBuffer(const int Dimensions[3], const float* Buffer)
{
    if (!Buffer) return nullptr;

    int32 Width  = Dimensions[0];
    int32 Height = Dimensions[1];
    int32 Depth  = Dimensions[2];

    // Create transient volume texture
    UVolumeTexture* VolumeTex = UVolumeTexture::CreateTransient(
        Width,
        Height,
        Depth,
        PF_R32_FLOAT, // Single float per voxel
        NAME_None
    );
    if (!VolumeTex)
    {
        return nullptr;
    }

    // Fill the volume texture using UpdateSourceFromFunction
    bool bSuccess = VolumeTex->UpdateSourceFromFunction(
        [=](int32 X, int32 Y, int32 Z, void* DestData)
        {
            int64 Index = Z * Width * Height + Y * Width + X;
            float* Dest = static_cast<float*>(DestData);
            *Dest = Buffer[Index];
        },
        Width,
        Height,
        Depth,
        TSF_R32F // Source format for single float
    );

    if (bSuccess)
    {
        VolumeTex->UpdateResource();
		UE_LOG(LogTemp, Warning, TEXT("[CreateVolumeFromFloatBuffer] Done !!") );
		return VolumeTex;
    }

	// PlatformData is obsolete.
    return nullptr;
}


//----------------------------------------------------------------------------------|  |--//
void PrintMinMax(const float* Buffer, int32 dimensions[3])
{
	int32 TotalSize = dimensions[0] * dimensions[1] * dimensions[2];

    if (!Buffer || TotalSize <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Buffer is null or empty"));
        return;
    }

    float MinValue = Buffer[0];
    float MaxValue = Buffer[0];

    for (int32 i = 1; i < TotalSize; ++i)
    {
        if (Buffer[i] < MinValue) MinValue = Buffer[i];
        if (Buffer[i] > MaxValue) MaxValue = Buffer[i];
    }

    UE_LOG(LogTemp, Warning, TEXT("[PrintMinMax] Buffer Min: %f, Max: %f"), MinValue, MaxValue);
}


void GenerateTestData(float*& Buffer, int32 Dimensions[3])
{
    // Define dimensions
    Dimensions[0] = 64; // Width
    Dimensions[1] = 64; // Height
    Dimensions[2] = 64; // Depth

    // Allocate buffer
    int64 TotalSize = static_cast<int64>(Dimensions[0]) * Dimensions[1] * Dimensions[2];
    Buffer = new float[TotalSize];

    // Fill buffer with test data
    for (int32 Z = 0; Z < Dimensions[2]; ++Z)
    {
        for (int32 Y = 0; Y < Dimensions[1]; ++Y)
        {
            for (int32 X = 0; X < Dimensions[0]; ++X)
            {
                int64 Index = Z * Dimensions[0] * Dimensions[1] + Y * Dimensions[0] + X;
                Buffer[Index] = static_cast<float>(Index) / (TotalSize - 1.0);  
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("[GenerateTestData] TotalSize : %d "), TotalSize);
}


void ReleaseTestData(float*& Buffer)
{
    if (Buffer)
    {
        delete[] Buffer;
        Buffer = nullptr;
    }
}


//----------------------------------------------------------------------------------|  |--//
//----------------------------------------------------------------------------------|  |--//
AVti2VolumeTexture::AVti2VolumeTexture()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SpicyRoot"));
/*
    // Create mesh component
    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CubeComponent"));
    Mesh->SetupAttachment(RootComponent);

    // Default cube mesh
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeMesh.Succeeded())
    {
        Mesh->SetStaticMesh(CubeMesh.Object);
    }
*/
	BaseMaterial = nullptr; 
	MeshComponent = nullptr; 
    VolumeTexture = nullptr;
    DynamicMaterial = nullptr;
/*
    static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("/Engine/BasicShapes/Plane"));
	UStaticMeshComponent* meshComponent = nullptr; 
    meshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaneComponent"));
	meshComponent->SetStaticMesh(PlaneMesh.Object);

	Decay = 0.05; 
*/
}


//----------------------------------------------------------------------------------|  |--//
void AVti2VolumeTexture::BeginPlay()
{
	Super::BeginPlay();

	UStaticMeshComponent* meshComponent = CreatePlaneComponent(this, FVector(2.0), FVector(-250.0f,0.0f,110.0f), FRotator(90.0f,.0f,0.0f)); 
	CreateDrawDebugBox(meshComponent, this->GetWorld()); 

	int pixels = 8; 
	int  widthLocal = 128 * pixels; 	
	int heightLocal =  64 * pixels; 
	TArray<float> dataLocal = CreateDataTest(widthLocal, heightLocal); 

	int tilesX = Deep.X; 
	int tilesY = Deep.Y; 
	int tilesTotal = tilesX * tilesY; 

	int  widthTotal =  widthLocal * tilesX; 
	int heightTotal = heightLocal * tilesY;
	UTexture2D* dynamicTexture = CreateDynamicTexture2D(this, widthTotal, heightTotal); 

    std::string materialPath = "/Game/SpicyTech/LevelC/M_RuntimeTexture.M_RuntimeTexture"; 
	UMaterialInstanceDynamic* dynamicMaterial = CreateMaterialDynamic(this, materialPath, "TextureParameter"); 
    dynamicMaterial->SetTextureParameterValue(FName("TextureParameter"), dynamicTexture);
	meshComponent->SetMaterial(0, dynamicMaterial); 

	if(Zed >= tilesX * tilesY) 
	{
		UE_LOG(LogTemp, Warning, TEXT("[GenerateVolumeTexture] %d > %d x %d !!"), Zed, tilesX, tilesY);
		return ; 
	} 

	TArray<float> dataTotal; 
	dataTotal.Init(0.0f, widthTotal * heightTotal);

	//DataTiles(dataTotal,            0, dataLocal, widthLocal, heightLocal, widthTotal, heightTotal); 
	//DataTiles(dataTotal, tilesTotal-1, dataLocal, widthLocal, heightLocal, widthTotal, heightTotal); 
	//DataTiles(dataTotal,          Zed, dataLocal, widthLocal, heightLocal, widthTotal, heightTotal); 

	for(int i=0; i < tilesTotal; i++) DataTiles(dataTotal, i, dataLocal, widthLocal, heightLocal, widthTotal, heightTotal);

	UpdateDynamicTexture2D(dynamicTexture, dataTotal);

	std::ostringstream oss;
	oss << "dynamicTextureTest" << std::setw(4) << std::setfill('0') << Zed << ".png";
	std::string fileName = oss.str(); 
	//SaveDynamicTexture2D(dynamicTexture, fileName); 

	MeshComponent = meshComponent; 

	//GenerateVolumeTexture(); // :/ 
}


//----------------------------------------------------------------------------------|  |--//
void AVti2VolumeTexture::GenerateVolumeTexture(UStaticMeshComponent* Mesh)
{
	// Mesh / BBbox 
    //SetActorLocation(FVector(300.0f, 0.0f, 100.0f));
    //SetActorRotation(FRotator(0.0f, 45.0f, 0.0f));	
	Mesh->SetRelativeLocation(FVector(300,0,100));
	Mesh->SetRelativeRotation(FRotator(0,45,0));

	FVector HalfSize = Mesh->GetStaticMesh()->GetBoundingBox().GetExtent(); // local unscaled
	FVector Scale = Mesh->GetComponentScale();
	HalfSize *= Scale; // apply component scale

	FVector Location = Mesh->GetComponentLocation();
	FQuat Rotation = Mesh->GetComponentQuat();
	FColor Color = FColor::Red;
	float Duration = 0.f;
	bool bPersistent = true;

	DrawDebugBox(GetWorld(), Location, HalfSize, Rotation, Color, bPersistent, Duration, 0, 2.0f);

    int32 Dimensions[3];
    float* Buffer = nullptr;

    // Generate test data
    GenerateTestData(Buffer, Dimensions);
	PrintMinMax(Buffer, Dimensions);


    // Create volume texture
    VolumeTexture = CreateVolumeFromFloatBuffer(Dimensions, Buffer);
	if(VolumeTexture) UE_LOG(LogTemp, Warning, TEXT("[GenerateVolumeTexture] 'VolumeTexture' OK ! "));

    ReleaseTestData(Buffer);

	// Set 'DynamicMaterial' 
	FString materialPath("/Game/SpicyTech/LevelC/M_VolumeTest.M_VolumeTest");
	UMaterialInterface* VolumeMat = LoadObject<UMaterialInterface>(nullptr, *materialPath);
	if(!VolumeMat) UE_LOG(LogTemp, Warning, TEXT("[GenerateVolumeTexture] 'VolumeMat' error ... "));

	// 'VolumeText' -> 'TextureSampleParameterVolume' node name in the material.
	DynamicMaterial = UMaterialInstanceDynamic::Create(VolumeMat, this);
	if(!DynamicMaterial) UE_LOG(LogTemp, Warning, TEXT("[GenerateVolumeTexture] 'DynamicMaterial' error ... "));

	if (DynamicMaterial)
	{
		UTexture* DummyTex = nullptr;
		FMaterialParameterInfo ParamInfo(FName("VolumeTex"));

		if ( DynamicMaterial->GetTextureParameterValue(ParamInfo, DummyTex) )
		{
			DynamicMaterial->SetTextureParameterValue(FName("VolumeTex"), VolumeTexture);
    		UE_LOG(LogTemp, Warning, TEXT("[GenerateVolumeTexture] 'VolumeTex' found in '%s' Texture "), DummyTex ? *DummyTex->GetName() : TEXT("??"));

        	if(Mesh) 
			{
				Mesh->SetMaterial(0, DynamicMaterial);
				UE_LOG(LogTemp, Warning, TEXT("[GenerateVolumeTexture] 'SetMaterial' Done !!"));	
			}
			else 
			{
				UE_LOG(LogTemp, Warning, TEXT("[GenerateVolumeTexture] 'SetMaterial' error ... "));	
			} 
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[GenerateVolumeTexture] 'VolumeTex' does not exist!"));
		}
	}

    UE_LOG(LogTemp, Warning, TEXT("[GenerateVolumeTexture] Done!"));
} 


//----------------------------------------------------------------------------------|  |--//
void AVti2VolumeTexture::GenerateVolumeTexture2()
{
	// 1) 
	std::string fname = "F:/Download/DecafPV560/Data/rsnaCervicalSpineFractureDetection.vti"; 
	vtkImageData* domain = nullptr; 

	int dimensions[3] = {}; 
	float* buffer = nullptr; 

	UVolumeTexture* volumeTexture = CreateVolumeFromFloatBuffer(dimensions, buffer);

	if (!volumeTexture) {
    	UE_LOG(LogTemp, Error, TEXT("VolumeTexture is null or has no resource!"));
	}

	// 2) 
	FString materialPath("/Game/SpicyTech/LevelC/M_VolumeDebug.M_VolumeDebug");
	UMaterialInterface* VolumeMat = LoadObject<UMaterialInterface>(nullptr, *materialPath);

	// VolumeTexture -> 'TextureSampleParameterVolume' node name in the material.
	UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(VolumeMat, this);
	MID->SetTextureParameterValue(FName("VolumeTexture"), volumeTexture);


    FVector HalfSize(100, 100, 100); // default cube size
    UProceduralMeshComponent* VolumeMesh = CreateProceduralBox(this, HalfSize);	

	if (VolumeMesh)
    {
        VolumeMesh->SetMaterial(0, MID);
		UE_LOG(LogTemp, Warning, TEXT("[AVti2VolumeTexture] 'SetMaterial' Done! "));
    }

    if (!RootComponent)
    {
        RootComponent = NewObject<USceneComponent>(this);
        RootComponent->RegisterComponent();
    }

    VolumeMesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

    // --- 5) Assign Material ---
    VolumeMesh->SetMaterial(0, MID);
    VolumeMesh->SetVisibility(true);
    VolumeMesh->SetCastShadow(false);

    // --- 6) Scale to VTI dimensions ---
    FVector VtiScale(dimensions[0], dimensions[1], dimensions[2]);
    //VolumeMesh->SetWorldScale3D(VtiScale / 100.0f); // adjust scale factor if needed

    UE_LOG(LogTemp, Warning, TEXT("[AVti2VolumeTexture] Volume mesh visible with scale: %d x %d x %d"), 
        dimensions[0], dimensions[1], dimensions[2]);


FVector Location = VolumeMesh->GetComponentLocation();
FVector Extent = FVector(100,100,100);
FColor Color = FColor::Red;
float Duration = 0; // 0 = one frame, >0 = seconds
bool bPersistent = true;

DrawDebugBox(GetWorld(), Location, HalfSize, Color, bPersistent, Duration, 0, 2.0f);		

	UE_LOG(LogTemp, Warning, TEXT("[AVti2VolumeTexture] Done! "));
	if(domain) domain->Delete();
}


//----------------------------------------------------------------------------------|  |--//
void AVti2VolumeTexture::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

//----------------------------------------------------------------------------------|  |--//
//----------------------------------------------------------------------------------|  |--//
UVolumeTexture* CreateVolumeFromTexture2D(UTexture2D* SourceTexture, int32 Depth)
{
    if (!SourceTexture) return nullptr;

    FTexture2DMipMap& Mip = SourceTexture->GetPlatformData()->Mips[0];
    FByteBulkData* RawImageData = &Mip.BulkData;
    void* Data = RawImageData->Lock(LOCK_READ_ONLY);

    int32 Width  = Mip.SizeX;
    int32 Height = Mip.SizeY;
    int32 PixelSize = 4; // BGRA8

    // Create volume texture
    UVolumeTexture* VolumeTex = UVolumeTexture::CreateTransient(
        Width,
        Height,
        Depth,
        PF_B8G8R8A8,
        NAME_None
    );
    if (!VolumeTex)
    {
        RawImageData->Unlock();
        return nullptr;
    }

    // Fill volume texture data
    bool bSuccess = VolumeTex->UpdateSourceFromFunction(
        [=](int32 X, int32 Y, int32 Z, void* DestData)
        {
            int64 SliceSize = Width * Height * PixelSize;
            const uint8* Src = static_cast<const uint8*>(Data) + (Y * Width + X) * PixelSize;
            uint8* Dest = static_cast<uint8*>(DestData);

            // Copy slice pixel
            FMemory::Memcpy(Dest, Src, PixelSize);
        },
        Width,
        Height,
        Depth,
        TSF_BGRA8
    );

    RawImageData->Unlock();

    if (bSuccess)
    {
        VolumeTex->UpdateResource();
        return VolumeTex;
    }

    return nullptr;
}



//----------------------------------------------------------------------------------|  |--//