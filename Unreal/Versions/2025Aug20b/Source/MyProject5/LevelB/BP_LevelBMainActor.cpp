#include "BP_LevelBMainActor.h"
#include "Kismet/GameplayStatics.h"


//----------------------------------------------------------------------| |--//
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
		CanvasSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f)); // Green (initial color)
		CanvasSlot->SetOffsets(FMargin(0.f));
	}

	//UPipelineTools::SetUIOnlyInputMode(SliderWidget, playerController); 
	return SliderWidget;
}


//----------------------------------------------------------------------| |--//
//----------------------------------------------------------------------| |--//
static void ModifyAxialTexture(
	std::string slideName, 
	TMap<FString, USliderWidget*> sliderWidgets, 
	APlayerController* playerController,
	UTexture2D* axialTexture
)
{
	FString key(slideName.c_str()); 
	USliderWidget** Found = sliderWidgets.Find(*key); 
	USliderWidget* sliderWidget = *Found;

	if(sliderWidget == nullptr) 
	{
		UE_LOG(LogTemp, Warning, TEXT("[ModifyAxialTexture] 'sliderWidget' Fails!!") ); 
		return ; 
	}

	// Create a material (M_Runtime2DTexture) then  
	//	> Material domain (User interface) 
	//	> Add 'Vector Parameter' (BaseColor)
	//	> Add 'Texture Sample Parameter2D' (Param2D)
	FString ImageIdPath = TEXT("ImageId"); 
	FString Param2DName = TEXT("Param2D"); 
	FString MaterialPath = TEXT("/Game/SpicyTech/LevelB/M_Runtime2DTexture.M_Runtime2DTexture");

	sliderWidget->SetLiveRender(playerController, MaterialPath, Param2DName); 

	UWidget* FoundWidget = sliderWidget->GetWidgetFromName(*ImageIdPath); 
	if (FoundWidget == nullptr) return ;

	UImage* ImageWidget = Cast<UImage>(FoundWidget); 
	if (ImageWidget == nullptr) return ;

	ImageWidget->SetColorAndOpacity( FLinearColor(1.0f, 1.0f, 1.0f, 1.0f) ); 

	if(sliderWidget->dynamicMaterial) ImageWidget->SetBrushFromMaterial( sliderWidget->dynamicMaterial );

	if(axialTexture == nullptr) UE_LOG(LogTemp, Error, TEXT("[ModifyAxialTexture] 'axialTexture = nullptr' Fails!!") ); 

	axialTexture->UpdateResource();
	ImageWidget->SetBrushFromTexture( axialTexture );
	
	UE_LOG(LogTemp, Error, TEXT("[ModifyAxialTexture] '%s' Done !!"), *key);
}


//----------------------------------------------------------------------| |--//
void CreatePixelTextureDisplay2(
	std::string slideName, 
	TMap<FString, USliderWidget*> sliderWidgets, 
	APixelTextureDisplay*& actor, 
	APlayerController* playerController,
	AnatomicalProperties result
)
{
	if(GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("[CreatePixelTextureDisplay] ... !!"));

	vtkImageData* cutter = result.vti_ptr; 
	if(cutter == nullptr) 
	{
		UE_LOG(LogTemp, Error, TEXT("[CreatePixelTextureDisplay] 'cutter' fails !!") );
		return ; 
	} 

	int numberOfCells = cutter->GetNumberOfCells(); 

	// SEE : https://gist.github.com/jmake/72bc2c549371d8b00804487120027099 
	//APixelTextureDisplay*& actor = this->dummy; 

	if (actor && !actor->IsActorBeingDestroyed())
	{
		actor->Destroy();
		actor = nullptr; 
		UE_LOG(LogTemp, Error, TEXT("[CreatePixelTextureDisplay] Removed! ") );
	}

	UTexture2D* texture2D = AnatomicalBuffer2UTexture2D(result, playerController->GetWorld(), actor); 
	ModifyAxialTexture(slideName, sliderWidgets, playerController, texture2D); 

	UE_LOG(LogTemp, Warning, TEXT("[CreatePixelTextureDisplay] width : %d, height : %d numberOfCells: %d"), result.width, result.height, numberOfCells);
	if(GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("[CreatePixelTextureDisplay] Done !!"));
} 



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

	//CuttersRsnaCervicalSpineFractureDetection(); 

} 


void ABP_LevelBMainActor::CuttersRsnaCervicalSpineFractureDetection() 
{
	// X.1) Create 'UMainWidget'
	playerController = UGameplayStatics::GetPlayerController(this, 0);

	UMainWidget* mainInstance = nullptr; 
	mainInstance = CreateLayout(); 

	// X.1) 
	//std::string vtiFile = "F:/z2025_1/Dicom/DecafPV560/domain.vti"; 
	std::string vtiFile = "F:/Download/DecafPV560/Data/rsnaCervicalSpineFractureDetection.vti"; 
	std::vector<double> range; 
	vtkImageData* domain = nullptr; 
	LoadVtiFile(vtiFile, domain, range); 

	// Cut 1
	CreateAnatomicalBufferAsync(domain, "TopLeft", this->dummy);  	
	CreateAnatomicalBufferAsync(domain, "TopRight", this->dummy);  	
	CreateAnatomicalBufferAsync(domain, "BottomLeft", this->dummy);  	

	// Surface 
	vtkPolyData* vtp = nullptr; 
	SurfaceProperties buffer2 = {};
	Vti2SurfaceBuffer(
		domain, 
		0.5, range[0], range[1],
		vtp, 
		buffer2.points,
		buffer2.n_rows, 
		buffer2.n_cols,
		buffer2.triangles,
		buffer2.n_triangles
	); 	

	UE_LOG(LogTemp, Warning, TEXT("[Vti2AnatomicalBuffer] n_triangles : %d n_rows : %d n_cols : %d"), buffer2.n_triangles, buffer2.n_rows, buffer2.n_cols);
}

//----------------------------------------------------------------------| |--//
//----------------------------------------------------------------------| |--//
void ABP_LevelBMainActor::CreateAnatomicalBufferAsync(
														vtkImageData* domain, 
														std::string slideName, 
														//double* anatomicalView, float x0, float y0, float z0, 
														APixelTextureDisplay*& actor
													)  
{
	FString key(slideName.c_str()); 
	USliderWidget** Found = SliderWidgets2.Find(*key); 

	USliderWidget* sliderWidget = *Found;

	sliderWidget->callBackTask = [key, domain, slideName](float value)
	{

		double* anatomicalView = nullptr;
		double sliceX = 0.5, sliceY = 0.5, sliceZ = 0.5;

		if (slideName == "TopRight") {
			sliceZ = value;
			anatomicalView = AXIAL;
		} 
		else if (slideName == "TopLeft") {
			sliceY = value; 
			anatomicalView = CORONAL;
		} 
		else if (slideName == "BottomLeft") {
			sliceY = value;
			anatomicalView = SAGITTAL;
		}

	    AnatomicalProperties buffer = {};
        buffer.data = Vti2AnatomicalBuffer(
                                            domain, 
                                            anatomicalView, 
                                            sliceX, sliceY, sliceZ, 
                                            buffer.vti_ptr, 
                                            buffer.width, 
                                            buffer.height, 
                                            buffer.spacingX, 
                                            buffer.spacingY 
                                        ); 

		UE_LOG(LogTemp, Error, TEXT("[callBackTask] %s(%f) Done !!"), *key, value);
		return buffer; 
	}; 

	sliderWidget->callBackAfter = [this,&actor,slideName](AnatomicalProperties result)
	{
		//this->CreatePixelTextureDisplay(slideName, dummy, result); 
		CreatePixelTextureDisplay2(slideName, SliderWidgets2, actor, playerController, result); 
	}; 
}


//----------------------------------------------------------------------| |--//
//----------------------------------------------------------------------| |--//
/*
void ABP_LevelBMainActor::CreatePixelTextureDisplay(std::string slideName, APixelTextureDisplay*& actor, AnatomicalProperties result)
{
	if(GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("[CreatePixelTextureDisplay] ... !!"));

	vtkImageData* cutter = result.vti_ptr; 
	if(cutter == nullptr) 
	{
		UE_LOG(LogTemp, Error, TEXT("[CreatePixelTextureDisplay] 'cutter' fails !!") );
		return ; 
	} 

	int numberOfCells = cutter->GetNumberOfCells(); 

	// SEE : https://gist.github.com/jmake/72bc2c549371d8b00804487120027099 
	//APixelTextureDisplay*& actor = this->dummy; 

	if (actor && !actor->IsActorBeingDestroyed())
	{
		actor->Destroy();
		actor = nullptr; 
		UE_LOG(LogTemp, Error, TEXT("[CreatePixelTextureDisplay] Removed! ") );
	}

	UTexture2D* texture2D = AnatomicalBuffer2UTexture2D(result, playerController->GetWorld(), actor); 
	ModifyAxialTexture(slideName, SliderWidgets2, playerController, texture2D); 

	UE_LOG(LogTemp, Warning, TEXT("[CreatePixelTextureDisplay] width : %d, height : %d numberOfCells: %d"), result.width, result.height, numberOfCells);
	if(GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("[CreatePixelTextureDisplay] Done !!"));
} 
*/

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
	SliderWidgets2.Add(TEXT("TopLeft"), SliderTL);

	// Top-right
	UCanvasPanel* TopRight  = UPipelineTools::CreateCanvasPanel(mainWidgetInstance, "TopRight", 0.5f, 0.0f, 1.0f, 0.5f, FLinearColor::White); 
	USliderWidget* SliderTR = AddSliderWidgetToCanvas(TopRight, SliderBPPath, playerController);
	SliderWidgets2.Add(TEXT("TopRight"), SliderTR);

	// Bottom-left
	UCanvasPanel* BottomLeft = UPipelineTools::CreateCanvasPanel(mainWidgetInstance, "BottomLeft", 0.0f, 0.5f, 0.5f, 1.0f, FLinearColor::Yellow);  
	USliderWidget* SliderBL = AddSliderWidgetToCanvas(BottomLeft, SliderBPPath, playerController);
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