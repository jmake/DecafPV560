#include "PipelineTools.h"


UTextureRenderTarget2D* UPipelineTools::CreateRenderTarget(UObject* WorldContextObject, int32 Width, int32 Height)
{
    UTextureRenderTarget2D* RT = NewObject<UTextureRenderTarget2D>(WorldContextObject);
    RT->Rename(TEXT("PipelineTools_TextureRenderTarget2D"));

    RT->InitCustomFormat(Width, Height, PF_B8G8R8A8, false);
    RT->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA8;
    RT->UpdateResourceImmediate(true);

    return RT;
}


void UPipelineTools::CreateSceneCaptureActor(UObject* WorldContextObject, UTextureRenderTarget2D* RenderTarget, FVector Location, FRotator Rotation)
{
    if (!WorldContextObject || !RenderTarget) return ;

    UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
    ASceneCapture2D* CaptureActor = World->SpawnActor<ASceneCapture2D>(Location, Rotation);

    USceneCaptureComponent2D* CaptureComp = CaptureActor->GetCaptureComponent2D();

    if (CaptureActor && CaptureComp)
    {
        CaptureComp->TextureTarget = RenderTarget;
        CaptureComp->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
        CaptureComp->bCaptureEveryFrame = true;
        CaptureComp->FOVAngle = 90.0f;
    }
}


ASceneCapture2D* UPipelineTools::CreateSceneCaptureActor2(APlayerController* PC, UTextureRenderTarget2D* RenderTarget)
{
	if(PC == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CreateSceneCaptureActor2] 'PC' fails!! ") );    
        return nullptr; 
    }

    ASceneCapture2D* CaptureActor = PC->GetWorld()->SpawnActor<ASceneCapture2D>();
	if (CaptureActor == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CreateSceneCaptureActor2] 'CaptureActor' fails!! ") );    
        return nullptr; 
    }

    AActor* ViewTarget = PC->GetViewTarget(); 
	if (ViewTarget == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CreateSceneCaptureActor2] 'ViewTarget' fails!! ") );    
        return nullptr; 
    }

    // Attach the SceneCapture actor to the view target
    CaptureActor->AttachToActor(ViewTarget, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

    USceneCaptureComponent2D* CaptureComp = CaptureActor->GetCaptureComponent2D();
	if (CaptureComp == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CreateSceneCaptureActor2] 'CaptureComp' fails!! ") );    
        return nullptr; 
    }

    CaptureComp->TextureTarget = RenderTarget;
    CaptureComp->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
    CaptureComp->bCaptureEveryFrame = true;
    CaptureComp->FOVAngle = 90.0f;


/*
	AActor* ViewTarget = PC->GetViewTarget();
	if (ViewTarget == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CreateSceneCaptureActor2] 'ViewTarget' fails!! ") );    
        return ; 
    }

	ACameraActor* CameraActor = Cast<ACameraActor>(ViewTarget);
	if (CameraActor == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CreateSceneCaptureActor2] 'CameraActor' fails!! ") );    
        return ; 
    }

    // Find or create SceneCaptureComponent2D on the CameraActor
    USceneCaptureComponent2D* CaptureComp = CameraActor->FindComponentByClass<USceneCaptureComponent2D>();
    if (!CaptureComp)
    {
        CaptureComp = NewObject<USceneCaptureComponent2D>(CameraActor);

        CaptureComp->TextureTarget = RenderTarget;
        CaptureComp->SetupAttachment( CameraActor->GetRootComponent() );
        CaptureComp->RegisterComponent();
    }    
*/

    return CaptureActor; 
}


UMaterialInstanceDynamic* UPipelineTools::CreateMaterialForRT(UObject* WorldContextObject, UMaterialInterface* BaseMaterial, UTextureRenderTarget2D* RenderTarget)
{
    if (!BaseMaterial || !RenderTarget) return nullptr;
    
    UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(BaseMaterial, WorldContextObject);
    MID->SetTextureParameterValue(FName("TextureParam"), RenderTarget);

    return MID;
}


UUserWidget* UPipelineTools::CreateWidgetWithImage(UObject* WorldContextObject, TSubclassOf<UUserWidget> WidgetClass, UTexture* Texture)
{
    if (!WorldContextObject || !WidgetClass || !Texture) return nullptr;

    UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);

    UUserWidget* Widget = CreateWidget<UUserWidget>(World, WidgetClass);
    if (Widget)
    {
        // Assume the widget has an Image named "TargetImage"
        UImage* ImageWidget = Cast<UImage>(Widget->GetWidgetFromName(TEXT("TargetImage")));
        if (ImageWidget)
        {
            FSlateBrush Brush;
            Brush.SetResourceObject(Texture);
            Brush.ImageSize = FVector2D(512, 512); // match your RT size

            ImageWidget->SetBrush(Brush);
        }
    }
    return Widget;
}


UUserWidget* UPipelineTools::CreateWidgetWithMaterial(UObject* WorldContextObject, TSubclassOf<UUserWidget> WidgetClass, UMaterialInterface* Material)
{
    if (!WorldContextObject || !WidgetClass || !Material) return nullptr;

    UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);

    UUserWidget* Widget = CreateWidget<UUserWidget>(World, WidgetClass);
    if (Widget)
    {
        UImage* ImageWidget = Cast<UImage>(Widget->GetWidgetFromName(TEXT("TargetImage")));
        if (ImageWidget)
        {
            ImageWidget->SetBrushFromMaterial(Material);
            ImageWidget->SetDesiredSizeOverride(FVector2D(512, 512));
        }
    }
    return Widget;
}
