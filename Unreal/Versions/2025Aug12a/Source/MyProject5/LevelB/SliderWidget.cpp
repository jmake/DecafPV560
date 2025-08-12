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

    textManager = new TextManager(); 
    textManager->Construct(TextBlockId); 

    buttonManager = new ButtonManager(); 
    buttonManager->Construct(ButtonId); 
    buttonManager->OnClicked()->AddDynamic(this, &USliderWidget::OnChangedButton);
    this->OnChangedButton(); 

    sliderManager = new SliderManager(); 
    sliderManager->Construct(SliderId); 
    sliderManager->OnValueChanged()->AddDynamic(this, &USliderWidget::OnChangedSlider);
    //SliderId->OnValueChanged.AddDynamic(this, &USliderWidget::OnChangedSlider);
    this->OnChangedSlider( SliderId->GetValue() );
}

/*
void USliderWidget::OnChangedSlider(float input) 
{
    if(SliderId == nullptr) return;

    textManager->Before( std::to_string(input) ); 

    UE_LOG(LogTemp, Warning, TEXT("[SliderConstruct] Handle Value : %f"), input);    
}
*/

void USliderWidget::OnChangedButton()
{

    UE_LOG(LogTemp, Warning, TEXT("[USliderWidget] OnChangedButton!!") );    
}


void USliderWidget::OnChangedSlider(float input) 
{
    sliderManager->Before(); 
    textManager->Before( std::to_string(sliderManager->GetValue()) ); 

    AsyncTaskApply(); 
}


void USliderWidget::TaskApply()
{
    textManager->During(); 
    sliderManager->During(); 
}


void USliderWidget::TaskEnd()
{
    textManager->After(); 
    sliderManager->After(); 
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