#include "PixelTextureDisplay.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/Material.h"
#include "Engine/Texture2D.h"
#include "UObject/ConstructorHelpers.h"
#include "Math/UnrealMathUtility.h"

// SaveTextureToPNG
#include "ImageUtils.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Modules/ModuleManager.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"


bool SaveTextureToPNG(UTexture2D* Texture, const FString& FilePath)
{
    if (!Texture || !Texture->GetPlatformData() || Texture->GetPlatformData()->Mips.Num() == 0)
    {
        return false;
    }

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

    if (ImageWrapper->SetRaw(RawData.GetData(), RawData.Num(), Width, Height, ERGBFormat::BGRA, 8))
    {
        const TArray64<uint8>& PNGData = ImageWrapper->GetCompressed(100);

        return FFileHelper::SaveArrayToFile(PNGData, *FilePath);
    }

    return false;
}




void APixelTextureDisplay::BeginPlay()
{
    Super::BeginPlay();
}


void APixelTextureDisplay::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    //UpdateActorTransformIfChanged(); 
}


APixelTextureDisplay::APixelTextureDisplay()
{
    PrimaryActorTick.bCanEverTick = false;

    PlaneComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Plane"));
    RootComponent = PlaneComponent;

    static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("/Engine/BasicShapes/Plane"));
    UE_LOG(LogTemp, Warning, TEXT("[APixelTextureDisplay] PlaneMesh : %d "), PlaneMesh.Succeeded() );

    PlaneComponent->SetStaticMesh(PlaneMesh.Object);

    //static ConstructorHelpers::FObjectFinder<UMaterial> Mat(TEXT("/Engine/EngineMaterials/DefaultMaterial")); // Basic Material!!
    //
    // Content Browser -> Material (M_RuntimeTexture) 
    //                 -> Add a 'TextureSampleParameter2D' -> Set parameter name "Texture"
    //                 -> Connect it to Base Color
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> Mat(TEXT("/Game/M_RuntimeTexture"));
    BaseMaterial = Mat.Object; 
}


void APixelTextureDisplay::Init(int32 InWidth, int32 InHeight)
{
    Width = InWidth;
    Height = InHeight;

    DynamicTexture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);
    DynamicTexture->Filter = TF_Nearest;
    DynamicTexture->SRGB = false;
    DynamicTexture->UpdateResource();

    DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
    DynamicMaterial->SetTextureParameterValue(FName("Texture"), DynamicTexture);
    PlaneComponent->SetMaterial(0, DynamicMaterial);

    UpdateActorTransformIfChanged(); 
}


void APixelTextureDisplay::CreateOrUpdateTexture(int32 NewWidth, int32 NewHeight)
{
    if (!DynamicTexture || NewWidth != Width || NewHeight != Height)
    {
        Width = NewWidth;
        Height = NewHeight;

        DynamicTexture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);
        DynamicTexture->Filter = TF_Nearest;
        DynamicTexture->SRGB = false;
        DynamicTexture->UpdateResource();

        if (DynamicMaterial)
        {
            DynamicMaterial->SetTextureParameterValue(FName("Texture"), DynamicTexture);
        }
    }
}


void APixelTextureDisplay::Apply(TArrayView<float> Data, float ScaleFactor) //void APixelTextureDisplay::Apply(TArray<float> Data, float ScaleFactor)
{
    if (!DynamicTexture || Data.Num() != Width * Height) return;

    if (FMath::Abs(ScaleFactor - LastScaleFactor) > KINDA_SMALL_NUMBER)
    {
        PlaneScaleUpdate(Width, Height, ScaleFactor);
    }

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
}


void APixelTextureDisplay::Save()
{
    FString OutputPath = FPaths::ProjectSavedDir() / TEXT("MyTexture.png");
    SaveTextureToPNG(DynamicTexture, OutputPath);

    UE_LOG(LogTemp, Warning, TEXT("Texture saved to: %s"), *OutputPath);
}


void APixelTextureDisplay::PlaneScaleUpdate(int32 W, int32 H, float ScaleFactor)
{
    if (PlaneComponent)
    {
        FVector Scale((float)W * ScaleFactor * 0.01f, (float)H * ScaleFactor * 0.01f, 1.f);
        PlaneComponent->SetWorldScale3D(Scale);
        LastScaleFactor = ScaleFactor;
    }
}




void APixelTextureDisplay::UpdateActorTransformIfChanged()
{
    if (!PlaneComponent) return;

    if (!ActorPosition.Equals(LastPosition, 0.01f) || !ActorRotation.Equals(LastRotation, 0.01f))
    {
        PlaneComponent->SetRelativeLocation(ActorPosition);
        PlaneComponent->SetRelativeRotation(ActorRotation);

        LastPosition = ActorPosition;
        LastRotation = ActorRotation;
    }    

    UE_LOG(LogTemp, Log, TEXT("Actor rotation: Pitch=%f Yaw=%f Roll=%f"), ActorRotation.Pitch, ActorRotation.Yaw, ActorRotation.Roll);
}

