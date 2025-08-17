// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

//#include "SliderWidget.h"
//#include "UObject/ConstructorHelpers.h"

#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/RadialSlider.h" // -> Add 'AdvancedWidgets' to Source\MyProject5\MyProject5.Build.cs

#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"

#include "Blueprint/WidgetTree.h" 

#include "string"
#include "MainWidget.h"

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Camera/CameraActor.h"
#include "Components/SceneCaptureComponent2D.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/SceneCapture2D.h"
#include "Components/Image.h"

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PipelineTools.generated.h" // last one!!


/*
	Displaying a live render from a SceneCaptureComponent2D in a UI widget image
*/
UCLASS()
class MYPROJECT5_API UPipelineTools : public UObject
{
	GENERATED_BODY()

public:
    // Step 1: Create Render Target
    UFUNCTION(BlueprintCallable, Category = "SpicyTools")
    static UTextureRenderTarget2D* CreateRenderTarget(UObject* WorldContextObject, int32 Width, int32 Height);

    // Step 2: Create Material Instance bound to RT
    UFUNCTION(BlueprintCallable, Category = "SpicyTools")
    static UMaterialInstanceDynamic* CreateMaterialForRT(UObject* WorldContextObject, UMaterialInterface* BaseMaterial, UTextureRenderTarget2D* RenderTarget);

    // Step 3: Create Scene Capture Actor and bind to RT
    UFUNCTION(BlueprintCallable, Category = "SpicyTools")
    static void CreateSceneCaptureActor(UObject* WorldContextObject, UTextureRenderTarget2D* RenderTarget, FVector Location, FRotator Rotation);

    UFUNCTION(BlueprintCallable, Category = "SpicyTools")
	static ASceneCapture2D* CreateSceneCaptureActor2(APlayerController* PC, UTextureRenderTarget2D* RenderTarget); 

    // Step 4: Create widget and apply Material or RT
    //UFUNCTION(BlueprintCallable, Category = "SpicyTools")
    //static UUserWidget* CreateWidgetWithImage(UObject* WorldContextObject, TSubclassOf<UUserWidget> WidgetClass, UTexture* Texture);
	//
    //UFUNCTION(BlueprintCallable, Category = "SpicyTools")
	//static UUserWidget* CreateWidgetWithMaterial(UObject* WorldContextObject, TSubclassOf<UUserWidget> WidgetClass, UMaterialInterface* Material); 


    UFUNCTION(BlueprintCallable, Category = "SpicyTools")
    static TSubclassOf<UMainWidget> UMainWidgetFinder(FString& filename) //(std::string filename)
    {
        //static ConstructorHelpers::FClassFinder<UMainWidget> WidgetBPClass(*FString(filename.c_str()));
        static ConstructorHelpers::FClassFinder<UMainWidget> WidgetBPClass(*filename);
        if (WidgetBPClass.Succeeded()) return WidgetBPClass.Class;

        return nullptr;
    }


    UFUNCTION(BlueprintCallable, Category = "SpicyTools")
    static UCanvasPanel* CreateCanvasPanel(
        UMainWidget* ParentWidget,
        const FString& CanvasNameString, 
        float AnchorLeft,
        float AnchorTop,
        float AnchorRight,
        float AnchorBottom,
        FLinearColor Color
    )
    {
        if (!ParentWidget || !ParentWidget->WidgetTree) return nullptr;

        UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(ParentWidget->WidgetTree->RootWidget);

        // If no root yet, create one
        if (!RootCanvas)
        {
            RootCanvas = ParentWidget->WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
            ParentWidget->WidgetTree->RootWidget = RootCanvas;
        }

        // Create child CanvasPanel
        FName CanvasName(*CanvasNameString);
        UCanvasPanel* ChildCanvas = NewObject<UCanvasPanel>(RootCanvas, CanvasName);
        if (!ChildCanvas) return nullptr;

        RootCanvas->AddChild(ChildCanvas);

        // Anchor the child Canvas
        if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(ChildCanvas->Slot))
        {
            CanvasSlot->SetAnchors(FAnchors(AnchorLeft, AnchorTop, AnchorRight, AnchorBottom));
            CanvasSlot->SetOffsets(FMargin(0.f));
        }

        // Add a background border
        UBorder* Background = NewObject<UBorder>(ChildCanvas);
        if (Background)
        {
            Background->SetBrushColor(Color);
            ChildCanvas->AddChild(Background);

            if (UCanvasPanelSlot* BackgroundSlot = Cast<UCanvasPanelSlot>(Background->Slot))
            {
                BackgroundSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
                BackgroundSlot->SetOffsets(FMargin(0.f));
            }
        }

        //RootCanvas->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
        return ChildCanvas;
    }


    UFUNCTION(BlueprintCallable, Category = "SpicyTools")
    static UTextBlock* AddTextToCanvas(UCanvasPanel* ParentCanvas, 
        const FString& TextString, 
        float AnchorLeft = 0.0,
        float AnchorTop = 0.0,
        float AnchorRight = 1.0,
        float AnchorBottom = 1.0
    )
    {
        if (!ParentCanvas) return nullptr;

        // Create TextBlock dynamically
        UTextBlock* TextWidget = NewObject<UTextBlock>(ParentCanvas);
        if (!TextWidget) return nullptr;

        TextWidget->SetText(FText::FromString(TextString));
        TextWidget->SetColorAndOpacity(FSlateColor(FLinearColor::Black));
        TextWidget->SetJustification(ETextJustify::Center);

        // Add TextBlock to the CanvasPanel
        ParentCanvas->AddChild(TextWidget);

        // Set anchors & offsets for the TextBlock
        if (UCanvasPanelSlot* TextSlot = Cast<UCanvasPanelSlot>(TextWidget->Slot))
        {
            // 0.0 |    top | left 
            // 1.0 | bottom | right
            TextSlot->SetAnchors( FAnchors(AnchorLeft,AnchorTop,AnchorRight,AnchorBottom) ); // fill parent
            TextSlot->SetOffsets(FMargin(0.f));
        }

        return TextWidget;
    }


    UFUNCTION(BlueprintCallable, Category = "SpicyTools")
    static UButton* AddButtonToCanvas(
        UCanvasPanel* ParentCanvas,
        const FString& TextString, 
        float AnchorLeft = 0.0f,
        float AnchorTop = 0.0f,
        float AnchorRight = 1.0f,
        float AnchorBottom = 1.0f,
        FLinearColor Color = FLinearColor::Gray
    )
    {
        if (!ParentCanvas) return nullptr;

        // Create Button dynamically
        UButton* ButtonWidget = NewObject<UButton>(ParentCanvas);
        if (!ButtonWidget) return nullptr;

        ParentCanvas->AddChild(ButtonWidget);

        // Button color
        FButtonStyle ButtonStyle;
        ButtonStyle.SetNormal( FSlateColorBrush(Color) );
        ButtonStyle.SetHovered( FSlateColorBrush(FLinearColor::Red) );
        ButtonWidget->SetStyle(ButtonStyle);

        // Set anchors & offsets for the Button
        if (UCanvasPanelSlot* ButtonSlot = Cast<UCanvasPanelSlot>(ButtonWidget->Slot))
        {
            ButtonSlot->SetAnchors(FAnchors(AnchorLeft, AnchorTop, AnchorRight, AnchorBottom));
            //ButtonSlot->SetOffsets(FMargin(0.f, 0.f, Size.X, Size.Y)); // X = width, Y = height
            ButtonSlot->SetOffsets(FMargin(0.f));
        }

        // Text 
        UTextBlock* TextWidget = NewObject<UTextBlock>(ButtonWidget);
        if (!TextWidget) return nullptr;

        TextWidget->SetText(FText::FromString(TextString));
        TextWidget->SetColorAndOpacity(FSlateColor(FLinearColor::Black));
        TextWidget->SetJustification(ETextJustify::Center);
        ButtonWidget->AddChild(TextWidget);

        return ButtonWidget;
    }


    UFUNCTION(BlueprintCallable, Category = "SpicyTools")
    static UMaterialInstanceDynamic* CreateDynamicMaterial(FString materialPath, UObject* owner)
    {
        UMaterialInterface* baseMaterial = nullptr; 
        baseMaterial = LoadObject<UMaterialInterface>(nullptr, *materialPath);

        UMaterialInstanceDynamic* dynamicMaterial = nullptr; 
        
        dynamicMaterial = UMaterialInstanceDynamic::Create(baseMaterial, owner);

        if(dynamicMaterial == nullptr) UE_LOG(LogTemp, Warning, TEXT("[CreateDynamicMaterial] fails!!") ); 
        return dynamicMaterial; 
    }


    UFUNCTION(BlueprintCallable, Category = "SpicyTools")
    static ASceneCapture2D* CreateSceneCapture(APlayerController* playerController, UObject* owner)
    {
        UTextureRenderTarget2D* renderTarget = nullptr; 
        renderTarget = CreateRenderTarget(owner, 1024, 1024);

        ASceneCapture2D* sceneCapture = nullptr; 
        //UPipelineTools::CreateSceneCaptureActor(this, RenderTarget, FVector(0, 0, 200), FRotator(-30, 0, 0));
        sceneCapture = CreateSceneCaptureActor2(playerController, renderTarget);
        return sceneCapture; 
    }

UFUNCTION(BlueprintCallable, Category = "SpicyTools") static 
void SetUIOnlyInputMode(UUserWidget* FocusWidget, APlayerController* PC)
{
    if (!PC || !FocusWidget) return;

    //  Direct access to bIsFocusable is deprecated. Please use the getter. 
    //FocusWidget->bIsFocusable = true;

    // Add to viewport if not already
    if (!FocusWidget->IsInViewport())
    {
        FocusWidget->AddToViewport();
    }

    // Prepare input mode

FInputModeGameAndUI InputMode;
InputMode.SetWidgetToFocus(FocusWidget->TakeWidget());
InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

    // Apply mode to player controller
    PC->SetInputMode(InputMode);
    PC->bShowMouseCursor = true;
}



template<typename TResult>
static void SetAsyncFuncs(TFunction<TResult()> Task, TFunction<void(TResult)> After)
{
    auto Promise = MakeShared<TPromise<TResult>>();
    TFuture<TResult> Future = Promise->GetFuture();

    Async(EAsyncExecution::Thread, [Task, Promise]()
    {
        TResult Result = Task();
        Promise->SetValue(Result);
    });

    Future.Next([After](const TResult& Result)
    {
        AsyncTask(ENamedThreads::GameThread, [After, Result]()
        {
            After(Result);
        });
    });
}


    
    UFUNCTION(BlueprintCallable, Category = "SpicyTools") static 
    void XYZ() {}

}; // UPipelineTools

/*

void AMyActor::BeginPlay()
{
    Super::BeginPlay();

    // Create pipeline instance (pass 'this' as World context)
    UPipelineTools* Pipeline = UPipelineTools::CreatePipeline(this);
    if (!Pipeline) return;

    // 1) Create RT
    UTextureRenderTarget2D* RT = Pipeline->CreateRenderTarget(1024, 1024);

    // 2) Create a material instance that uses the render target
    //    -> Make sure MyBaseUIMaterial has Domain = User Interface and a texture parameter named "TextureParam"
    UMaterialInstanceDynamic* MID = Pipeline->CreateMaterialForRT(MyBaseUIMaterial, RT, TEXT("TextureParam"));

    // 3) Spawn SceneCapture2D and attach the RT
    ASceneCapture2D* Capture = Pipeline->CreateSceneCaptureActor(RT, FVector(0, 0, 200), FRotator(-30, 0, 0), true);

    // 4) Create widget (the widget must contain an Image named "TargetImage")
    UUserWidget* Widget = Pipeline->CreateWidgetWithMaterial(MyWidgetClass, MID, TEXT("TargetImage"), FVector2D(512,512));
    if (Widget)
    {
        Widget->AddToViewport();
    }
}


*/