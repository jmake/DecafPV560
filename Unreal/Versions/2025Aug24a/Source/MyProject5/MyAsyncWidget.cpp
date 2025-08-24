#include "MyAsyncWidget.h"
#include "Components/Button.h" //  undefined type 'UButton'


void UMyAsyncWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (StartButton)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UMyAsyncWidget] StartButton is bound!"));
        StartButton->SetIsEnabled(true);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[UMyAsyncWidget] StartButton is NOT bound! Check name and 'Is Variable'"));
    }

    if (StartButton)
    {
        StartButton->OnClicked.AddDynamic(this, &UMyAsyncWidget::OnClicked);
    }
}


void UMyAsyncWidget::OnClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("[UMyAsyncWidget] Button was pressed"));

    ButtonAsyncTaskInit(); 
}


void UMyAsyncWidget::ButtonAsyncTaskInit()
{
    if (!StartButton) return;

    StartButton->SetIsEnabled(false);

    auto Promise = MakeShared<TPromise<FString>>();
    TFuture<FString> Future = Promise->GetFuture();

    Async(EAsyncExecution::Thread, [Promise]()
    {
        FPlatformProcess::Sleep(3.0f); // Simulate task
        Promise->SetValue(TEXT("Task Complete!"));
    });

    TFunction<void(const FString&)> Callback = nullptr; 
    Callback = [this](const FString& Result) { ButtonAsyncTaskComplete(Result); };

    Future.Next(Callback);
}


void UMyAsyncWidget::ButtonAsyncTaskComplete(const FString& Result)
{
    AsyncTask(ENamedThreads::GameThread, [this, Result]()
    {
        if (StartButton)
        {
            StartButton->SetIsEnabled(true);
        }

        LastAsyncResult = Result;
        UE_LOG(LogTemp, Warning, TEXT("[UMyAsyncWidget] Async Task Result: %s"), *LastAsyncResult);
    });    
}


/*
void UMyAsyncWidget::ButtonAsyncTaskComplete(const FString& Result)
{    
    AsyncTask(ENamedThreads::GameThread, [this]()
    {
        if (StartButton)
        {
            StartButton->SetIsEnabled(true);
        }
    });  
}
*/