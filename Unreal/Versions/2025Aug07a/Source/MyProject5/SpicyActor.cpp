#include "SpicyActor.h" // always first?? 
#include "Components/Button.h"


TSubclassOf<UMyAsyncWidget> WidgetClassCreate()
{
	std::string filename = "/Game/SpicyTech/WBP_AsyncWidget";

    static ConstructorHelpers::FClassFinder<UMyAsyncWidget> WidgetBPClass(*FString(filename.c_str()));
    if (WidgetBPClass.Succeeded())
    {
        return WidgetBPClass.Class;
    }

    return nullptr;
}


void ASpicyActor::WidgetCreate()
{	
	if(WidgetClass == nullptr) return ; 

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	
	//WidgetInstance = CreateWidget<UMyAsyncWidget>(GetWorld(), WidgetClass);
	WidgetInstance = CreateWidget<UMyAsyncWidget>(PC, WidgetClass);
    if (WidgetInstance && PC)
    {
		WidgetInstance->AddToViewport();

		// It seems this crashig ... ?? 
		if (WidgetInstance->StartButton)
		{
			//WidgetInstance->StartButton->SetIsEnabled(true);
			//WidgetInstance->StartButton->OnClicked.AddDynamic(this, &ASpicyActor::ButtonClicked);
		}
	}
}


void ASpicyActor::ButtonClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("[ASpicyActor] Button clicked from Widget!"));
}


ASpicyActor::ASpicyActor()
{
	PrimaryActorTick.bCanEverTick = true;

	WidgetClass = WidgetClassCreate(); 
}


void ASpicyActor::BeginPlay()
{
	Super::BeginPlay();

	WidgetCreate(); 
}

void ASpicyActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

