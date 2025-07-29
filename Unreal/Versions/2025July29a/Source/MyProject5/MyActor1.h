// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

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
	// Sets default values for this actor's properties
	AMyActor1();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY()
	UMaterial* DebugMaterial = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    FLinearColor DebugColor = FLinearColor::Green;

private:
    UPROPERTY()
    UProceduralMeshComponent* Mesh;

	UMaterialInstanceDynamic* DynMaterialInstance = nullptr;

	SpicyTech2::VtkTest *dicom = nullptr; 
};
