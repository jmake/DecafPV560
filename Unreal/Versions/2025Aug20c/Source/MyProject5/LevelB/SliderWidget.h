#pragma once

#include "VtkInterface.h"

#include "PipelineTools.h"

#include "EngineUtils.h" 
#include "Engine/World.h"
#include "Camera/CameraActor.h"
#include "GameFramework/PlayerController.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

#include "Engine/TextureRenderTarget2D.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"


#include "Components/Image.h"
#include "Components/Button.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/RadialSlider.h" // -> Add 'AdvancedWidgets' to Source\MyProject5\MyProject5.Build.cs


#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SliderWidget.generated.h" // Should always be the last


//DECLARE_MULTICAST_DELEGATE_OneParam(FOnValueChangedNative, float);

class ImageManager
{
private : 
	UImage* widget = nullptr; 

	ASceneCapture2D* captureActor = nullptr; 
	UMaterialInstanceDynamic* dynamicMaterial = nullptr;

public : 
	void Construct(UImage* image)
	{
		if(image == nullptr) return;  

		widget = image; 
		widget->SetVisibility(ESlateVisibility::Visible);
        widget->SetIsEnabled(true);
		widget->SetColorAndOpacity( FLinearColor(0.0f, 1.0f, 0.0f, 1.0f) );
	}


	void Before(std::string message) 
	{
	    if(widget == nullptr) return;  
	} 

	void During()
	{
	}

	void After() 
	{
	    if(widget == nullptr) return;  
	} 	


	void SetImage() 
	{
		static ConstructorHelpers::FObjectFinder<UTexture2D> TextureFinder(TEXT("/Game/Path/To/YourTexture"));
		if (TextureFinder.Succeeded())
		{
			widget->SetBrushFromTexture(TextureFinder.Object);
		}	
	}
}; 



class ButtonManager
{
private : 
	UButton* widget = nullptr; 

public : 
	void Construct(UButton* button)
	{
		if(button == nullptr) return;  

		widget = button; 
		widget->SetVisibility(ESlateVisibility::Visible);
        widget->SetIsEnabled(true);
	}

	FOnButtonClickedEvent* OnClicked()
	{
		if (!widget) return nullptr;

		return &(widget->OnClicked);
	}	

	void SetState(bool state)
	{
		return widget->SetIsEnabled(state);
	}
}; 


class TextManager
{
private : 
	std::string text; 
	UTextBlock* widget = nullptr; 

	void String2Text(std::string message) 
	{
		if(widget == nullptr) return;  

		FString UnrealString(message.c_str());
		FText UnrealText = FText::FromString(UnrealString);

		widget->SetText(UnrealText);
	}	

public : 
	void Construct(UTextBlock* textBlock)
	{
		if(textBlock == nullptr) return;  

		widget = textBlock; 
		widget->SetIsEnabled(true);
		widget->SetVisibility(ESlateVisibility::Hidden); // Collapsed  Hidden Visible
		String2Text("Construct..."); 
	}

	void SetText(std::string message) 
	{
	    if(widget == nullptr) return;  

		widget->SetVisibility(ESlateVisibility::Visible);
		String2Text(message); 
	} 

	void After() 
	{
	    if(widget == nullptr) return;  

		widget->SetVisibility(ESlateVisibility::Hidden);
		String2Text("After..."); 
	} 	


}; 


class SliderManager
{
	// -> Add 'AdvancedWidgets' to Source\MyProject5\MyProject5.Build.cs
private : 
	float value = 0.0; 
	URadialSlider* slider = nullptr; 

public : 
    //FOnValueChangedNative OnValueChangedNative; // member delegate

	void Construct(URadialSlider* widget)
	{
		if(widget == nullptr) return;  

		slider = widget; 
        slider->SetSliderHandleStartAngle(0.0f);
        slider->SetSliderHandleEndAngle(360.0f);
        slider->SetStepSize(0.05); 
        slider->SetValue(0.33); 

        value = slider->GetNormalizedSliderHandlePosition();
	}

/*
	void SetSliderCallback(TFunction<void(float)> task, TFunction<void(float)> after)
    {
        callBackTask = task;
        callBackAfter = after;

        if (slider)
        {
//            slider->OnValueChanged.AddDynamic(this, &SliderManager::HandleSliderChanged);
        }
    }


    UFUNCTION()
    void HandleSliderChanged(float Value)
    {
//        if (callBackTask == nullptr) return ; 
//        if (callBackAfter == nullptr) return ; 

//		UPipelineTools::Set AsyncFuncs<float>(callBackTask, callBackAfter); 
    }	
*/

	FOnFloatValueChangedEvent* OnValueChanged()
	{
		if (!slider) return nullptr;

		return &(slider->OnValueChanged);
	}


	void Deactivate() 
	{
	    if(slider == nullptr) return;  

		slider->SetIsEnabled(false);
		slider->SetVisibility(ESlateVisibility::Collapsed); // Collapsed  Hidden 

        value = slider->GetNormalizedSliderHandlePosition();
    	UE_LOG(LogTemp, Warning, TEXT("[SliderManager] Before value : %f"), value);
	}

	void Task() 
	{
    	FPlatformProcess::Sleep(1.0f); 

    	UE_LOG(LogTemp, Warning, TEXT("[SliderManager] During value : %f"), value);
	} 

	void Activate() 
	{
	    if(slider == nullptr) return;  

		slider->SetVisibility(ESlateVisibility::Visible);
		slider->SetIsEnabled(true);

        value = slider->GetNormalizedSliderHandlePosition();
		UE_LOG(LogTemp, Warning, TEXT("[SliderManager] After value : %f"), value);		
	}

	float GetValue() 
	{
		return value; 
	}

}; 



UCLASS(Blueprintable)
class MYPROJECT5_API USliderWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

//	float value = 0.5; 

    UFUNCTION() // Must be a UFUNCTION to bind to delegate
    void OnChangedSlider(float value); 


    UFUNCTION() // Expose it to the Unreal reflection 
    void ButtonQuitGame(); 

	void AsyncTaskApply(); 
	void AsyncTaskComplete(const FString& Result); 

    // UPROPERTY(BlueprintReadOnly, Category = "Async") // optional  
    FString LastAsyncResult;    

	TextManager* textManager = nullptr;
	ImageManager* imageManager = nullptr;
	ButtonManager* buttonManager = nullptr;
	SliderManager* sliderManager = nullptr; 

public : 
	ASceneCapture2D* captureActor = nullptr; 
	UMaterialInstanceDynamic* dynamicMaterial = nullptr;

	void SetLiveRender(APlayerController* playerController, FString materialPath, FString param2DName); 
	void UpdateLiveRender(FVector CamLoc, FRotator CamRot);  

	void TaskEnd(); 
	void TaskApply(); 

	// This assumes that the UMG Blueprint contains a Slider widget named 'SliderId' 
	//UPROPERTY(meta = (BindWidget))
	//class USlider* SliderId = nullptr;
	//
	// Add 'AdvancedWidgets' to Source\MyProject5\MyProject5.Build.cs
	UPROPERTY(meta = (BindWidget))
	class URadialSlider* SliderId = nullptr;

	// This assumes that the UMG Blueprint contains a Slider widget named 'TextBlockId' 
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TextBlockId = nullptr;

    UPROPERTY(meta = (BindWidget)) 
    class UButton* ButtonId = nullptr;

    UPROPERTY(meta = (BindWidget)) 
	class UImage* ImageId = nullptr; 

public : 
	/*
	//TFunction< float( void) > callBackTask = [](){return 0.0;}; 
	//TFunction<  void(float) > callBackAfter = [](float){}; 
	TFunction< float( void) > callBackTask = nullptr; 
	TFunction<  void(float) > callBackAfter = nullptr; 
	*/
	/*
	TFunction<float(float)> callBackTask = nullptr; 
	TFunction< void(float)> callBackAfter = nullptr; 
	UFUNCTION() void HandleSliderChanged(float value); 
	*/

public : 

//TFunction< void(float)> CallBack = nullptr; 
//TMap<FString, TFunction< void(float)> > SliderCallBacks;

UFUNCTION() void HandleCallBack(float value); 

TFunction<AnatomicalProperties(float)> callBackTask = nullptr; 
TFunction< void(AnatomicalProperties)> callBackAfter = nullptr; 

};