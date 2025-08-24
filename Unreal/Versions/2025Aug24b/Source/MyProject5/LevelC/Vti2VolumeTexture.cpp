#include "Vti2VolumeTexture.h"


//----------------------------------------------------------------------------------|  |--//
void SaveTArrayUint8(
						TArray<uint8> RawData, 
						int Width, 
						int Height,
						std::string fname
					)
{
	if(RawData.Num() != Width * Height * 4) 
	{
		UE_LOG(LogTemp, Warning, TEXT("[SaveTArrayUint8] Error !!") ) ;
		return ; 
	} 

    FString FilePath = FPaths::ProjectSavedDir() / UTF8_TO_TCHAR(fname.c_str()); 

    // Use PNG image wrapper
    IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
    TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

	if (ImageWrapper->SetRaw(RawData.GetData(), RawData.Num(), Width, Height, ERGBFormat::BGRA, 8))
    {
        const TArray64<uint8>& PNGData = ImageWrapper->GetCompressed(100);
        FFileHelper::SaveArrayToFile(PNGData, *FilePath);
    }

	UE_LOG(LogTemp, Warning, TEXT("[SaveTArrayUint8] '%s' Done !!"), *FilePath) ;
}


//----------------------------------------------------------------------------------|  |--//
void CreateVolumeTextureFromFloatData2(
	UVolumeTexture*& VolumeTex, 
	int32 Width, 
	int32 Height, 
	int32 tilesX, 
	int32 tilesY,
	FLinearColor ColorMin, 
	FLinearColor ColorMax,
	TArrayView<float> Data, 
	bool reorder=false 
)
{
	int32 Depth = tilesX * tilesY; 

    if (Data.Num() != Width * Height * Depth)  return ;

    // Step 1: find global min/max
    float Min = TNumericLimits<float>::Max();
    float Max = TNumericLimits<float>::Lowest();
    for (float Val : Data)
    {
        Min = FMath::Min(Min, Val);
        Max = FMath::Max(Max, Val);
    }

    // Step 2: create transient volume texture
	if (VolumeTex)
	{
		VolumeTex->Source.Init(0, 0, 0, 0, TSF_BGRA8);
		VolumeTex->UpdateResource();
	}	
	else
	{
		VolumeTex = UVolumeTexture::CreateTransient(Width, Height, Depth, PF_B8G8R8A8, NAME_None);
	}

	int32 PixelSize = 4; // PF_B8G8R8A8 
	TArray<uint8> VolumeBuffer;
	VolumeBuffer.Init(255, Width * Height * Depth * PixelSize); // 255 -> Black 

    // Step 3: lambda writes directly from float Data
	auto func = [=, &VolumeBuffer, &Data](int32 X, int32 Y, int32 Z, void* DestData)
    {
        uint8* Dest = static_cast<uint8*>(DestData);

		int32 Index = (Z * Width * Height) + (Y * Width + X);
		if(Index >= 0 && Index < Data.Num() )
		{
			float Normalized = FMath::GetMappedRangeValueClamped(FVector2D(Min, Max), FVector2D(0.f, 1.f), Data[Index]);
			FColor Color = FMath::Lerp(ColorMin, ColorMax, Normalized).ToFColor(false);

			Dest[0] = Color.B;
			Dest[1] = Color.G;
			Dest[2] = Color.R;
			Dest[3] = Color.A;

			VolumeBuffer[Index * PixelSize + 0] = Color.B; 
			VolumeBuffer[Index * PixelSize + 1] = Color.G; 
			VolumeBuffer[Index * PixelSize + 2] = Color.R; 
			VolumeBuffer[Index * PixelSize + 3] = Color.A; 
		}
	};

    // Step 4: update volume texture
    VolumeTex->UpdateSourceFromFunction(func, Width, Height, Depth, TSF_BGRA8);
	VolumeTex->UpdateResource();

	// Step 5: Save 
	int  widthTotal =  Width * tilesX; 
	int heightTotal = Height * tilesY; 	

	std::ostringstream oss;
	oss << std::setw(3) << std::setfill('0') <<"texture_W"<< Width <<"x"<< tilesX <<"_H"<< Height <<"x"<< tilesY;   
	std::string dynamicTextureName = oss.str(); 

	SaveTArrayUint8(VolumeBuffer, widthTotal, heightTotal, dynamicTextureName + "_B.png"); 

    UE_LOG(LogTemp, Warning, TEXT("[CreateVolumeTextureFromFloatData] Volume texture Created !!"));
}


//----------------------------------------------------------------------------------|  |--//
UVolumeTexture* CreateVolumeTextureFromBuffer1(int32 Width, int32 Height, int32 Depth, const TArray<uint8>& VolumeBuffer) 
{
	if (VolumeBuffer.Num() != Width * Height * Depth * 4) return nullptr;

	// 'PF_B8G8R8A8' or 'PF_R8G8B8A8' ?? // CreateDynamicTexture2D -> PF_B8G8R8A8 
	UVolumeTexture* VolumeTex = UVolumeTexture::CreateTransient(Width, Height, Depth, PF_R8G8B8A8, NAME_None); // < Only here!!

	if (!VolumeTex) return nullptr;

    VolumeTex->UpdateSourceFromFunction(
        [=, &VolumeBuffer](int32 X, int32 Y, int32 Z, void* DestData)
        {
            uint8* Dest = static_cast<uint8*>(DestData);
            int32 SliceIndex = Z * Width * Height;
            int32 PixelIndex = (Y * Width + X) ;
            for (int32 i = 0; i < 4; ++i)
            {
                Dest[i] = VolumeBuffer[(SliceIndex + PixelIndex) * 4 + i];
            }
        },
        Width, Height, Depth, TSF_BGRA8
    );

    VolumeTex->UpdateResource();
    UE_LOG(LogTemp, Warning, TEXT("[CreateVolumeTextureFromBuffer] Volume texture created!"));
    return VolumeTex;
}


UVolumeTexture* CreateVolumeTextureFromBuffer2(int32 Width, int32 Height, int32 Depth, const TArray<uint8>& VolumeBuffer) 
{
	if (VolumeBuffer.Num() != Width * Height * Depth * 4) return nullptr;

	UVolumeTexture* VolumeTex = UVolumeTexture::CreateTransient(Width, Height, Depth, PF_B8G8R8A8, NAME_None); // < CreateDynamicTexture2D 

	if (!VolumeTex) return nullptr;


	auto func = [=, &VolumeBuffer](int32 X, int32 Y, int32 Z, void* DestData)
	{
		uint8* Dest = static_cast<uint8*>(DestData);

		int32 PixelSize = 4;                   // PF_B8G8R8A8 
		int64 RowSize   = PixelSize * Width  ; // Bytes in one row
		int64 SliceSize =   RowSize * Height ; // Bytes in one slice (2D layer)

		int32 SliceIndex = Z * SliceSize;               // Z * Width * Height * 4;
		int32 PixelIndex = Y * RowSize + X * PixelSize; // (Y * Width + X) * 4 -> (Y * Width + X) * PixelSize -> Y * Width * PixelSize + X * PixelSize
		for (int32 i = 0; i < PixelSize; ++i) 
		{
			int32 Index = SliceIndex + PixelIndex + i;
			Dest[i] = VolumeBuffer[Index];
		}
	};
	
    VolumeTex->UpdateSourceFromFunction(func, Width, Height, Depth, TSF_BGRA8);
    VolumeTex->UpdateResource();

    UE_LOG(LogTemp, Warning, TEXT("[CreateVolumeTextureFromBuffer] Volume texture created!"));
    return VolumeTex;
}


//----------------------------------------------------------------------------------|  |--//
UVolumeTexture* CreateVolumeTextureFromFloatData1(int32 Width, int32 Height, int32 Depth, TArrayView<float> Data)
{
    if (Data.Num() != Width * Height * Depth) return nullptr;

    // Step 1: find global min/max
    float Min = TNumericLimits<float>::Max();
    float Max = TNumericLimits<float>::Lowest();
    for (float Val : Data)
    {
        Min = FMath::Min(Min, Val);
        Max = FMath::Max(Max, Val);
    }

    // Step 3: convert float -> Pixel
    FLinearColor ColorMin = FLinearColor::Red;  // or any mapping you want
    FLinearColor ColorMax = FLinearColor::Blue;

    // Step 2: allocate uint8 buffer for Pixel
int32 PixelSize = 4; // PF_B8G8R8A8 
TArray<uint8> VolumeBuffer;
//VolumeBuffer.Init(0, Width * Height * Depth * PixelSize); // 0 -> White?
VolumeBuffer.Init(255, Width * Height * Depth * PixelSize); // 0 -> Black?

    for (int32 z = 0; z < Depth; ++z)
    {
        for (int32 y = 0; y < Height; ++y)
        {
            for (int32 x = 0; x < Width; ++x)
            {
                int32 Index = (z * Width * Height) + (y * Width + x);

                float Normalized = FMath::GetMappedRangeValueClamped(FVector2D(Min, Max), FVector2D(0.f, 1.f), Data[Index]);
                FColor Color = FMath::Lerp(ColorMin, ColorMax, Normalized).ToFColor(false);

                int32 BufferIndex = Index * PixelSize;
                VolumeBuffer[BufferIndex + 0] = Color.B; // 255
                VolumeBuffer[BufferIndex + 1] = Color.G; // 0 
                VolumeBuffer[BufferIndex + 2] = Color.R; // 0 
                VolumeBuffer[BufferIndex + 3] = Color.A; // 255
            }
        }
    }

// Step 4: create volume texture from uint8 buffer
//return CreateVolumeTextureFromBuffer1(Width, Height, Depth, VolumeBuffer);
return CreateVolumeTextureFromBuffer2(Width, Height, Depth, VolumeBuffer);
}



//----------------------------------------------------------------------------------|  |--//
void DataTiles1(
	TArray<float>& dataTotal,  
	int z, 
	TArray<float> dataLocal, 
	int widthLocal, 
	int heightLocal, 
	int widthTotal, 
	int heightTotal
) 
{
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

			dataTotal[indexG] = var; // or z for debug
		}
	}

	//return dataTotal;
}


//----------------------------------------------------------------------------------|  |--//
void DataTiles2( // depth-major linear layout
    TArray<float>& dataTotal,
    int z,
    TArray<float> dataLocal,
    int widthLocal,
    int heightLocal,
    int widthTotal,
    int heightTotal
)
{
    // Tiles per row (fits exactly because of multiples)
    int tilesPerRow = widthTotal / widthLocal;

    // Tile grid position
    int tileX = z % tilesPerRow;
    int tileY = z / tilesPerRow;

    // Precompute per-slice size
    int sliceSize = widthLocal * heightLocal;

    for (int yL = 0; yL < heightLocal; yL++)
    {
        for (int xL = 0; xL < widthLocal; xL++)
        {
            int indexL = yL * widthLocal + xL;
            float var = dataLocal[indexL];

            // Flip Y to match the desired orientation (avoid later reordering)
            int flippedY = (heightLocal - 1) - yL;

            // Final index in 3D-ordered buffer
            int indexG = z * sliceSize + flippedY * widthLocal + xL;

            dataTotal[indexG] = var; // or z for debug
        }
    }

    //return dataTotal;
}



//----------------------------------------------------------------------------------|  |--//
void CreateDataTest2(TArray<float>& data, int width, int height)
{
	data.Empty();  
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
}

//----------------------------------------------------------------------------------|  |--//
void CreateDataCircle3(TArray<float>& data, int width, int height, int radius)
{
    data.Empty();
    data.Init(0.0f, width * height);

    int centerX = width / 2;
    int centerY = height / 2;

    // Clamp radius between 0 and max(width, height)
    radius = FMath::Clamp(radius, 1.0, 0.5 * FMath::Min(width, height));

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int dx = x - centerX;
            int dy = y - centerY;
            float distanceSq = dx * dx + dy * dy;

            if (distanceSq <= radius * radius)
            {
                data[y * width + x] = 1.0f; // inside circle
            }
            else
            {
                data[y * width + x] = 0.0f; // outside circle
            }
        }
    }
}
//----------------------------------------------------------------------------------|  |--//


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
UStaticMeshComponent* CreateMeshComponent(
	AActor* Owner, 
	std::string meshPath, 
	FVector Scale = FVector(1.0, 1.0, 1.0),
	FVector Location = FVector::ZeroVector, 
	FRotator Rotation = FRotator::ZeroRotator
) 
{
//	std::string meshPath = "/Engine/BasicShapes/Plane.Plane"; 
    FString PathFString(meshPath.c_str());
	UStaticMesh* staticMesh = LoadObject<UStaticMesh>(nullptr, *PathFString);

//	UStaticMesh* staticMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (!staticMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CreateMeshComponent] 'UStaticMesh' fails !!"));
		return nullptr; 
	}

    if (!Owner->GetRootComponent())
    {
		UE_LOG(LogTemp, Warning, TEXT("[CreateMeshComponent] 'GetRootComponent' fails !!"));
		return nullptr; 
    }
	
    // Create a new StaticMeshComponent at runtime
	UStaticMeshComponent* RuntimeMeshComponent = nullptr; 
	RuntimeMeshComponent = NewObject<UStaticMeshComponent>(Owner);
    RuntimeMeshComponent->SetStaticMesh( staticMesh );
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

	UE_LOG(LogTemp, Warning, TEXT("[CreateMeshComponent] Done !!") ) ;
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
	// PF_B8G8R8A8 = 32-bit pixel format. 4 channels 
	//               (B,G,R,A) × 1 byte each = 4 bytes per pixel
	//               (B,G,R,A) × 8 bits each = 32-bits pixel formtat 
	// Example: 
	// 			pixel (0,0) stored as (B=255, G=0, R=0, A=255) -> shows as opaque red in the engine.
	//
    UTexture2D* DynamicTexture = nullptr;
	DynamicTexture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8); // -> Width * Height * 4 bytes
    DynamicTexture->Filter = TF_Nearest;
    DynamicTexture->SRGB = false;
    DynamicTexture->UpdateResource();
	
    //DynamicMaterial->SetTextureParameterValue(FName("Texture"), DynamicTexture);
	UE_LOG(LogTemp, Warning, TEXT("[CreateDynamicTexture2D] Done !!") ) ;
	return DynamicTexture; 
}


//----------------------------------------------------------------------------------|  |--//
void UpdateDynamicTexture2D(
	UTexture2D* DynamicTexture, 
	TArrayView<float> Data, 
	FLinearColor ColorMin,
	FLinearColor ColorMax
) 
{
    float Min = TNumericLimits<float>::Max();
    float Max = TNumericLimits<float>::Lowest();
    for (float Val : Data)
    {
        Min = FMath::Min(Min, Val);
        Max = FMath::Max(Max, Val);
    }

//    FLinearColor ColorMin = FLinearColor::White;
//    FLinearColor ColorMax = FLinearColor::Black;

	uint8* MipData = static_cast<uint8*>(DynamicTexture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE)); // Width * Height * 4 
    for (int32 i = 0; i < Data.Num(); i++)
    {
        float T = FMath::GetMappedRangeValueClamped(FVector2D(Min, Max), FVector2D(0.f, 1.f), Data[i]);
        FColor Color = FMath::Lerp(ColorMin, ColorMax, T).ToFColor(false);

		// PF_B8G8R8A8 
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
TArray<float> VtiFile2Buffer2(
	const std::string& fname, 
	std::string key, 
	vtkImageData*& vti_ptr, 
	int dimensions[4], 
	double range[2], 
	int scale=1
)
{
	TArray<float> data; 

	// Loading 
	if(vti_ptr)
	{ 
		vti_ptr->Delete();
		vti_ptr = nullptr; 
	} 

	vti_ptr = ReadVTIFile(fname); 
	if(vti_ptr == nullptr)
	{
    	UE_LOG(LogTemp, Warning, TEXT("[CreateVti] 'ReadVTIFile' fails !!") );
		return data ; 
	}

	vti_ptr = ReduceImage(vti_ptr, scale, scale, scale); 
	if(vti_ptr == nullptr)
	{
    	UE_LOG(LogTemp, Warning, TEXT("[CreateVti] 'ReduceImage' fails !!") );
		return data ; 
	}
/*
	vti_ptr = ExtractVOIWithDefaults(vti_ptr); 
	if(vti_ptr == nullptr)
	{
    	UE_LOG(LogTemp, Warning, TEXT("[CreateVti] 'ExtractVOIWithDefaults' fails !!") );
		return data ; 
	}
*/

	// Verification 
	vtkPointData* pd = vti_ptr->GetPointData(); 
  	vtkDataArray* array = pd->GetArray( key.c_str() );

	if (array)
	{
		UE_LOG(LogTemp, Warning, TEXT("[VtiFile2Buffer]  '%s' found! "), *FString(key.c_str()));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[VtiFile2Buffer] '%s' no exists : "), *FString(key.c_str()));

		int numArrays = pd->GetNumberOfArrays();
		for (int i = 0; i < numArrays; ++i)
		{
			vtkDataArray* a = pd->GetArray(i);
			if (a && a->GetName())
			{
				FString arrayName = UTF8_TO_TCHAR(a->GetName());
				UE_LOG(LogTemp, Warning, TEXT("  - alternative '%s'"), *arrayName);
			}		
		}

		UE_LOG(LogTemp, Warning, TEXT("[VtiFile2Buffer] '!array' ") );
		return data; 
	}	

	// Data extraction 
	std::fill(range, range + 2, -1);
	std::fill(dimensions, dimensions + 4, -1);
	std::vector<float> buffer = IterateImageDataArray(vti_ptr, key, range, dimensions, dimensions[3]); 

	int components = dimensions[3]; 
	int total = dimensions[0] * dimensions[1] * dimensions[2] * components; 
	UE_LOG(LogTemp, Warning, TEXT("[VtiFile2Buffer] range : (%f,%f)"), range[0], range[1]);
	UE_LOG(LogTemp, Warning, TEXT("[VtiFile2Buffer] dimensions : (%d,%d,%d)x%d -> %d"), dimensions[0], dimensions[1], dimensions[2], components, total);

	if(components != 1) 
	{
		UE_LOG(LogTemp, Warning, TEXT("[VtiFile2Buffer] components : %d > 1 -> '%s' is not scalar!! "), components, *FString(key.c_str()) );
		return data; 		
	}

	// Data normalization 
	data.Init(-TNumericLimits<float>::Max(), total);
	for (int i = 0; i < data.Num(); i++)
	{ 
		data[i] = buffer[i];
		//data[i] = (buffer[i] - range[0]) / (range[1] - range[0]) ; 
		//data[i] = NormalizeToRange(buffer[i], 0.0f, 1.0f, range[0], range[1]);
	}
	UE_LOG(LogTemp, Warning, TEXT("[VtiFile2Buffer] data : (%d)"), data.Num());

    float Min = TNumericLimits<float>::Max();
    float Max = TNumericLimits<float>::Lowest();
    for (float Val : data)
    {
        Min = FMath::Min(Min, Val);
        Max = FMath::Max(Max, Val);
    }
    UE_LOG(LogTemp, Warning, TEXT("[VtiFile2Buffer] (min,max) = (%f,%f) "), Min, Max);

	return data; 
} 


//----------------------------------------------------------------------------------|  |--//
float* VtiFile2Buffer1(
	const std::string& fname, 
	std::string key, 
	vtkImageData*& vti_ptr, 
	int dimensions[3],
	double range[2],  
	int shrink
) 
{
	// Loading 
	if(vti_ptr)
	{ 
		vti_ptr->Delete();
		vti_ptr = nullptr; 
	} 

	vti_ptr = ReadVTIFile(fname); 
	vti_ptr = ReduceImage(vti_ptr, 2, 2, 2); 

	int dimensionsDummy[3] = {};
	std::vector<float> data; 
	//data = IterateImageDataArray(vti_ptr, key, dimensionsDummy); 
	UE_LOG(LogTemp, Warning, TEXT("[VtiFile2Buffer] 'IterateImageDataArray' data : %d elements "), data.size() );

	CenterImageAtOrigin(vti_ptr); 

	vti_ptr->GetDimensions(dimensions); 

	auto total = dimensions[0] * dimensions[1] * dimensions[2]; 
	UE_LOG(LogTemp, Warning, TEXT("[VtiFile2Buffer] dimensions : (%d,%d,%d) -> %d"), dimensions[0], dimensions[1], dimensions[2], total);

	double spacing[3];
	vti_ptr->GetSpacing(spacing);  
	UE_LOG(LogTemp, Warning, TEXT("[VtiFile2Buffer] spacing: x=%.2f, y=%.2f, z=%.2f"), spacing[0], spacing[1], spacing[2]);

	vtkPointData* pd = vti_ptr->GetPointData(); 
  	vtkDataArray* array = pd->GetArray( key.c_str() );

	if (array)
	{
		UE_LOG(LogTemp, Warning, TEXT("[VtiFile2Buffer] Array found: %s"), *FString(key.c_str()));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[VtiFile2Buffer] '!array' %s"), *FString(key.c_str()));

		int numArrays = pd->GetNumberOfArrays();
		for (int i = 0; i < numArrays; ++i)
		{
			vtkDataArray* a = pd->GetArray(i);
			if (a && a->GetName())
			{
				FString arrayName = UTF8_TO_TCHAR(a->GetName());
				UE_LOG(LogTemp, Warning, TEXT("  - %s"), *arrayName);
			}		
		}

		UE_LOG(LogTemp, Warning, TEXT("[VtiFile2Buffer] '!array' ") );
		return nullptr; 
	}	

	//auto range = std::vector<double>(2, std::numeric_limits<double>::max()); 
	//array->GetRange(range.data());
	array->GetRange( range );  
	UE_LOG(LogTemp, Warning, TEXT("[VtiFile2Buffer] range : (%f,%f)"), range[0], range[1]);

	int length = -1; 
	float* buffer = ArrayGet3<float>(pd, key, length); 
	UE_LOG(LogTemp, Warning, TEXT("[VtiFile2Buffer] length : %d "), length);


return data.data(); 

	//// Row-major order (X fastest, then Y, then Z) -> index = x + y * dimX + z * dimX * dimY
	//return buffer; 
/*
	//// X->Y->Z with flipped Y
	std::vector<float> tilesArray;
	ExportArrayToDataTiles(vti_ptr, key, tilesArray); 

	return tilesArray.data(); 
*/
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
void SaveDynamicTexture2D(
	AActor* owner,
	TArrayView<float> data, 
	int widthTotal, 
	int heightTotal, 
	std::string fname, 
	FLinearColor colorMin, 
	FLinearColor colorMax
)
{
	// Saving 
	std::ostringstream oss;
	oss << std::setw(3) << std::setfill('0') <<"texture_W"<< widthTotal <<"_H"<< heightTotal;   
	std::string dynamicTextureName = oss.str(); 

	UTexture2D* dt = CreateDynamicTexture2D(owner, widthTotal, heightTotal); 
//	CreateMeshComponent1(dt); 	
	UpdateDynamicTexture2D(dt, data, colorMin, colorMax);
	SaveDynamicTexture2D(dt, dynamicTextureName + "_D1.png"); 

	UE_LOG(LogTemp, Warning, TEXT("[SaveDynamicTexture2D] Done! "));
}


//----------------------------------------------------------------------------------|  |--//
void AVti2VolumeTexture::CreateMeshComponent1(UTexture2D* dynamicTexture)
{
	std::string planePath = "/Engine/BasicShapes/Plane.Plane"; 
	UStaticMeshComponent* planeComponent = CreateMeshComponent(this, planePath, FVector(1.0), FVector(-350.0f,-150.0f,110.0f), FRotator(90.0f,.0f,0.0f)); 
	CreateDrawDebugBox(planeComponent, this->GetWorld()); 

    std::string materialPath = "/Game/SpicyTech/LevelC/M_RuntimeTexture.M_RuntimeTexture"; 
	UMaterialInstanceDynamic* dynamicMaterial = CreateMaterialDynamic(this, materialPath, "TextureParameter"); 

	if(dynamicMaterial) planeComponent->SetMaterial(0, dynamicMaterial); 
	if(dynamicMaterial) dynamicMaterial->SetTextureParameterValue(FName("TextureParameter"), dynamicTexture);

	MeshComponent1 = planeComponent; 
	DynamicMaterial1 = dynamicMaterial; 
} 


void AVti2VolumeTexture::CreateMeshComponent2(UVolumeTexture* volumeTexture)
{
	std::string cubePath = "/Engine/BasicShapes/Cube.Cube"; 
	UStaticMeshComponent* cubeComponent = CreateMeshComponent(this, cubePath, FVector(1.0), FVector(-250.0f,100.0f,110.0f), FRotator(0.0f,.0f,0.0f)); 

	std::string materialPath2 = "/Game/SpicyTech/LevelC/M_VolumeTest.M_VolumeTest"; 
	UMaterialInstanceDynamic* dynamicMaterial2 = CreateMaterialDynamic(this, materialPath2, "VolumeTex"); 

	if(dynamicMaterial2) cubeComponent->SetMaterial(0, dynamicMaterial2); 
	if(dynamicMaterial2) dynamicMaterial2->SetTextureParameterValue(FName("VolumeTex"), volumeTexture);

	MeshComponent2 = cubeComponent; 
	DynamicMaterial2 = dynamicMaterial2; 
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
AVti2VolumeTexture::AVti2VolumeTexture()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SpicyRoot"));

	VolumeTexture2 = nullptr;
    DynamicMaterial1 = nullptr;

	MeshComponent1 = nullptr; 
	MeshComponent2 = nullptr; 
	MeshComponent3 = nullptr; 
}


//----------------------------------------------------------------------------------|  |--//
void AVti2VolumeTexture::BeginPlay()
{
	Super::BeginPlay();

	UpdateDymanicTexture(); 

	//CreateVti2(); // :)

	CreateVti3(); // :)

}


//----------------------------------------------------------------------------------|  |--//
void AVti2VolumeTexture::CleanDymanicTexture() 
{
	if (DynamicTexture1)
	{
		DynamicTexture1->ConditionalBeginDestroy();
		DynamicTexture1 = nullptr;
	}

	if (DynamicMaterial1)
	{
		DynamicMaterial1->ConditionalBeginDestroy();
		DynamicMaterial1 = nullptr;
	}

	if (DynamicMaterial2)
	{
		DynamicMaterial2->ConditionalBeginDestroy();
		DynamicMaterial2 = nullptr;
	}


	if (MeshComponent1)
	{
		MeshComponent1->DestroyComponent();
		MeshComponent1 = nullptr;
	}	

	if (MeshComponent2)
	{
		MeshComponent2->DestroyComponent();
		MeshComponent2 = nullptr;
	}

	if (MeshComponent3)
	{
		MeshComponent3->DestroyComponent();
		MeshComponent3 = nullptr;
	}	

	UE_LOG(LogTemp, Warning, TEXT("[UpdateDymanicTexture] Deep : %s"), *Deep.ToString());

	UE_LOG(LogTemp, Warning, TEXT("[UpdateDymanicTexture] maxDeep : %d"), maxDeep);

	UE_LOG(LogTemp, Warning, TEXT("[UpdateDymanicTexture] ColorMin : %s"), *ColorMin.ToString());

	UE_LOG(LogTemp, Warning, TEXT("[UpdateDymanicTexture] ColorMax : %s"), *ColorMax.ToString());

	UE_LOG(LogTemp, Warning, TEXT("[UpdateDymanicTexture] Dimensions : %s"), *Dimensions.ToString());
}


void AVti2VolumeTexture::UpdateDymanicTexture()
{
	CleanDymanicTexture(); 

	int minDim = 4; 
	int maxDim = 512; 

	int tilesX = Deep.X; 
	int tilesY = Deep.Y; 

	int  widthLocal = FMath::Clamp(Dimensions[0], minDim, maxDim);
	int heightLocal = FMath::Clamp(Dimensions[1], minDim, maxDim);

	int  widthTotal =  widthLocal * tilesX; 
	int heightTotal = heightLocal * tilesY;
	int   deepLocal = FMath::Clamp(tilesX * tilesY, minDim, maxDim);

	
	TArray<float> dataTotal1; dataTotal1.Init(0.0f, widthTotal * heightTotal);
	TArray<float> dataTotal2; dataTotal2.Init(0.0f, widthTotal * heightTotal);

	maxDeep = FMath::Clamp(maxDeep, -1, deepLocal); 
	int dummy = (maxDeep > 0)?(maxDeep):(deepLocal); 
	for(int i=0; i < dummy; i++)
	{ 
		TArray<float> dataLocal; 
		//CreateDataTest2(dataLocal, widthLocal, heightLocal); 
		CreateDataCircle3(dataLocal, widthLocal, heightLocal, Parameters.X); 

		DataTiles1(dataTotal1, i, dataLocal, widthLocal, heightLocal, widthTotal, heightTotal);
		DataTiles2(dataTotal2, i, dataLocal, widthLocal, heightLocal, widthTotal, heightTotal);
	}
}



//----------------------------------------------------------------------------------|  |--//
void AVti2VolumeTexture::CreateVti3()
{
//	CleanDymanicTexture(); 

	// -- Extacting -- 
	double range[2] = {}; 
	int dimensions[4] = {}; 
	vtkImageData* vti_ptr = nullptr; 

	// F:\z2025_1\ParaView600\bin\paraview.exe 
	std::string fname;
/*
	fname = "F:\\Download\\DecafPV560\\Data\\rsnaCervicalSpineFractureDetection.vti"; 
	//std::string key = "ImageScalars"; // "ImageScalars" scale > 1 | "voxels" scale = 1;
	TArray<float> Data = VtiFile2Buffer2(fname, "voxels", vti_ptr, dimensions, range, 1); 
	//TArray<float> Data = VtiFile2Buffer2(fname, "ImageScalars", vti_ptr, dimensions, range, 2); 
*/
	fname = "F:\\Download\\DecafPV560\\Data\\VTIs\\iguana.vti"; 
	fname = "F:\\Download\\DecafPV560\\Data\\VTIs\\iguana_VOI.vti"; 
	//fname = "F:\\Download\\DecafPV560\\Data\\VTIs\\mr_VOI.vti"; 	
	//fname = "F:\\Download\\DecafPV560\\Data\\VTIs\\t2_VOI.vti"; 	
	fname = "F:\\Download\\DecafPV560\\Data\\VTIs\\spine_VOI.vti"; 	

	TArray<float> Data = VtiFile2Buffer2(fname, "scalars", vti_ptr, dimensions, range, 1); 

	if(dimensions[3] != 1)
	{
    	UE_LOG(LogTemp, Warning, TEXT("[CreateVti] components : %d > 1 !!"), dimensions[3]);
		return ; 
	}

	// --- Atlas Layout Creation ---
	int  widthLocal = dimensions[0];  // slice width
	int heightLocal = dimensions[1];  // slice height
	int   deepLocal = dimensions[2];  // number of slices
	int   sizeLocal = widthLocal * heightLocal * deepLocal; 
	UE_LOG(LogTemp, Warning, TEXT("[CreateVti] dimensions : (%d,%d,%d) -> %d "), widthLocal, heightLocal, deepLocal, sizeLocal);

	// --- tile --- 
	int tilesX = static_cast<int>(ceil(sqrt(deepLocal)));
	int tilesY = static_cast<int>(ceil((float)deepLocal / tilesX));
	UE_LOG(LogTemp, Warning, TEXT("[CreateVti] tiles : (%d,%d)"), tilesX, tilesY );

	int  widthTotal = widthLocal  * tilesX;   // total atlas width
	int heightTotal = heightLocal * tilesY;   // total atlas height
	int   sizeTotal = widthTotal * heightTotal; 
	UE_LOG(LogTemp, Warning, TEXT("[CreateVti] totals : (%d,%d) -> %d "), widthTotal, heightTotal, sizeTotal);

	if (sizeLocal > sizeTotal) 
	{
		UE_LOG(LogTemp, Warning, TEXT("[CreateVti] sizeLocal > sizeTotal (%d > %d)"), sizeLocal, sizeTotal );
		return;
	}

	TArray<float> buffer; 
	buffer.Init(0.0, sizeTotal);
	for (int i=0; i < Data.Num(); i++) buffer[i] = Data[i]; 

	vtiDims.Clear(); 
	vtiDims.buffer = buffer; 
	vtiDims.widthLocal = widthLocal; 
	vtiDims.heightLocal = heightLocal;
	vtiDims.tilesX = tilesX;
	vtiDims.tilesY = tilesY;
	vtiDims.widthTotal = widthTotal;  
	vtiDims.heightTotal = heightTotal; 

	UpdateVti3(); 
} 


void AVti2VolumeTexture::UpdateVti3()
{
	CleanDymanicTexture(); 

	ApplyDymanicTexture(
		vtiDims.buffer, 
		vtiDims.widthLocal, 
		vtiDims.heightLocal, 
		vtiDims.tilesX, 
		vtiDims.tilesY, 
		vtiDims.widthTotal, 
		vtiDims.heightTotal
	); 

    UE_LOG(LogTemp, Warning, TEXT("[UpdateVti3]  %d x %d -> %d"), 
		vtiDims.widthTotal,
		vtiDims.heightTotal,  
		vtiDims.widthTotal * vtiDims.heightTotal );	
} 


//----------------------------------------------------------------------------------|  |--//
void AVti2VolumeTexture::CreateVti2()
{
	CleanDymanicTexture(); 

	// -- Extacting -- 
	double range[2] = {}; 
	int dimensions[4] = {}; 
	vtkImageData* vti_ptr = nullptr; 

	// F:\z2025_1\ParaView600\bin\paraview.exe 
	std::string fname;
/*
	fname = "F:\\Download\\DecafPV560\\Data\\rsnaCervicalSpineFractureDetection.vti"; 
	//std::string key = "ImageScalars"; // "ImageScalars" scale > 1 | "voxels" scale = 1;
	TArray<float> Data = VtiFile2Buffer2(fname, "voxels", vti_ptr, dimensions, range, 1); 
	//TArray<float> Data = VtiFile2Buffer2(fname, "ImageScalars", vti_ptr, dimensions, range, 2); 
*/

	fname = "F:\\Download\\DecafPV560\\Data\\VTIs\\iguana.vti"; 
	fname = "F:\\Download\\DecafPV560\\Data\\VTIs\\iguana_VOI.vti"; 
	TArray<float> Data = VtiFile2Buffer2(fname, "scalars", vti_ptr, dimensions, range, 1); 

	if(dimensions[3] != 1)
	{
    	UE_LOG(LogTemp, Warning, TEXT("[CreateVti] components : %d > 1 !!"), dimensions[3]);
		return ; 
	}

	// -- RGBA8 -- 
	int PixelSize = 4;
/*
	std::vector<uint8> rgbBuffer(Data.Num() * PixelSize); // Assuming RGBA8
    UE_LOG(LogTemp, Warning, TEXT("[CreateVti] atlasBuffer : %d ... "), Data.Num() );

    FLinearColor colorMin = FLinearColor::White;
    FLinearColor colorMax = FLinearColor::Black;
	
	for (int Index = 0; Index < Data.Num(); Index++) //for (float Val : Data)
	{
		float Val = Data[Index]; 
		float Normalized = FMath::GetMappedRangeValueClamped(FVector2D(range[0], range[1]), FVector2D(0.f, 1.f), Val);
		FColor Color = FMath::Lerp(colorMin, colorMax, Normalized).ToFColor(false);

		int32 BufferIndex = Index * PixelSize;
		rgbBuffer[BufferIndex + 0] = Color.B; // 255
		rgbBuffer[BufferIndex + 1] = Color.G; // 0 
		rgbBuffer[BufferIndex + 2] = Color.R; // 0 
		rgbBuffer[BufferIndex + 3] = Color.A; // 255
	}
*/
	// --- Atlas Layout Creation ---
	int  widthLocal = dimensions[0];  // slice width
	int heightLocal = dimensions[1];  // slice height
	int   deepLocal = dimensions[2];  // number of slices
	int   sizeLocal = widthLocal * heightLocal * deepLocal; 
	UE_LOG(LogTemp, Warning, TEXT("[CreateVti] dimensions : (%d,%d,%d) -> %d "), widthLocal, heightLocal, deepLocal, sizeLocal);

	// --- tile --- 
	int tilesX = static_cast<int>(ceil(sqrt(deepLocal)));
	int tilesY = static_cast<int>(ceil((float)deepLocal / tilesX));
	UE_LOG(LogTemp, Warning, TEXT("[CreateVti] tiles : (%d,%d)"), tilesX, tilesY );

	int  widthTotal = widthLocal  * tilesX;   // total atlas width
	int heightTotal = heightLocal * tilesY;   // total atlas height
	int   sizeTotal = widthTotal * heightTotal; 
	UE_LOG(LogTemp, Warning, TEXT("[CreateVti] totals : (%d,%d) -> %d "), widthTotal, heightTotal, sizeTotal);

	if (sizeLocal > sizeTotal) 
	{
		UE_LOG(LogTemp, Warning, TEXT("[CreateVti] sizeLocal > sizeTotal (%d > %d)"), sizeLocal, sizeTotal );
		return;
	}
/*
	// Loop through all slices and voxels
	TArray<uint8> atlasBuffer; 
	atlasBuffer.Init(-TNumericLimits<float>::Max(), sizeTotal * PixelSize);

	for (int z = 0; z < deepLocal; z++)
	{
		int tileX = z % tilesX;   // column of slice
		int tileY = z / tilesX;   // row of slice

		int offsetX = tileX * widthLocal;
		int offsetY = tileY * heightLocal;

		for (int y = 0; y < heightLocal; y++)
		{
			for (int x = 0; x < widthLocal; x++)
			{
				// Compute atlas coordinates
				int atlasX = offsetX + x;
				int atlasY = offsetY + y;

				// Compute 1D index in row-major order
				int index = atlasY * widthTotal + atlasX; 

				if (index >= 0 && index < Data.Num()) 
				{
float Val = Data[index]; // -> voxel value!
float Normalized = FMath::GetMappedRangeValueClamped(FVector2D(range[0], range[1]), FVector2D(0.f,1.f), Val);

//float Normalized = FMath::GetMappedRangeValueClamped(FVector2D(0.0, deepLocal), FVector2D(0.f,1.f), z);

//float Normalized = Data[index]; 

					FColor Color = FMath::Lerp(ColorMin, ColorMax, Normalized).ToFColor(false);
					atlasBuffer[index * PixelSize + 0] = Color.B;
					atlasBuffer[index * PixelSize + 1] = Color.G;
					atlasBuffer[index * PixelSize + 2] = Color.R;
					atlasBuffer[index * PixelSize + 3] = Color.A;
				}
				else
				{
					atlasBuffer[index * PixelSize + 0] = 255;
					atlasBuffer[index * PixelSize + 1] = 255;
					atlasBuffer[index * PixelSize + 2] = 255;
					atlasBuffer[index * PixelSize + 3] = 255;
				}
			}
		}
	} // deepLocal 


	//SaveDynamicTexture2D(this, atlasBuffer, widthTotal, heightTotal, "createVti", colorMin, colorMax); 	
	SaveTArrayUint8(atlasBuffer, widthTotal, heightTotal, "createVti.png"); 
*/
	TArray<float> buffer; 
	buffer.Init(0.0, sizeTotal);
	for (int i=0; i < Data.Num(); i++) buffer[i] = Data[i]; 
/*
	CreateVolumeTextureFromFloatData2(
		VolumeTexture2, 
		widthLocal, heightLocal, tilesX, tilesY,
		ColorMin, ColorMax,  
		buffer);	

	CreateMeshComponent2(VolumeTexture2); 
*/
	ApplyDymanicTexture(buffer, widthLocal, heightLocal, tilesX, tilesY, widthTotal, heightTotal); 

    UE_LOG(LogTemp, Warning, TEXT("[CreateVti] Done !!") );
}




void AVti2VolumeTexture::ApplyDymanicTexture(
	TArray<float> buffer, 
	int widthLocal, 
	int heightLocal, 
	int tilesX, 
	int tilesY, 
	int widthTotal, 
	int heightTotal
)
{
	CreateVolumeTextureFromFloatData2(
		VolumeTexture2, 
		widthLocal, heightLocal, 
		tilesX, tilesY,
		ColorMin, ColorMax,  
		buffer);

	CreateMeshComponent2(VolumeTexture2); 

	// Saving 
	std::ostringstream oss;
	oss << std::setw(3) << std::setfill('0') <<"texture_W"<< widthLocal <<"x"<< tilesX <<"_H"<< heightLocal <<"x"<< tilesY;   
	std::string dynamicTextureName = oss.str(); 

	UTexture2D* dt = CreateDynamicTexture2D(this, widthTotal, heightTotal); 
	CreateMeshComponent1(dt); 	
	UpdateDynamicTexture2D(dt, buffer, ColorMin, ColorMax);
	SaveDynamicTexture2D(dt, dynamicTextureName + "_C1.png"); 
}


//----------------------------------------------------------------------------------|  |--//
//----------------------------------------------------------------------------------|  |--//