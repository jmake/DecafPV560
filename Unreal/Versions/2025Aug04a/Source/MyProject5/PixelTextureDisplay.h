#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PixelTextureDisplay.generated.h"


UCLASS()
class MYPROJECT5_API APixelTextureDisplay : public AActor
{
    GENERATED_BODY()

private:
    FVector LastPosition = FVector::ZeroVector;
    FRotator LastRotation = FRotator::ZeroRotator;

    void UpdateActorTransformIfChanged();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;


public:
    APixelTextureDisplay();

    void Init(int32 InWidth, int32 InHeight);

    void Apply(TArrayView<float> Data, float ScaleFactor); 

//    template <template<typename> class T, typename U> void Apply(T<U> Data, float ScaleFactor);

//    void Apply(const TArray<float>& Data, float ScaleFactor);
//    void PlaneUpdate(AActor* Target);
    void Save(); 

//    TArray<float> GeneratePerlinData(int32 W, int32 H, float Time, float Scale);
//    TArray<float> GenerateChessboardData(int32 W, int32 H, int32 NumSquaresX, int32 NumSquaresY); 


//    void EnsureMaterial();
    void CreateOrUpdateTexture(int32 NewWidth, int32 NewHeight);
//    void CreatePlane();
    void PlaneScaleUpdate(int32 W, int32 H, float ScaleFactor);

    UPROPERTY()
    UStaticMeshComponent* PlaneComponent = nullptr;

    UPROPERTY()
    UMaterialInstanceDynamic* DynamicMaterial = nullptr;

    UPROPERTY()
    UTexture2D* DynamicTexture = nullptr;

    UPROPERTY()
    //UMaterial* BaseMaterial = nullptr;
    UMaterialInterface* BaseMaterial = nullptr;

    int32 Width = 0;
    int32 Height = 0;
    float LastScaleFactor = -1.f;

    FLinearColor ColorMin = FLinearColor::White;
    FLinearColor ColorMax = FLinearColor::Black;

public:	
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform")
FVector ActorPosition = FVector::ZeroVector;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform")
FRotator ActorRotation = FRotator::ZeroRotator;

};

/*
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PixelTextureDisplay.generated.h" // should always be the last

UCLASS()
class MYPROJECT5_API APixelTextureDisplay : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APixelTextureDisplay();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


public:
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Texture")
FColor ColorMin = FColor::White;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Texture")
FColor ColorMax = FColor::Black;

void Init(int32 InWidth, int32 InHeight);
void Apply(const TArray<float>& Data, float ScaleFactor);
void PlaneUpdate(AActor* Target);
void PlaneScaleUpdate(int32 W, int32 H, float ScaleFactor);

TArray<float> GeneratePerlinData(int32 W, int32 H, float Time);


private:
int32 Width = 0;
int32 Height = 0;
float LastScaleFactor = -1.0f;

UPROPERTY()
UTexture2D* DynamicTexture = nullptr;

UPROPERTY()
UStaticMeshComponent* PlaneComponent = nullptr;

UPROPERTY()
UMaterial* GeneratedMaterial = nullptr;

UPROPERTY()
UMaterialInstanceDynamic* DynamicMaterial = nullptr;

void CreateOrUpdateTexture(int32 NewWidth, int32 NewHeight);
void EnsureMaterial();
void CreatePlane();
void CreateDefaultUnlitMaterial();

};
*/