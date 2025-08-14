#include "BP_LevelBMainActor.h"
#include "Kismet/GameplayStatics.h"

#include "SliderWidget.h"
#include "PipelineTools.h"

//----------------------------------------------------------------------| |--//
//----------------------------------------------------------------------| |--//
ABP_LevelBMainActor::ABP_LevelBMainActor()
{
	PrimaryActorTick.bCanEverTick = true;

	std::string filename = "/Game/SpicyTech/LevelB/WBP_LevelB"; 
	MainWidget = UMainWidgetFinder(filename); 
}


TSubclassOf<UMainWidget> ABP_LevelBMainActor::UMainWidgetFinder(std::string filename)
{
    static ConstructorHelpers::FClassFinder<UMainWidget> WidgetBPClass(*FString(filename.c_str()));
    if (WidgetBPClass.Succeeded()) return WidgetBPClass.Class; 

    return nullptr;
}


//----------------------------------------------------------------------| |--//
//----------------------------------------------------------------------| |--//
void ABP_LevelBMainActor::BeginPlay()
{
	Super::BeginPlay();

	WidgetCreate(); 
}


void ABP_LevelBMainActor::WidgetCreate()
{
	//APlayerController* PC = GetWorld()->GetFirstPlayerController();
	playerController = UGameplayStatics::GetPlayerController(this, 0);

	MainWidgetInstance = CreateWidget<UMainWidget>(playerController, UMainWidget::StaticClass());
	//if(MainWidget) MainWidgetInstance = CreateWidget<UMainWidget>(playerController, MainWidget);
	
    if (MainWidgetInstance == nullptr) return ; 

	LiveRenderCreate(MainWidgetInstance); 

	int zorder = 999; 
	//MainWidgetInstance->Rename( TEXT("MyWidgetInstanceName") );
	MainWidgetInstance->AddToViewport(zorder);
	//BlockBackGroud( playerController ); 
	//LiveRenderApply( playerController ); 
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
void ABP_LevelBMainActor::LiveRenderCreate(UMainWidget* mainWidgetInstance) 
{
	FString SliderBPPath = TEXT("/Game/SpicyTech/LevelB/WBP_RadialSlider3.WBP_RadialSlider3_C"); // 'Content Drawer' > 'Widget Blueprint' > 'Copy Reference' 

	FLinearColor Cyan(0.f, 1.f, 1.f, 1.f);
	//UCanvasPanel* Top1    = UPipelineTools::CreateCanvasPanel(mainWidgetInstance, "Top1",    0.0f, 0.0f, 0.5f, 0.5f, Cyan); // Top-left
	//UTextBlock* MyText1 = UPipelineTools::AddTextToCanvas(Top1, "MyText1");

	//UCanvasPanel* Top2    = UPipelineTools::CreateCanvasPanel(mainWidgetInstance, "Top2",    0.5f, 0.0f, 1.0f, 0.5f, FLinearColor::White);  // Top-right
	//UTextBlock* MyText2 = UPipelineTools::AddTextToCanvas(Top2, "MyText2", 0.0, 0.0, 0.2, 0.2);
	//UTextBlock* MyText3 = UPipelineTools::AddTextToCanvas(Top2, "MyText3", 0.75, 0.75, 1.0, 1.0);

	UCanvasPanel* Bottom1 = UPipelineTools::CreateCanvasPanel(mainWidgetInstance, "Bottom1", 0.0f, 0.5f, 0.5f, 1.0f, FLinearColor::Yellow);  // Bottom-left
	USliderWidget* Slider1 = UPipelineTools::AddSliderWidgetToCanvas(Bottom1, SliderBPPath, playerController);
	UButton* MyButton1 = UPipelineTools::AddButtonToCanvas(Bottom1, "MyButton1", 0.1, 0.1, 0.2, 0.2); 

	UCanvasPanel* Bottom2 = UPipelineTools::CreateCanvasPanel(mainWidgetInstance, "Bottom2", 0.5f, 0.5f, 1.0f, 1.0f, FLinearColor::Green); // Bottom-right
	USliderWidget* Slider2 = UPipelineTools::AddSliderWidgetToCanvas(Bottom2, SliderBPPath, playerController);
}


void ABP_LevelBMainActor::LiveRenderApply(APlayerController* PC)
{
	/*
	Create a material (M_Runtime2DTexture) 
		> Material domain (User interface) 
		> Add 'Vector Parameter' (BaseColor)
		> Add 'Texture Sample Parameter2D' (Param2D)
	*/	
	FString MainWidgetPath = TEXT("WBP_RadialSlider3"); 

	UUserWidget* RadialSliderWidget = Cast<UUserWidget>(MainWidgetInstance->GetWidgetFromName(*MainWidgetPath)) ; 
	if(RadialSliderWidget == nullptr) 
	{
		UE_LOG(LogTemp, Warning, TEXT("[LiveRender] 'RadialSliderWidget' fails!!") ); 
		return ; 
	}


	FString ImageIdPath = TEXT("ImageId"); 
	FString Param2DName = TEXT("Param2D"); 
	FString MaterialPath = TEXT("/Game/SpicyTech/LevelB/M_Runtime2DTexture.M_Runtime2DTexture");

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

	if(captureActor == nullptr) return ; 

	captureActor->SetActorLocationAndRotation(CamLoc, CamRot);
}


//----------------------------------------------------------------------| |--//
//----------------------------------------------------------------------| |--//