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

    void Init(int32 InWidth, int32 InHeight, float sizex, float sizey);

    void Apply(TArrayView<float> Data, float ScaleFactor); 

//    template <template<typename> class T, typename U> void Apply(T<U> Data, float ScaleFactor);

//    void Apply(const TArray<float>& Data, float ScaleFactor);
//    void PlaneUpdate(AActor* Target);
    void Save(std::string fname); 

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
