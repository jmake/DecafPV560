#pragma once

#include "MainWidget.h"

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
	TSubclassOf<UMainWidget> WidgetClassCreate(std::string filename); 

	void WidgetCreate(); 


    UPROPERTY()
    UMainWidget* WidgetInstance = nullptr;		

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UMainWidget> WidgetClass = nullptr;

};
