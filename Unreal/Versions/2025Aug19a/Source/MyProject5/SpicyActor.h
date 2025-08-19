#pragma once

//#include "Sources/vtktools4.hpp"
#include "MyAsyncWidget.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpicyActor.generated.h"  // Should always be the last


UCLASS()
class MYPROJECT5_API ASpicyActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ASpicyActor();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	void WidgetCreate(); 

    UPROPERTY()
    UMyAsyncWidget* WidgetInstance = nullptr;	

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI") // Expose WidgetClass to the Editor
	UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UMyAsyncWidget> WidgetClass = nullptr;

	UFUNCTION() //  Must be a UFUNCTION to bind to delegate 
	void ButtonClicked(); 
};