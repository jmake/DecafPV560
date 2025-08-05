// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "PixelTextureDisplay.h"

#include "ProceduralMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"

#include "Materials/Material.h"
#include "Materials/MaterialInterface.h"
//#include "MaterialShared.h"  // <- defines MD_Surface
#include "MaterialDomain.h"     // Required for EMaterialDomain::MD_Surface


#include "SpicyTechDicom2.hpp"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyActor1.generated.h" // should always be the last



UCLASS()
class MYPROJECT5_API AMyActor1 : public AActor
{
	GENERATED_BODY()
	
public:	
	AMyActor1();

protected:
	virtual void BeginPlay() override;

private: 
	void IsoSourface(); 
	void CutterAxial(); 
	void CutterCoronal(){}; 
	void CutterSagittal(){}; 

    UPROPERTY()
    UProceduralMeshComponent* Mesh;

	UMaterialInstanceDynamic* DynMaterialInstance = nullptr;

	SpicyTech2::VtkTest *dicom = nullptr; 

public:	
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spicy")
float Perlin = 1.0; 

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spicy")
FVector ActorPosition = FVector::ZeroVector;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spicy")
FRotator ActorRotation = FRotator::ZeroRotator;


private:
FVector LastPosition = FVector::ZeroVector;
FRotator LastRotation = FRotator::ZeroRotator;

void UpdateActorTransformIfChanged();


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY()
	UMaterial* DebugMaterial = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spicy")
    FLinearColor DebugColor = FLinearColor::Green;


public:	
    UFUNCTION(BlueprintCallable)
	void Test1(); 

    UFUNCTION(BlueprintCallable)
	void Test2(float value); 

};
