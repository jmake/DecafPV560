#include "BP_LevelBMainActor.h"
#include "Kismet/GameplayStatics.h"


//----------------------------------------------------------------------| |--//
//----------------------------------------------------------------------| |--//
ABP_LevelBMainActor::ABP_LevelBMainActor()
{
	PrimaryActorTick.bCanEverTick = true;

	std::string filename = "/Game/SpicyTech/LevelB/WBP_LevelB"; 
	MainWidget = UMainWidgetFinder(filename); 
}


void ABP_LevelBMainActor::BeginPlay()
{
	Super::BeginPlay();

	WidgetCreate(); 

	//LiveRender(); 
}


//----------------------------------------------------------------------| |--//
//----------------------------------------------------------------------| |--//
TSubclassOf<UMainWidget> ABP_LevelBMainActor::UMainWidgetFinder(std::string filename)
{
    static ConstructorHelpers::FClassFinder<UMainWidget> WidgetBPClass(*FString(filename.c_str()));
    if (WidgetBPClass.Succeeded()) return WidgetBPClass.Class; 

    return nullptr;
}


void ABP_LevelBMainActor::WidgetCreate()
{	
	if(MainWidget == nullptr) return ; 

	//APlayerController* PC = GetWorld()->GetFirstPlayerController();
	playerController = UGameplayStatics::GetPlayerController(this, 0);
	MainWidgetInstance = CreateWidget<UMainWidget>(playerController, MainWidget);
	//MainWidgetInstance = CreateWidget<UMainWidget>(GetWorld(), MainWidget); // :) 
	
	int zorder = 999; 
    if (MainWidgetInstance)
    {
    	MainWidgetInstance->Rename( TEXT("MyWidgetInstanceName") );
		MainWidgetInstance->AddToViewport(zorder);
		//BlockBackGroud( playerController ); 
	} 

	LiveRender( playerController ); 
} 


void ABP_LevelBMainActor::BlockBackGroud(APlayerController* PC)
{
	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(MainWidgetInstance->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

	PC->SetInputMode(InputMode);
	PC->bShowMouseCursor = true;
}


//----------------------------------------------------------------------| |--//
//----------------------------------------------------------------------| |--//
/*
void ABP_LevelBMainActor::LiveRender(APlayerController* PC)
{
	if(PC == nullptr) return ; 

	AActor* ViewTarget = PC->GetViewTarget();
	if (ViewTarget == nullptr) return ; 
	
	ACameraActor* CameraActor = Cast<ACameraActor>(ViewTarget);
	if (CameraActor == nullptr) return ; 
} 
*/


void ABP_LevelBMainActor::LiveRender(APlayerController* PC)
{
	/*
	Create a material (M_Runtime2DTexture) 
		> Material domain (User interface) 
		> Add 'Vector Parameter' (BaseColor)
		> Add 'Texture Sample Parameter2D' (Param2D)
	*/	
	FString ImageIdPath = TEXT("ImageId"); 
	FString Param2DName = TEXT("Param2D"); 
	FString MaterialPath = TEXT("/Game/SpicyTech/LevelB/M_Runtime2DTexture.M_Runtime2DTexture");
	FString MainWidgetPath = TEXT("WBP_RadialSlider"); 

	UUserWidget* RadialSliderWidget = Cast<UUserWidget>(MainWidgetInstance->GetWidgetFromName(*MainWidgetPath)) ; 
	if(RadialSliderWidget == nullptr) 
	{
		UE_LOG(LogTemp, Warning, TEXT("[LiveRender] 'RadialSliderWidget' fails!!") ); 
		return ; 
	}

    UTextureRenderTarget2D* RenderTarget = nullptr; 
	RenderTarget = UPipelineTools::CreateRenderTarget(this, 1024, 1024);

    //ASceneCapture2D* SceneCapture = 
	//UPipelineTools::CreateSceneCaptureActor(this, RenderTarget, FVector(0, 0, 200), FRotator(-30, 0, 0));
	captureActor = UPipelineTools::CreateSceneCaptureActor2(PC, RenderTarget);

	UMaterialInterface* BaseMaterial = nullptr; 
	UMaterialInstanceDynamic* DynamicMaterial = nullptr;

	BaseMaterial = LoadObject<UMaterialInterface>(nullptr, *MaterialPath);
	DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);

	if(DynamicMaterial == nullptr) UE_LOG(LogTemp, Warning, TEXT("[LiveRender] 'Material' fails!!") ); 

	if (UWidget* FoundWidget = RadialSliderWidget->GetWidgetFromName(*ImageIdPath))
	{
		if (UImage* ImageWidget = Cast<UImage>(FoundWidget))
		{
			if(DynamicMaterial)
			{
				//// FinalColor = (MaterialOutput * BrushTint) * ColorAndOpacity ?? 
				DynamicMaterial->SetTextureParameterValue(*Param2DName, RenderTarget);
				////DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"),  FLinearColor(1.0f, 1.0f, 1.0f, 1.0f) );
				////ImageWidget->SetColorAndOpacity( FLinearColor(1.0f, 1.0f, 1.0f, 1.0f) ); // ?? 
				ImageWidget->SetColorAndOpacity( FLinearColor(1.0f, 1.0f, 1.0f, 1.0f) ); 
				ImageWidget->SetBrushFromMaterial(DynamicMaterial);
				//ImageWidget->SetDesiredSizeOverride(FVector2D(512, 512));	
			}
			else
			{
				ImageWidget->SetColorAndOpacity( FLinearColor(0.0f, 1.0f, 0.0f, 1.0f) ); 
			}
		}
	}
}


//----------------------------------------------------------------------| |--//
//----------------------------------------------------------------------| |--//
void ABP_LevelBMainActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(playerController == nullptr) return ; 

	FVector CamLoc;
	FRotator CamRot;
	playerController->PlayerCameraManager->GetCameraViewPoint(CamLoc, CamRot);

    //ASceneCapture2D* CaptureActor = playerController->GetWorld()->SpawnActor<ASceneCapture2D>();
	captureActor->SetActorLocationAndRotation(CamLoc, CamRot);

}


//----------------------------------------------------------------------| |--//
//----------------------------------------------------------------------| |--//