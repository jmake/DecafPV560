#include "BP_LevelBMainActor.h"
#include "Kismet/GameplayStatics.h"


//----------------------------------------------------------------------| |--//
static void BlockBackGroud(UMainWidget* mainWidgetInstance, APlayerController* playerController)
{
	//FInputModeUIOnly InputMode; // ?? 
	FInputModeGameAndUI InputMode; // ?? 
	InputMode.SetWidgetToFocus(mainWidgetInstance->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

	playerController->SetInputMode(InputMode);
	playerController->bShowMouseCursor = true;
	playerController->bEnableClickEvents = true;
	playerController->bEnableMouseOverEvents = true;
}


//----------------------------------------------------------------------| |--//
static TSubclassOf<UMainWidget> UMainWidgetFinder(std::string className)
{
    static ConstructorHelpers::FClassFinder<UMainWidget> widgetBPClass( *FString(className.c_str()) );
    if (widgetBPClass.Succeeded()) return widgetBPClass.Class; 

    return nullptr;
}


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
static void CreateVtkSurface(AMyActor1* vtkInstance) 
{
	if(vtkInstance == nullptr) return ; 

	//vtkInstance->CreateSourface(0.5, [this](){ WidgetCreate(); }); 
	vtkInstance->CreateSourface(0.5, [](){}); 
	AsyncTask([](){}, [](){}); 
}


//----------------------------------------------------------------------| |--//
static void CreateVtkTexture(AMyActor1* vtkInstance, double value = 0.5) 
{
	if(vtkInstance == nullptr) return ; 

	//double       value = 0.5;
	double    axial[9] = {1, 0,  0, 0, 1,  0, 0, 0, 1}; 
	double   center[3] = {0.5, 0.5, value};

	APixelTextureDisplay* textureActor = nullptr; 
	vtkInstance->CreateTextureAsync(nullptr, axial, center, 
	[](APixelTextureDisplay* actor){

		UTexture2D* texture2D = nullptr; 
		texture2D = actor->DynamicTexture; 

		if(texture2D == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("[CreateTextureAsync] fails !!") );
			return ;
		}

		UE_LOG(LogTemp, Warning, TEXT("[CreateTextureAsync] Size: %d X %d"), texture2D->GetSizeX(), texture2D->GetSizeY());

		//axialTexture = texture2D;
		UE_LOG(LogTemp, Warning, TEXT("[CreateTextureAsync] Applied !!") );
	}); 

}


//----------------------------------------------------------------------| |--//
static void ImageWidgetModify(
	USliderWidget* radialSliderWidget, 
	APlayerController* playerController,
	UTexture2D* axialTexture
)
{
	//
	// Create a material (M_Runtime2DTexture) 
	//	> Material domain (User interface) 
	//	> Add 'Vector Parameter' (BaseColor)
	//	> Add 'Texture Sample Parameter2D' (Param2D)	
	//
	if(radialSliderWidget == nullptr) 
	{
		UE_LOG(LogTemp, Warning, TEXT("[LiveRenderApply] 'radialSliderWidget' fails!!") ); 
		return ; 
	}

	FString ImageIdPath = TEXT("ImageId"); 
	FString Param2DName = TEXT("Param2D"); 
	FString MaterialPath = TEXT("/Game/SpicyTech/LevelB/M_Runtime2DTexture.M_Runtime2DTexture");

	radialSliderWidget->SetLiveRender(playerController, MaterialPath, Param2DName); 

	UWidget* FoundWidget = radialSliderWidget->GetWidgetFromName(*ImageIdPath); 
	if (FoundWidget == nullptr) return ;

	UImage* ImageWidget = Cast<UImage>(FoundWidget); 
	if (ImageWidget == nullptr) return ;

	ImageWidget->SetColorAndOpacity( FLinearColor(1.0f, 1.0f, 1.0f, 1.0f) ); 

	if(radialSliderWidget->dynamicMaterial) ImageWidget->SetBrushFromMaterial( radialSliderWidget->dynamicMaterial );

	if(axialTexture == nullptr) UE_LOG(LogTemp, Warning, TEXT("[LiveRenderApply] 'axialTexture' fails!!") ); 
	ImageWidget->SetBrushFromTexture( axialTexture );
}


//----------------------------------------------------------------------| |--//
static void SliderWidgetModify(
								std::string slideName, 
								TMap<FString, USliderWidget*> sliderWidgets, 
								AMyActor1* vtkInstance
							)
{
	FString key(slideName.c_str()); 
	USliderWidget** Found = sliderWidgets.Find(*key); 

	USliderWidget* sliderWidget = *Found;

	if(sliderWidget == nullptr) 
	{
		UE_LOG(LogTemp, Warning, TEXT("[LiveRenderApply] 'sliderWidget' fails!!") ); 
		return ; 
	}

	sliderWidget->callBackTask = [](float value)
	{
	    UE_LOG(LogTemp, Warning, TEXT("[callBackTask] value : %f"), value);
		return value;
	}; 

	sliderWidget->callBackAfter = [vtkInstance](float response)
	{
		CreateVtkTexture(vtkInstance, response); 
	    UE_LOG(LogTemp, Warning, TEXT("[callBackAfter] response : %f"), response);
	}; 
}


//----------------------------------------------------------------------| |--//
//----------------------------------------------------------------------| |--//
ABP_LevelBMainActor::ABP_LevelBMainActor()
{
	PrimaryActorTick.bCanEverTick = true;

	std::string filename = "/Game/SpicyTech/LevelB/WBP_LevelB"; 
	MainWidget = UMainWidgetFinder(filename); 
}


//----------------------------------------------------------------------| |--//
//----------------------------------------------------------------------| |--//
void ABP_LevelBMainActor::BeginPlay()
{
	Super::BeginPlay();

	// X.1) Create 'UMainWidget'
	playerController = UGameplayStatics::GetPlayerController(this, 0);

	UMainWidget* mainInstance = nullptr; 
	mainInstance = CreateLayout(); 

	// X.1) 
	vtkInstance = AddVtkActor(playerController->GetWorld(), FName(TEXT("VTK_Instance")), this); 
	FString fname("F:/z2025_1/Dicom/DecafPV560/domain.vti"); 
	vtkInstance->LoadVtiFile(fname); 

//	CreateVtkSurface(vtkInstance); 
//	CreateVtkTexture(vtkInstance); 
//	axialTexture = vtkInstance->CreateAxial(0.5); 

	// X.1) 
	//LiveRenderUpdate(); 
	LiveRenderApply("TopRight"); 
	SliderWidgetModify("TopRight", SliderWidgets2, vtkInstance); 
}


//----------------------------------------------------------------------| |--//
//----------------------------------------------------------------------| |--//
void ABP_LevelBMainActor::LiveRenderApply(USliderWidget* radialSliderWidget)
{
	//
	// Create a material (M_Runtime2DTexture) 
	//	> Material domain (User interface) 
	//	> Add 'Vector Parameter' (BaseColor)
	//	> Add 'Texture Sample Parameter2D' (Param2D)	
	//
	if(radialSliderWidget == nullptr) 
	{
		UE_LOG(LogTemp, Warning, TEXT("[LiveRenderApply] 'radialSliderWidget' fails!!") ); 
		return ; 
	}

	FString ImageIdPath = TEXT("ImageId"); 
	FString Param2DName = TEXT("Param2D"); 
	FString MaterialPath = TEXT("/Game/SpicyTech/LevelB/M_Runtime2DTexture.M_Runtime2DTexture");

	radialSliderWidget->SetLiveRender(playerController, MaterialPath, Param2DName); 

	UWidget* FoundWidget = radialSliderWidget->GetWidgetFromName(*ImageIdPath); 
	if (FoundWidget == nullptr) return ;

	UImage* ImageWidget = Cast<UImage>(FoundWidget); 
	if (ImageWidget == nullptr) return ;

	ImageWidget->SetColorAndOpacity( FLinearColor(1.0f, 1.0f, 1.0f, 1.0f) ); 

	if(radialSliderWidget->dynamicMaterial) ImageWidget->SetBrushFromMaterial( radialSliderWidget->dynamicMaterial );

	if(axialTexture == nullptr) UE_LOG(LogTemp, Warning, TEXT("[LiveRenderApply] 'axialTexture' fails!!") ); 
	ImageWidget->SetBrushFromTexture( axialTexture );
}


void ABP_LevelBMainActor::LiveRenderApply(std::string slideName)
{
	FString key(slideName.c_str()); 

	if (USliderWidget** Found = SliderWidgets2.Find(*key))
	{
//LiveRenderApply( *Found ); //USliderWidget* slider = *Found;
ImageWidgetModify(*Found, playerController, axialTexture); 
		UE_LOG(LogTemp, Warning, TEXT("[LiveRenderApply] key '%s' Done !!"), *key);
	}
	else 
	{
		UE_LOG(LogTemp, Warning, TEXT("[LiveRenderApply] key '%s' Fails !!"), *key);
		return ; 
	}
}

void ABP_LevelBMainActor::LiveRenderUpdate()
{
/*
	for (auto& elem : SliderWidgets2)
	{
		FString key = elem.Key; 
		USliderWidget* slider = elem.Value; 

		UE_LOG(LogTemp, Warning, TEXT("[LiveRenderUpdate] Key: %s"), *key);
		if(slider)
		{
			LiveRenderApply(slider); 
		}
	}
*/
	for (auto& elem : SliderWidgets2)
	{
		LiveRenderApply( TCHAR_TO_UTF8(*elem.Key) );  
	} 

	UE_LOG(LogTemp, Warning, TEXT("[LiveRenderUpdate] Done !!") ); 
} 

//----------------------------------------------------------------------| |--//
//----------------------------------------------------------------------| |--//
UMainWidget* ABP_LevelBMainActor::CreateLayout()
{
	UMainWidget* mainWidgetInstance = nullptr;	

	mainWidgetInstance = CreateWidget<UMainWidget>(playerController, UMainWidget::StaticClass()); // From scratch 
	//if(MainWidget) mainWidgetInstance = CreateWidget<UMainWidget>(playerController, MainWidget); // From 'Blue Print Widget Class'

	LiveRenderCreate(mainWidgetInstance); 

	mainWidgetInstance->AddToViewport(); //(zorder);
	//BlockBackGroud(mainWidgetInstance); // Blocker 

	UE_LOG(LogTemp, Warning, TEXT("[CreateLayout] done!") );
	return mainWidgetInstance; 
} 


//----------------------------------------------------------------------| |--//
void ABP_LevelBMainActor::LiveRenderCreate(UMainWidget* mainWidgetInstance) 
{
    if (mainWidgetInstance == nullptr) return ; 

	// 'Content Drawer' > 'Widget Blueprint' > 'Copy Reference' 
	FString SliderBPPath = TEXT("/Game/SpicyTech/LevelB/WBP_RadialSlider3.WBP_RadialSlider3_C"); 
	
	FLinearColor Cyan(0.f, 1.f, 1.f, 1.f);

	// Top-left
	UCanvasPanel* TopLeft    = UPipelineTools::CreateCanvasPanel(mainWidgetInstance, "TopLeft", 0.0f, 0.0f, 0.5f, 0.5f, FLinearColor::Red); 
	//UTextBlock* MyText1 = UPipelineTools::AddTextToCanvas(TopLeft, "MyText1");
	USliderWidget* SliderTL = AddSliderWidgetToCanvas(TopLeft, SliderBPPath, playerController);
//	LiveRenderApply( SliderTL ); 
	SliderWidgets2.Add(TEXT("TopLeft"), SliderTL);

	// Top-right
	UCanvasPanel* TopRight    = UPipelineTools::CreateCanvasPanel(mainWidgetInstance, "TopRight", 0.5f, 0.0f, 1.0f, 0.5f, FLinearColor::White); 
	USliderWidget* SliderTR = AddSliderWidgetToCanvas(TopRight, SliderBPPath, playerController);
//	LiveRenderApply( SliderTR ); 
	SliderWidgets2.Add(TEXT("TopRight"), SliderTR);

	// Bottom-left
	UCanvasPanel* BottomLeft = UPipelineTools::CreateCanvasPanel(mainWidgetInstance, "BottomLeft", 0.0f, 0.5f, 0.5f, 1.0f, FLinearColor::Yellow);  
	USliderWidget* SliderBL = AddSliderWidgetToCanvas(BottomLeft, SliderBPPath, playerController);
//	LiveRenderApply( SliderBL ); 
	SliderWidgets2.Add(TEXT("BottomLeft"), SliderBL);

	// Bottom-right
	UCanvasPanel* BottomRight = UPipelineTools::CreateCanvasPanel(mainWidgetInstance, "BottomRight", 0.5f, 0.5f, 1.0f, 1.0f, FLinearColor::Transparent); 
	UButton* ButtonBR = UPipelineTools::AddButtonToCanvas(BottomRight, "ButtonBR", 0.1, 0.1, 0.2, 0.2); 
	//USliderWidget* SliderBR = AddSliderWidgetToCanvas(BottomRight, SliderBPPath, playerController);
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

	for (USliderWidget* Slider : SliderWidgets1) Slider->UpdateLiveRender(CamLoc, CamRot);

	for (auto& elem : SliderWidgets2)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Key: %s"), *elem.Key);
		if (elem.Value)
		{
			elem.Value->UpdateLiveRender(CamLoc, CamRot);
		}
	}

}


//----------------------------------------------------------------------| |--//
//----------------------------------------------------------------------| |--//