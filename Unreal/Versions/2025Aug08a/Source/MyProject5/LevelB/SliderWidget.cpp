#include "SliderWidget.h"

#include <string>   // For std::string


void USliderWidget::NativeConstruct()
{
    Super::NativeConstruct();

    //if(StartButton) StartButton->OnClicked.AddDynamic(this, &USliderWidget::Apply);
/*
    if(SliderId) 
    {
        //SliderId->SetMinValue(0.0);
        //SliderId->SetMaxValue(1.0);

        SliderId->SetSliderHandleStartAngle(0.0f);
        SliderId->SetSliderHandleEndAngle(360.0f);
        SliderId->SetStepSize(0.05); 
        SliderId->SetValue(0.33); 

        float NormalizedValue = SliderId->GetNormalizedSliderHandlePosition();
        UE_LOG(LogTemp, Warning, TEXT("[USliderWidget] Handle Position: %f"), NormalizedValue);

        SliderId->OnValueChanged.AddDynamic(this, &USliderWidget::OnChanged);
    }
*/
    textManager = new TextManager(); 
    textManager->Construct(TextBlockId); 

    sliderManager = new SliderManager(); 
    sliderManager->Construct(SliderId); 
    
    SliderId->OnValueChanged.AddDynamic(this, &USliderWidget::OnChanged);
    this->OnChanged( SliderId->GetValue() );
}


void USliderWidget::OnChanged(float input) 
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