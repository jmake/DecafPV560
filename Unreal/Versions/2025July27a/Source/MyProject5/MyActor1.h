// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "SpicyTechDicom2.hpp"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyActor1.generated.h" // should always be the last



UCLASS()
class MYPROJECT5_API AMyActor1 : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMyActor1();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, Category = "Test")
	int32 TestInteger = 41;		


	SpicyTech2::VtkTest *dicom = nullptr; 

    //std::string fname = "F:\\z2025_1\\Dicom\\NII\\1.2.826.0.1.3680043.10633.nii"; 
    //SpicyTech3::Nifti *nifti = nullptr; //new SpicyTech3::Nifti(); 

};
