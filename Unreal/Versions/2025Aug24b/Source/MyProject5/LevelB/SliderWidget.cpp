#include "SliderWidget.h"

#include <string>   // For std::string


void SetCameraViewToImage(ACameraActor* CameraActor, UImage* ImageWidget)
{
    if (!CameraActor || !ImageWidget) return;

    // Find or create SceneCaptureComponent2D on the CameraActor
    USceneCaptureComponent2D* CaptureComp = CameraActor->FindComponentByClass<USceneCaptureComponent2D>();
    if (!CaptureComp)
    {
        CaptureComp = NewObject<USceneCaptureComponent2D>(CameraActor);
        CaptureComp->SetupAttachment(CameraActor->GetRootComponent());
        CaptureComp->RegisterComponent();
    }

    // Create Render Target if needed
    if (!CaptureComp->TextureTarget)
    {
        UTextureRenderTarget2D* RenderTarget = NewObject<UTextureRenderTarget2D>();
        RenderTarget->InitAutoFormat(1024, 768);
        RenderTarget->ClearColor = FLinearColor::Black;
        RenderTarget->UpdateResourceImmediate(true);
        CaptureComp->TextureTarget = RenderTarget;
    }

    // Force capture (optional)
    CaptureComp->CaptureScene();

    // Set the texture from render target to UImage widget
    //ImageWidget->SetBrushFromTexture(CaptureComp->TextureTarget);
}


ACameraActor* FindCameraByNameOrFallback(UWorld* World, const FString& CameraName)
{
    if (!World) return nullptr;

    // Search all CameraActors in the world
    for (TActorIterator<ACameraActor> It(World); It; ++It)
    {
        ACameraActor* Cam = *It;
        if (Cam && Cam->GetName() == CameraName)
        {
            return Cam;
        }
    }

    // If not found by name, fallback to active camera from PlayerController
    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (PC)
    {
        // Get current view target, could be a CameraActor or pawn with camera
        AActor* ViewTarget = PC->GetViewTarget();
        if (ViewTarget)
        {
            return Cast<ACameraActor>(ViewTarget);
        }
    }

    // Fallback failed, return nullptr
    return nullptr;
}


void USliderWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (!TextBlockId || !ButtonId || !SliderId || !ImageId) return;

    imageManager = new ImageManager(); 
    imageManager->Construct(ImageId); 

    textManager = new TextManager(); 
    textManager->Construct(TextBlockId); 

    buttonManager = new ButtonManager(); 
    buttonManager->Construct(ButtonId); 
    buttonManager->OnClicked()->AddDynamic(this, &USliderWidget::ButtonQuitGame);

    sliderManager = new SliderManager(); 
    sliderManager->Construct(SliderId); 

    sliderManager->OnValueChanged()->AddDynamic(this, &USliderWidget::HandleCallBack);
    this->HandleCallBack( SliderId->GetValue() );

/*
    sliderManager->OnValueChanged()->AddDynamic(this, &USliderWidget::OnChangedSlider);
    this->OnChangedSlider( SliderId->GetValue() );

    sliderManager->OnValueChanged()->AddDynamic(this, &USliderWidget::HandleSliderChanged); 
*/
}



void USliderWidget::HandleCallBack(float value) 
{
    sliderManager->Deactivate();  
    textManager->SetText( std::to_string(value) ); 
    buttonManager->SetState(false);

    std::function<AnatomicalProperties(void)> dummyTask = [this,value]()
    { 
//        if(callBackTask) callBackTask(value); 
//        else FPlatformProcess::Sleep(1.0f); 
//        return value;
        if(callBackTask) return callBackTask(value); 
        return AnatomicalProperties{}; 
    }; 

    std::function<void(AnatomicalProperties)> dummyAfter = [this](AnatomicalProperties response) 
    { 
        if(callBackAfter) callBackAfter(response); 

        textManager->After(); 
        sliderManager->Activate(); 
        buttonManager->SetState(true);
    }; 


    UPipelineTools::SetAsyncFuncs<AnatomicalProperties>(dummyTask, dummyAfter); 

/*
    if(CallBack) CallBack(value); 

	for (auto& elem : SliderCallBacks)
	{
		elem.Value( value ); 
		UE_LOG(LogTemp, Warning, TEXT("[SliderCallBacks] %s(%f)"), *elem.Key, value);
	}

    UE_LOG(LogTemp, Warning, TEXT("[HandleCallBack] Done !!") );
*/
} 


//void USliderWidget::HandleSliderChanged(float value) {
/*
    std::function<float(void)> dummyTask = [value](){return value;}; 
    std::function<void(float)> dummyAfter = [](float response) 
    {
	    UE_LOG(LogTemp, Warning, TEXT("[HandleSliderChanged] response : %f"), response);
    };

    if(callBackTask == nullptr) callBackTask = dummyTask; 
    if(callBackAfter == nullptr) callBackAfter = dummyAfter; 

    UPipelineTools::SetAsyncFuncs<float>(callBackTask, callBackAfter); 
*/
/*
    std::function<float(void)> dummyTask = [this,value]()
    { 
        if(callBackTask == nullptr) return value;

        return callBackTask(value); 
    }; 

    std::function<void(float)> dummyAfter = [this](float response) 
    { 
        if(callBackAfter == nullptr) return;
        callBackAfter(response); 
    }; 

    UPipelineTools::SetAsyncFuncs<float>(dummyTask, dummyAfter); 
*/
//}


void USliderWidget::SetLiveRender(APlayerController* playerController, FString materialPath, FString param2DName)
{
    UObject* owner = this; 

	UTextureRenderTarget2D* renderTarget = nullptr; 
	renderTarget = UPipelineTools::CreateRenderTarget(owner, 1024, 1024);

    ASceneCapture2D* sceneCapture = nullptr; 
	//UPipelineTools::CreateSceneCaptureActor(this, RenderTarget, FVector(0, 0, 200), FRotator(-30, 0, 0));
	captureActor = UPipelineTools::CreateSceneCaptureActor2(playerController, renderTarget);

    dynamicMaterial = UPipelineTools::CreateDynamicMaterial(materialPath, owner); 
    dynamicMaterial->SetTextureParameterValue(*param2DName, renderTarget);
}


void USliderWidget::UpdateLiveRender(FVector CamLoc, FRotator CamRot) 
{
	if(captureActor == nullptr) return ; 

	captureActor->SetActorLocationAndRotation(CamLoc, CamRot);
}



void USliderWidget::ButtonQuitGame()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("World not initialized — skip quit."));
        return;
    }

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC)
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayerController not initialized — skip quit."));
        return;
    }

    float DelaySeconds = 1.0; 
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(
        TimerHandle, 
        [World,PC](){ 
            UKismetSystemLibrary::QuitGame(World, PC, EQuitPreference::Quit, false);
        },
        DelaySeconds,
        false
    );

    textManager->SetText("Closing..."); 

    //APlayerController* PC = GetOwningPlayer();
    //UKismetSystemLibrary::QuitGame(GetWorld(), PC, EQuitPreference::Quit, false);
    UE_LOG(LogTemp, Warning, TEXT("[USliderWidget] ButtonQuitGame!!") );    
}


void USliderWidget::OnChangedSlider(float input) 
{
    sliderManager->Deactivate(); 
    textManager->SetText( std::to_string(sliderManager->GetValue()) ); 

    AsyncTaskApply(); 
}


void USliderWidget::TaskApply()
{
    sliderManager->Task(); 
}


void USliderWidget::TaskEnd()
{
    textManager->After(); 
    sliderManager->Activate(); 
}


void USliderWidget::AsyncTaskApply()
{
    auto Promise = MakeShared<TPromise<FString>>();
    TFuture<FString> Future = Promise->GetFuture();

    Async(EAsyncExecution::Thread, [this, Promise]()
    {
        this->TaskApply(); 
        Promise->SetValue(TEXT("Task Complete!"));
    });

    TFunction<void(const FString&)> Callback = nullptr;     
    Callback = [this](const FString& Result) { AsyncTaskComplete(Result); };

    Future.Next(Callback);
}


void USliderWidget::AsyncTaskComplete(const FString& Result)
{
    AsyncTask(ENamedThreads::GameThread, [this, Result]()
    {
        this->TaskEnd(); 

        LastAsyncResult = Result;
        UE_LOG(LogTemp, Warning, TEXT("[USliderWidget] Async Task Result: %s"), *LastAsyncResult);
    });    
}