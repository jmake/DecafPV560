#include "BP_LevelBMainActor.h"
#include "Kismet/GameplayStatics.h"

//----------------------------------------------------------------------| |--//
static USliderWidget* AddSliderWidgetToCanvas(UCanvasPanel* ParentCanvas, const FString& BlueprintPath, APlayerController* playerController)
{
	if (!ParentCanvas || !playerController) return nullptr;

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
	USliderWidget* SliderWidget = CreateWidget<USliderWidget>(playerController, WidgetClass);
	if (!SliderWidget) return nullptr;

	// Add it to the canvas
	ParentCanvas->AddChild(SliderWidget);

	// Set it to fill the parent
	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(SliderWidget->Slot))
	{
		CanvasSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
		CanvasSlot->SetOffsets(FMargin(0.f));
	}

	//UPipelineTools::SetUIOnlyInputMode(SliderWidget, playerController); 
	return SliderWidget;
}


//----------------------------------------------------------------------| |--//
static AMyActor1* AddVtkActor(UWorld* world, FName ActorName, AActor* owner) 
{
	FVector SpawnLocation(0.f, 0.f, 0.f); // X, Y, Z
	FRotator SpawnRotation(0.f, 0.f, 0.f);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = owner;
    SpawnParams.Instigator = owner ? owner->GetInstigator() : nullptr; // safe way

	AMyActor1* MyActorInstance = world->SpawnActor<AMyActor1>(AMyActor1::StaticClass(), SpawnLocation, SpawnRotation, SpawnParams);
	if (!MyActorInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to spawn AMyActor1 at runtime"));
		return nullptr;
	}

    MyActorInstance->SetActorLabel(ActorName.ToString());


	UE_LOG(LogTemp, Warning, TEXT("[AddVtkActor] instance created!"));
	return MyActorInstance; 
}


//----------------------------------------------------------------------| |--//
static void AsyncTask(TFunction<void()> Task, TFunction<void()> After)
{
    auto Promise = MakeShared<TPromise<FString>>();
    TFuture<FString> Future = Promise->GetFuture();

    Async(EAsyncExecution::Thread, [Task, Promise]()
    {
		Task(); 
		Promise->SetValue( TEXT("Complete!") );
    });

    Future.Next([After](const FString& Result)
    {
        AsyncTask(ENamedThreads::GameThread, [After, Result]()
        {
			After(); 
            UE_LOG(LogTemp, Warning, TEXT("[AsyncTask] Result : '%s' "), *Result);
        });
    });
}

//----------------------------------------------------------------------| |--//


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

	//playerController = GetWorld()->GetFirstPlayerController();
	playerController = UGameplayStatics::GetPlayerController(this, 0);

	//WidgetCreate(); 

	vtkInstance = AddVtkActor(playerController->GetWorld(), FName(TEXT("VTK_Instance")), this); 

	FString fname("F:/z2025_1/Dicom/DecafPV560/domain.vti"); 
	vtkInstance->LoadVtiFile(fname); 

	vtkInstance->CreateSourface(0.5, [this](){ WidgetCreate(); }); 
	AsyncTask([](){}, [](){}); 

	axialTexture = vtkInstance->CreateAxial(0.5); 
}


void ABP_LevelBMainActor::WidgetCreate()
{
	UMainWidget* MainWidgetInstance = nullptr;	

	MainWidgetInstance = CreateWidget<UMainWidget>(playerController, UMainWidget::StaticClass());
	//if(MainWidget) MainWidgetInstance = CreateWidget<UMainWidget>(playerController, MainWidget);

	LiveRenderLayout(MainWidgetInstance); 

	MainWidgetInstance->AddToViewport(); //(zorder);
	//BlockBackGroud(MainWidgetInstance); 

	UE_LOG(LogTemp, Warning, TEXT("[WidgetCreate] done!") );
} 


void ABP_LevelBMainActor::BlockBackGroud(UMainWidget* MainWidgetInstance)
{
	//FInputModeUIOnly InputMode; // :( 
	FInputModeGameAndUI InputMode;
	InputMode.SetWidgetToFocus(MainWidgetInstance->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

	playerController->SetInputMode(InputMode);
	playerController->bShowMouseCursor = true;
	playerController->bEnableClickEvents = true;
	playerController->bEnableMouseOverEvents = true;
}


//----------------------------------------------------------------------| |--//
//----------------------------------------------------------------------| |--//
void ABP_LevelBMainActor::LiveRenderLayout(UMainWidget* MainWidgetInstance) 
{
    if (MainWidgetInstance == nullptr) return ; 

	// 'Content Drawer' > 'Widget Blueprint' > 'Copy Reference' 
	FString SliderBPPath = TEXT("/Game/SpicyTech/LevelB/WBP_RadialSlider3.WBP_RadialSlider3_C"); 
	
	FLinearColor Cyan(0.f, 1.f, 1.f, 1.f);

	// Top-left
	UCanvasPanel* TopLeft    = UPipelineTools::CreateCanvasPanel(MainWidgetInstance, "TopLeft", 0.0f, 0.0f, 0.5f, 0.5f, FLinearColor::Red); 
	//UTextBlock* MyText1 = UPipelineTools::AddTextToCanvas(TopLeft, "MyText1");
	USliderWidget* SliderTL = AddSliderWidgetToCanvas(TopLeft, SliderBPPath, playerController);
	LiveRenderApply( SliderTL ); 

	// Top-right
	UCanvasPanel* TopRight    = UPipelineTools::CreateCanvasPanel(MainWidgetInstance, "TopRight", 0.5f, 0.0f, 1.0f, 0.5f, FLinearColor::White); 
	USliderWidget* SliderTR = AddSliderWidgetToCanvas(TopRight, SliderBPPath, playerController);
	LiveRenderApply( SliderTR ); 

	// Bottom-left
	UCanvasPanel* BottomLeft = UPipelineTools::CreateCanvasPanel(MainWidgetInstance, "BottomLeft", 0.0f, 0.5f, 0.5f, 1.0f, FLinearColor::Yellow);  
	USliderWidget* SliderBL = AddSliderWidgetToCanvas(BottomLeft, SliderBPPath, playerController);
	LiveRenderApply( SliderBL ); 

	// Bottom-right
	UCanvasPanel* BottomRight = UPipelineTools::CreateCanvasPanel(MainWidgetInstance, "BottomRight", 0.5f, 0.5f, 1.0f, 1.0f, FLinearColor::Transparent); 
	UButton* ButtonBR = UPipelineTools::AddButtonToCanvas(BottomRight, "ButtonBR", 0.1, 0.1, 0.2, 0.2); 
	//USliderWidget* SliderBR = AddSliderWidgetToCanvas(BottomRight, SliderBPPath, playerController);
}


void ABP_LevelBMainActor::LiveRenderApply(USliderWidget* RadialSliderWidget)
{
	//
	// Create a material (M_Runtime2DTexture) 
	//	> Material domain (User interface) 
	//	> Add 'Vector Parameter' (BaseColor)
	//	> Add 'Texture Sample Parameter2D' (Param2D)	
	//
	if(RadialSliderWidget == nullptr) 
	{
		UE_LOG(LogTemp, Warning, TEXT("[LiveRenderApply] 'RadialSliderWidget' fails!!") ); 
		return ; 
	}

	FString ImageIdPath = TEXT("ImageId"); 
	FString Param2DName = TEXT("Param2D"); 
	FString MaterialPath = TEXT("/Game/SpicyTech/LevelB/M_Runtime2DTexture.M_Runtime2DTexture");

	RadialSliderWidget->SetLiveRender(playerController, MaterialPath, Param2DName); 

	UWidget* FoundWidget = RadialSliderWidget->GetWidgetFromName(*ImageIdPath); 
	if (FoundWidget == nullptr) return ;

	UImage* ImageWidget = Cast<UImage>(FoundWidget); 
	if (ImageWidget == nullptr) return ;

	ImageWidget->SetColorAndOpacity( FLinearColor(1.0f, 1.0f, 1.0f, 1.0f) ); 

	if(RadialSliderWidget->dynamicMaterial) ImageWidget->SetBrushFromMaterial( RadialSliderWidget->dynamicMaterial );

	if(axialTexture == nullptr) UE_LOG(LogTemp, Warning, TEXT("[LiveRenderApply] 'axialTexture' fails!!") ); 
	ImageWidget->SetBrushFromTexture( axialTexture );

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