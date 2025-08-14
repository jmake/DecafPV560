#include "BP_LevelBMainActor.h"
#include "Kismet/GameplayStatics.h"


//UFUNCTION(BlueprintCallable, Category = "SpicyTools")
static USliderWidget* AddSliderWidgetToCanvas(UCanvasPanel* ParentCanvas, const FString& BlueprintPath, APlayerController* PlayerController)
{
	if (!ParentCanvas || !PlayerController) return nullptr;

	// 'Content Drawer' > 'Widget Blueprint' > 'Copy Reference' 
	// 'Copy Reference' returns "/Script/UMGEditor.WidgetBlueprint'/Game/SpicyTech/LevelB/WBP_RadialSlider3.WBP_RadialSlider3" (without suffix '_C')
	if (!BlueprintPath.EndsWith(TEXT("_C")))     // Verify that the path ends with "_C"
	{
		UE_LOG(LogTemp, Error, TEXT("The Blueprint path must end with '_C': %s"), *BlueprintPath);
		return nullptr;
	}

	// Load the widget class
	TSubclassOf<UUserWidget> WidgetClass = LoadClass<UUserWidget>(nullptr, *BlueprintPath);
	if (!WidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load widget class: %s"), *BlueprintPath);
		return nullptr;
	}

	// Create the widget
	USliderWidget* SliderWidget = CreateWidget<USliderWidget>(PlayerController, WidgetClass);
	if (!SliderWidget) return nullptr;

	// Add it to the canvas
	ParentCanvas->AddChild(SliderWidget);

	// Set it to fill the parent
	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(SliderWidget->Slot))
	{
		CanvasSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
		CanvasSlot->SetOffsets(FMargin(0.f));
	}

	return SliderWidget;
}


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

	LiveRenderLayout(); 

	int zorder = 999; 
	//MainWidgetInstance->Rename( TEXT("MyWidgetInstanceName") );
	MainWidgetInstance->AddToViewport(zorder);
	//BlockBackGroud(  ); 
} 


void ABP_LevelBMainActor::BlockBackGroud()
{
	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(MainWidgetInstance->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

	playerController->SetInputMode(InputMode);
	playerController->bShowMouseCursor = true;
}


//----------------------------------------------------------------------| |--//
//----------------------------------------------------------------------| |--//
void ABP_LevelBMainActor::LiveRenderLayout() 
{
	FString SliderBPPath = TEXT("/Game/SpicyTech/LevelB/WBP_RadialSlider3.WBP_RadialSlider3_C"); // 'Content Drawer' > 'Widget Blueprint' > 'Copy Reference' 

	FLinearColor Cyan(0.f, 1.f, 1.f, 1.f);

	// Top-left
	UCanvasPanel* TopLeft    = UPipelineTools::CreateCanvasPanel(MainWidgetInstance, "TopLeft", 0.0f, 0.0f, 0.5f, 0.5f, Cyan); 
	//UTextBlock* MyText1 = UPipelineTools::AddTextToCanvas(TopLeft, "MyText1");
	USliderWidget* SliderTL = AddSliderWidgetToCanvas(TopLeft, SliderBPPath, playerController);
	LiveRenderApply( SliderTL ); 

	// Top-right
	UCanvasPanel* TopRight    = UPipelineTools::CreateCanvasPanel(MainWidgetInstance, "TopRight", 0.5f, 0.0f, 1.0f, 0.5f, FLinearColor::White); 
	//UTextBlock* MyText2 = UPipelineTools::AddTextToCanvas(TopRight, "MyText2", 0.0, 0.0, 0.2, 0.2);
	//UTextBlock* MyText3 = UPipelineTools::AddTextToCanvas(TopRight, "MyText3", 0.75, 0.75, 1.0, 1.0);
	USliderWidget* SliderTR = AddSliderWidgetToCanvas(TopRight, SliderBPPath, playerController);
	LiveRenderApply( SliderTR ); 

	// Bottom-left
	UCanvasPanel* BottomLeft = UPipelineTools::CreateCanvasPanel(MainWidgetInstance, "BottomLeft", 0.0f, 0.5f, 0.5f, 1.0f, FLinearColor::Yellow);  
	UButton* MyButtonBL = UPipelineTools::AddButtonToCanvas(BottomLeft, "MyButton1", 0.1, 0.1, 0.2, 0.2); 
	USliderWidget* SliderBL = AddSliderWidgetToCanvas(BottomLeft, SliderBPPath, playerController);
	LiveRenderApply( SliderBL ); 

	// Bottom-right
	//UCanvasPanel* BottomRight = UPipelineTools::CreateCanvasPanel(MainWidgetInstance, "BottomRight", 0.5f, 0.5f, 1.0f, 1.0f, FLinearColor::Green); 
	//USliderWidget* SliderBR = AddSliderWidgetToCanvas(BottomRight, SliderBPPath, playerController);

}


void ABP_LevelBMainActor::LiveRenderApply(USliderWidget* RadialSliderWidget)
{
	/*
	Create a material (M_Runtime2DTexture) 
		> Material domain (User interface) 
		> Add 'Vector Parameter' (BaseColor)
		> Add 'Texture Sample Parameter2D' (Param2D)
	*/	

	//FString MainWidgetPath = TEXT("WBP_RadialSlider3"); 
	//USliderWidget* RadialSliderWidget = Cast<USliderWidget>(MainWidgetInstance->GetWidgetFromName(*MainWidgetPath)) ; 
	if(RadialSliderWidget == nullptr) 
	{
		UE_LOG(LogTemp, Warning, TEXT("[LiveRender] 'RadialSliderWidget' fails!!") ); 
		return ; 
	}

	FString ImageIdPath = TEXT("ImageId"); 
	FString Param2DName = TEXT("Param2D"); 
	FString MaterialPath = TEXT("/Game/SpicyTech/LevelB/M_Runtime2DTexture.M_Runtime2DTexture");
/*
	// By using 'OwnerMaterial = this' each 'RadialSliderWidget' shares the same 'DynamicMaterial', on the other hand 
	//          'OwnerMaterial = RadialSliderWidget' each of them has its own 'DynamicMaterial'
	UObject* OwnerMaterial = RadialSliderWidget;  // or "this" 

	UTextureRenderTarget2D* RenderTarget = nullptr; 
	RenderTarget = UPipelineTools::CreateRenderTarget(OwnerMaterial, 1024, 1024);

    //ASceneCapture2D* SceneCapture = 
	//UPipelineTools::CreateSceneCaptureActor(this, RenderTarget, FVector(0, 0, 200), FRotator(-30, 0, 0));
	captureActor = UPipelineTools::CreateSceneCaptureActor2(playerController, RenderTarget);

	UMaterialInterface* BaseMaterial = nullptr; 
	BaseMaterial = LoadObject<UMaterialInterface>(nullptr, *MaterialPath);

	UMaterialInstanceDynamic* DynamicMaterial = nullptr;
	DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, OwnerMaterial);
*/
//	UMaterialInstanceDynamic* DynamicMaterial = nullptr;

	RadialSliderWidget->SetLiveRender(playerController, MaterialPath, Param2DName); 
//	captureActor = RadialSliderWidget->captureActor;
//	DynamicMaterial = RadialSliderWidget->dynamicMaterial; 

//	if(DynamicMaterial == nullptr) UE_LOG(LogTemp, Warning, TEXT("[LiveRender] 'Material' fails!!") ); 

	if (UWidget* FoundWidget = RadialSliderWidget->GetWidgetFromName(*ImageIdPath))
	{
		if (UImage* ImageWidget = Cast<UImage>(FoundWidget))
		{
			if(RadialSliderWidget->dynamicMaterial)
			{
				//// FinalColor = (MaterialOutput * BrushTint) * ColorAndOpacity ?? 
//				DynamicMaterial->SetTextureParameterValue(*Param2DName, RenderTarget);
				////DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"),  FLinearColor(1.0f, 1.0f, 1.0f, 1.0f) );
				////ImageWidget->SetColorAndOpacity( FLinearColor(1.0f, 1.0f, 1.0f, 1.0f) ); // ?? 
				ImageWidget->SetColorAndOpacity( FLinearColor(1.0f, 1.0f, 1.0f, 1.0f) ); 
				ImageWidget->SetBrushFromMaterial( RadialSliderWidget->dynamicMaterial );
				//ImageWidget->SetDesiredSizeOverride(FVector2D(512, 512));	
			}
			else
			{
				ImageWidget->SetColorAndOpacity( FLinearColor(0.0f, 1.0f, 0.0f, 1.0f) ); 
			}
		}
	}

	SliderWidgets.Add( RadialSliderWidget );
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

	//if(captureActor == nullptr) return ; 
	//captureActor->SetActorLocationAndRotation(CamLoc, CamRot);

	//UpdateLiveRender(CamLoc, CamRot);  

	for (USliderWidget* Slider : SliderWidgets)
	{
		Slider->UpdateLiveRender(CamLoc, CamRot);
	}

}


//----------------------------------------------------------------------| |--//
//----------------------------------------------------------------------| |--//