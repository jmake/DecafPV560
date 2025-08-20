#pragma once

#include "../Sources/vtktools4.hpp"

#include "Engine/StaticMesh.h"
#include "Materials/Material.h"
#include "UObject/ConstructorHelpers.h"
#include "Math/UnrealMathUtility.h"

// SaveTextureToPNG
#include "ImageUtils.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Modules/ModuleManager.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"

// 
#include "Engine/VolumeTexture.h"
#include "Engine/Texture2D.h"
#include "Engine/Texture.h"

#include "ProceduralMeshComponent.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"

// 
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/VolumeTexture.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/StaticMeshComponent.h"

//----------------------------------------------------------------------------------|  |--//
//----------------------------------------------------------------------------------|  |--//
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Vti2VolumeTexture.generated.h" // Last one!!


//----------------------------------------------------------------------------------|  |--//
//----------------------------------------------------------------------------------|  |--//
UCLASS()
class MYPROJECT5_API AVti2VolumeTexture : public AActor
{
	GENERATED_BODY()
	
public:	
	AVti2VolumeTexture();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

void GenerateVolumeTexture2(); 
void CreateMeshComponent1(UTexture2D* dynamicTexture); 
void CreateMeshComponent2(UVolumeTexture* volumeTexture); 


public : 

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "SpicyTools")
	int maxDeep = -1; 

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "SpicyTools")
	FVector2D Deep = FVector2D(1,1);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "SpicyTools")
	FVector2D Dimensions = FVector2D(1,1);

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "SpicyTools")
	UTexture2D* DynamicTexture1; 

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "SpicyTools")
    UVolumeTexture* VolumeTexture2;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "SpicyTools|MeshComponents")
    UMaterialInstanceDynamic* DynamicMaterial1;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "SpicyTools|MeshComponents")
    UMaterialInstanceDynamic* DynamicMaterial2;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "SpicyTools|MeshComponents")
    UStaticMeshComponent* MeshComponent1;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "SpicyTools|MeshComponents")
    UStaticMeshComponent* MeshComponent2;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "SpicyTools|MeshComponents")
    UStaticMeshComponent* MeshComponent3;

};
//----------------------------------------------------------------------------------|  |--//
//----------------------------------------------------------------------------------|  |--//