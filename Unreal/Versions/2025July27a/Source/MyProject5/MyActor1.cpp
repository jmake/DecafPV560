// Fill out your copyright notice in the Description page of Project Settings.

#include "MyActor1.h"
//#include "Kismet/GameplayStatics.h"

#include "StaticTest.h"
#include "ExternalLibrary.h"


/*
1) 
	ERROR : 
		... Cannot open include file: 'SpicyWrapper.h': ...
	SOLUTION : 
		In 'Source\MyProject5\MyProject5.Build.cs' add 'SpicyWrapper' -> 

		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", 
			"SpicyWrapper"
		});

2) 
	In 'Plugins\SpicyWrapper\Source\SpicyWrapper\SpicyWrapper.Build.cs' add -> 

	public SpicyWrapper(ReadOnlyTargetRules Target) : base(Target)
	{
        Type = ModuleType.External;
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "include"));
        PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "lib", "MyMathLib.lib"));
	}

*/

// Sets default values
AMyActor1::AMyActor1()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMyActor1::BeginPlay()
{
	auto* externalLibrary = new ExternalLibrary(); 
	float dynamicResult = externalLibrary->Sqrt(2);
	
	FString UEMessage = FString("DynamicResult:") + FString::SanitizeFloat(dynamicResult);
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 9999, FColor::Green, UEMessage); 	

	
	auto* staticTest = new StaticTest(); 
	float staticResult = staticTest->Sqrt(3);
	FString UEMessage2 = FString("    StaticResult:") + FString::SanitizeFloat(staticResult);
	if (GEngine) GEngine->AddOnScreenDebugMessage(0, 9999, FColor::Blue, UEMessage2, true); 	
	
} 
/*
void AMyActor1::BeginPlay()
{
	Super::BeginPlay();

	std::string msg = "[SpicyTech] Vtk Version : "; 

    // :) 
	dicom = new SpicyTech2::VtkTest();

	std::string vtkVersion = dicom->GetVersion(); 
	UE_LOG(LogTemp, Warning, TEXT("[SpicyTech] GetVersion: %s"), *FString(vtkVersion.c_str()));

	msg += vtkVersion; 

    //nifti = new SpicyTech3::Nifti(); 
    std::string fname = "F:\\z2025_1\\Dicom\\NII\\1.2.826.0.1.3680043.10633.nii"; 


	float Sum = 0.0; 
	FString UEMessage = FString(msg.c_str()); // + FString::SanitizeFloat(Sum);

	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Green, UEMessage); 
}
*/

// Called every frame
void AMyActor1::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

