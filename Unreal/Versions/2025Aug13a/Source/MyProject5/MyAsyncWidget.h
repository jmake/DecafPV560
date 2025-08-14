#pragma once

#include "Async/Async.h"
#include "Async/Future.h"

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyAsyncWidget.generated.h" // Should always be the last


UCLASS(Blueprintable)
class MYPROJECT5_API UMyAsyncWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;

    UFUNCTION() // Must be a UFUNCTION to bind to delegate
    void OnClicked(); 

    void ButtonAsyncTaskInit();

    void ButtonAsyncTaskComplete(const FString& Result);

public : 
	// Find a widget inside the Blueprint widget tree named exactly 'StartButton'
	// WBP_AsyncWidget (Blueprint widget) -> Hierarchy -> [Button] -> Rename as 'StartButton' 
    UPROPERTY(meta = (BindWidget)) 
    class UButton* StartButton = nullptr;

    // UPROPERTY(BlueprintReadOnly, Category = "Async") // optional  
    FString LastAsyncResult;    
};
