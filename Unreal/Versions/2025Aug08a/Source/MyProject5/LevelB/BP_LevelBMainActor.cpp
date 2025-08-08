#include "BP_LevelBMainActor.h"
#include "Kismet/GameplayStatics.h"


ABP_LevelBMainActor::ABP_LevelBMainActor()
{
	PrimaryActorTick.bCanEverTick = true;

	std::string filename = "/Game/SpicyTech/LevelB/WBP_LevelB"; 
	WidgetClass = WidgetClassCreate(filename); 
}


void ABP_LevelBMainActor::BeginPlay()
{
	Super::BeginPlay();
	WidgetCreate(); 
}


TSubclassOf<UMainWidget> ABP_LevelBMainActor::WidgetClassCreate(std::string filename)
{
    static ConstructorHelpers::FClassFinder<UMainWidget> WidgetBPClass(*FString(filename.c_str()));
    if (WidgetBPClass.Succeeded())
    {
        return WidgetBPClass.Class;
    }

    return nullptr;
}

void ABP_LevelBMainActor::WidgetCreate()
{	
	if(WidgetClass == nullptr) return ; 

	//APlayerController* PC = GetWorld()->GetFirstPlayerController();
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	WidgetInstance = CreateWidget<UMainWidget>(PC, WidgetClass);
	//WidgetInstance = CreateWidget<UMainWidget>(GetWorld(), WidgetClass); // :) 
	
	int zorder = 999; 
    if (WidgetInstance)
    {
		WidgetInstance->AddToViewport(zorder);

		// Blocking??
FInputModeUIOnly InputMode;
InputMode.SetWidgetToFocus(WidgetInstance->TakeWidget());
InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

PC->SetInputMode(InputMode);
PC->bShowMouseCursor = true;

	} 

} 


void ABP_LevelBMainActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}