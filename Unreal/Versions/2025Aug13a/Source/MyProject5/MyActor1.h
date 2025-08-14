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


//#include "SpicyTechDicom2.hpp"
#include <vtkPolyData.h>
#include <vtkImageData.h>

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyActor1.generated.h" // should always be the last


class CutterManager 
{
private : 
	//vtkPolyData* vtp;  
	APixelTextureDisplay* texture = nullptr; 

	vtkImageData* domain;  
	vtkImageData* cutter; 
	UWorld* world; 

public : 
	~CutterManager(){}; 
	CutterManager(){}; 

	void Finish(); 
	//vtkImageData* LoadFile(std::string); 

	void SetVti(vtkImageData*, UWorld*); 

	float* Create(double[9], float, float, float, int&, int&, float&, float&); 
	float* CreateAxial(float, float, float, int&, int&, float&, float&); 
	float* CreateCoronal(float, float, float, int&, int&, float&, float&); 
	float* CreateSagittal(float, float, float, int&, int&, float&, float&); 

	APixelTextureDisplay* CreateTexture(double[9], float, float, float, std::string);  
	APixelTextureDisplay* CreateAxialTexture(float, float, float, std::string); 
	APixelTextureDisplay* CreateCoronalTexture(float, float, float, std::string); 
	APixelTextureDisplay* CreateSagittalTexture(float, float, float, std::string); 
}; 



UCLASS()
class MYPROJECT5_API AMyActor1 : public AActor
{
	GENERATED_BODY()
	
public:	
	AMyActor1();
	~AMyActor1();

protected:
	virtual void BeginPlay() override;

private: 
vtkImageData* vti_ptr;
//std::vector<CutterManager*> cutters;
APixelTextureDisplay* textureActor = nullptr; 

std::vector<double> range; 
void LoadVtiFile(); 

	void TextureTest(); 
	void CutterCoronal(){}; 
	void CutterSagittal(){}; 

    UPROPERTY()
    UProceduralMeshComponent* Mesh;

	UMaterialInstanceDynamic* DynMaterialInstance = nullptr;


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
	void Test1()
	{
		UE_LOG(LogTemp, Warning, TEXT("Test1"));
	}; 

    UFUNCTION(BlueprintCallable)
	void Test2(float value) 
	{
		UE_LOG(LogTemp, Warning, TEXT("Test2 value : %f "), value );
	} 

UFUNCTION(BlueprintCallable)
void CreateAxial(float value);  

UFUNCTION(BlueprintCallable)
void CreateCoronal(float value);  

UFUNCTION(BlueprintCallable)
void CreateSagittal(float value);  

UFUNCTION(BlueprintCallable)
void CreateSourface(float value); 

};
