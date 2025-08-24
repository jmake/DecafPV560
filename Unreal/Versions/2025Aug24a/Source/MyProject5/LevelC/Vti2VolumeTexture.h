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


#if WITH_EDITOR
virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    const FName PropertyName1 = PropertyChangedEvent.GetPropertyName();
	const FName PropertyName2 = PropertyChangedEvent.MemberProperty->GetFName(); 

	UE_LOG(LogTemp, Warning, TEXT("[PostEditChangeProperty] GetPropertyName : '%s' MemberProperty: '%s' "), 
	*PropertyName1.ToString(), 
	*PropertyName2.ToString() 
	);

	if(
		PropertyName2 == "Parameters" || 
		PropertyName2 == "maxDeep" || 
		PropertyName2 == "Dimensions" || 
		PropertyName2 == "Deep" || 
		PropertyName2 == "ColorMin" || 
		PropertyName2 == "ColorMax" 
	) 
		UpdateDymanicTexture();
}
#endif

void CreateVti2(); 

void CleanDymanicTexture(); 
void UpdateDymanicTexture(); 


public : 

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "SpicyTools|DymanicTexture")
	FVector Parameters = FVector::ZeroVector; 

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "SpicyTools|DymanicTexture")
	FVector2D Dimensions = FVector2D(1,1);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "SpicyTools|DymanicTexture")
	FVector2D Deep = FVector2D(1,1);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "SpicyTools|DymanicTexture")
	FLinearColor ColorMin = FLinearColor::Blue ;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "SpicyTools|DymanicTexture")
	FLinearColor ColorMax = FLinearColor::Green ;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "SpicyTools|DymanicTexture")
    UVolumeTexture* VolumeTexture2;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "SpicyTools|DymanicTexture")
	int maxDeep = -1; 

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "SpicyTools|MeshComponents")
	UTexture2D* DynamicTexture1; 

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