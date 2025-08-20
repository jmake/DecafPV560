// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Templates/Function.h" 

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

/*
const double AXIAL[9] = {1, 0,  0, 0, 1,  0, 0, 0, 1}; 
const double CORONAL[9] = {1, 0,  0, 0, 0, -1, 0, 1, 0};
const double SAGITTAL[9] = {0, 0, -1, 1, 0,  0, 0, 1, 0}; 

void LoadVtiFile(
	const std::string& fname, 
	vtkImageData*& vti_ptr, 
	std::vector<double>& range
);  


struct AnatomicalProperties
{
	float* data;
	int32 width;
	int32 height;
	float spacingX;
	float spacingY;
};

float* Vti2AnatomicalBuffer(
							vtkImageData* domain, 
							double anatomicalView[9], 
							float x0, float y0, float z0, 
							vtkImageData*& cutter, 
							int& width, int& height, 
							float& sizeX, float& sizeY 
						); 

struct SurfaceProperties
{
	int n_rows; 
	int n_cols; 
	float* points; 
	int n_triangles; 
	long long* triangles;  
};

void Vti2SurfaceBuffer( 
						vtkImageData* domain, 
						float value, 
						float valueMin, 
						float valueMax, 
						vtkPolyData*& vtp, 
						float*& points, 
						int& n_rows, 
						int& n_cols, 
						long long*& triangles, 
						int n_triangles
					); 
*/

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

	void SetVti(vtkImageData*, UWorld*); 
	void CreateTextureAsync(APixelTextureDisplay*, std::string, double[9], float, float, float, TFunction<void(APixelTextureDisplay*)>);

	float* CreateBuffer(double[9], float, float, float, int&, int&, float&, float&); 
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
//void LoadVtiFile(); 

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
	UTexture2D* CreateAxial(float value);  

	UFUNCTION(BlueprintCallable)
	void CreateCoronal(float value);  

	UFUNCTION(BlueprintCallable)
	void CreateSagittal(float value);  

	//UFUNCTION(BlueprintCallable) // Unreal’s reflection system only supports UObjects, primitive types, and delegates,
	void CreateSourface(float value, TFunction<void()> After); 

	UFUNCTION(BlueprintCallable)
	void LoadVtiFile(FString& filename); 

	void CreateTextureAsync(APixelTextureDisplay* textureActor, double anatomicalView[9], double center[3], TFunction<void(APixelTextureDisplay*)> After);

};
