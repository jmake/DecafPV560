// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


//#include "Sources/vtktools2.hpp"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ManagerCutter.generated.h" // should always be the last


//----------------------------------------------------------------------------------|  |--//
//-----------------------------------------------------------------------------| Actor |--//
UCLASS()
class MYPROJECT5_API AManagerCutter : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AManagerCutter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
