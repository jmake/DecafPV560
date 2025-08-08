// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/Button.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/RadialSlider.h" // -> Add 'AdvancedWidgets' to Source\MyProject5\MyProject5.Build.cs

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SliderWidget.generated.h" // Should always be the last



class TextManager
{
private : 
	std::string text; 
	UTextBlock* widget = nullptr; 

public : 
	void Construct(UTextBlock* textBlock)
	{
		if(textBlock == nullptr) return;  

		widget = textBlock; 
		widget->SetIsEnabled(true);
		widget->SetVisibility(ESlateVisibility::Hidden); // Collapsed  Hidden Visible
		String2Text("Construct..."); 
	}

	void Before(std::string message) 
	{
	    if(widget == nullptr) return;  

		widget->SetVisibility(ESlateVisibility::Visible);
		String2Text(message); 
	} 

	void During() 
	{

	} 

	void After() 
	{
	    if(widget == nullptr) return;  

		widget->SetVisibility(ESlateVisibility::Hidden);
		String2Text("After..."); 
	} 	

	void String2Text(std::string message) 
	{
		if(widget == nullptr) return;  

		FString UnrealString(message.c_str());
		FText UnrealText = FText::FromString(UnrealString);

		widget->SetText(UnrealText);
	}	
}; 


class SliderManager
{
	// -> Add 'AdvancedWidgets' to Source\MyProject5\MyProject5.Build.cs
private : 
	float value = 0.0; 
	URadialSlider* slider = nullptr; 

public : 
	void Construct(URadialSlider* widget)
	{
		if(widget == nullptr) return;  

		slider = widget; 
        slider->SetSliderHandleStartAngle(0.0f);
        slider->SetSliderHandleEndAngle(360.0f);
        slider->SetStepSize(0.05); 
        slider->SetValue(0.33); 

        value = slider->GetNormalizedSliderHandlePosition();
        //UE_LOG(LogTemp, Warning, TEXT("[SliderManager] Handle Position: %f"), value);
	}

	void Before() 
	{
	    if(slider == nullptr) return;  

		slider->SetIsEnabled(false);
		slider->SetVisibility(ESlateVisibility::Collapsed); // Collapsed  Hidden 

        value = slider->GetNormalizedSliderHandlePosition();
    	UE_LOG(LogTemp, Warning, TEXT("[SliderManager] Before value : %f"), value);
	}

	void During() 
	{
//		if(slider == nullptr) return;  

    	FPlatformProcess::Sleep(1.0f); 

    	UE_LOG(LogTemp, Warning, TEXT("[SliderManager] During value : %f"), value);
	} 

	void After() 
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

	float value = 0.5; 

    UFUNCTION() // Must be a UFUNCTION to bind to delegate
    void OnChanged(float value); 

	void AsyncTaskApply(); 
	void AsyncTaskComplete(const FString& Result); 

    // UPROPERTY(BlueprintReadOnly, Category = "Async") // optional  
    FString LastAsyncResult;    

	TextManager* textManager = nullptr;
	SliderManager* sliderManager = nullptr; 

public : 
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

};