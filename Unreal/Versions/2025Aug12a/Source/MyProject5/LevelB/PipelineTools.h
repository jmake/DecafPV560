// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "string"
#include "MainWidget.h"

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Camera/CameraActor.h"
#include "Components/SceneCaptureComponent2D.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/SceneCapture2D.h"
#include "Components/Image.h"

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PipelineTools.generated.h"


/*
	Displaying a live render from a SceneCaptureComponent2D in a UI widget image
*/
UCLASS()
class MYPROJECT5_API UPipelineTools : public UObject
{
	GENERATED_BODY()

public:
    // Step 1: Create Render Target
    UFUNCTION(BlueprintCallable, Category = "PipelineTools")
    static UTextureRenderTarget2D* CreateRenderTarget(UObject* WorldContextObject, int32 Width, int32 Height);

    // Step 2: Create Material Instance bound to RT
    UFUNCTION(BlueprintCallable, Category = "PipelineTools")
    static UMaterialInstanceDynamic* CreateMaterialForRT(UObject* WorldContextObject, UMaterialInterface* BaseMaterial, UTextureRenderTarget2D* RenderTarget);

    // Step 3: Create Scene Capture Actor and bind to RT
    UFUNCTION(BlueprintCallable, Category = "PipelineTools")
    static void CreateSceneCaptureActor(UObject* WorldContextObject, UTextureRenderTarget2D* RenderTarget, FVector Location, FRotator Rotation);

    UFUNCTION(BlueprintCallable, Category = "PipelineTools")
	static ASceneCapture2D* CreateSceneCaptureActor2(APlayerController* PC, UTextureRenderTarget2D* RenderTarget); 

    // Step 4: Create widget and apply Material or RT
    UFUNCTION(BlueprintCallable, Category = "PipelineTools")
    static UUserWidget* CreateWidgetWithImage(UObject* WorldContextObject, TSubclassOf<UUserWidget> WidgetClass, UTexture* Texture);

    UFUNCTION(BlueprintCallable, Category = "PipelineTools")
	static UUserWidget* CreateWidgetWithMaterial(UObject* WorldContextObject, TSubclassOf<UUserWidget> WidgetClass, UMaterialInterface* Material); 


UFUNCTION(BlueprintCallable, Category = "PipelineTools")
static TSubclassOf<UMainWidget> UMainWidgetFinder(FString& filename) //(std::string filename)
{
    //static ConstructorHelpers::FClassFinder<UMainWidget> WidgetBPClass(*FString(filename.c_str()));
    static ConstructorHelpers::FClassFinder<UMainWidget> WidgetBPClass(*filename);
	if (WidgetBPClass.Succeeded()) return WidgetBPClass.Class;

    return nullptr;
}

};

/*

void AMyActor::BeginPlay()
{
    Super::BeginPlay();

    // Create pipeline instance (pass 'this' as World context)
    UPipelineTools* Pipeline = UPipelineTools::CreatePipeline(this);
    if (!Pipeline) return;

    // 1) Create RT
    UTextureRenderTarget2D* RT = Pipeline->CreateRenderTarget(1024, 1024);

    // 2) Create a material instance that uses the render target
    //    -> Make sure MyBaseUIMaterial has Domain = User Interface and a texture parameter named "TextureParam"
    UMaterialInstanceDynamic* MID = Pipeline->CreateMaterialForRT(MyBaseUIMaterial, RT, TEXT("TextureParam"));

    // 3) Spawn SceneCapture2D and attach the RT
    ASceneCapture2D* Capture = Pipeline->CreateSceneCaptureActor(RT, FVector(0, 0, 200), FRotator(-30, 0, 0), true);

    // 4) Create widget (the widget must contain an Image named "TargetImage")
    UUserWidget* Widget = Pipeline->CreateWidgetWithMaterial(MyWidgetClass, MID, TEXT("TargetImage"), FVector2D(512,512));
    if (Widget)
    {
        Widget->AddToViewport();
    }
}


*/