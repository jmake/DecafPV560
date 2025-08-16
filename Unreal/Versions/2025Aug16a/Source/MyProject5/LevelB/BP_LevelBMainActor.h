#pragma once

#include "../MyActor1.h"
#include "VtkInterface.h" 

#include "MainWidget.h"
#include "SliderWidget.h"
#include "PipelineTools.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BP_LevelBMainActor.generated.h"


UCLASS()
class MYPROJECT5_API ABP_LevelBMainActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABP_LevelBMainActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	//TSubclassOf<UMainWidget> UMainWidgetFinder(std::string filename); 

	UMainWidget* CreateLayout(); 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UMainWidget> MainWidget = nullptr;

public :
	void LiveRenderCreate(UMainWidget* MainWidgetInstance);  

	void LiveRenderApply(std::string slideName); 
	void LiveRenderApply(USliderWidget* RadialSliderWidget); 

	void LiveRenderUpdate(); 

	APlayerController* playerController = nullptr; 

	TArray<USliderWidget*> SliderWidgets1;
	TMap<FString, USliderWidget*> SliderWidgets2;

public : 
	AMyActor1* vtkInstance = nullptr; 
	UTexture2D* axialTexture = nullptr;

	APixelTextureDisplay* dummy = nullptr; 

};
