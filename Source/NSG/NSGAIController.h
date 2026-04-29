#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "NSGAIController.generated.h"

UCLASS()
class NSG_API ANSGAIController : public AAIController
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	// AI Params
	float RoamRadius = 800.0f;
	float SnipeDistance = 500.0f;
	float AvoidanceRadius = 300.0f;

	// State Management
	bool bIsSniping = false;
	float StateTimer = 0.0f;

private:
	FVector GetSteeringTarget();
};