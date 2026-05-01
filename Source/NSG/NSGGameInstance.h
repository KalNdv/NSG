#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "NSGGameInstance.generated.h"

UCLASS()
class NSG_API UNSGGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	// The ONLY thing we need to pass through... If it is co-op.
	UPROPERTY(BlueprintReadWrite, Category = "Game Settings")
	bool bWantsCoop = false;

	// Never mind, forgot about score!
	UPROPERTY(BlueprintReadWrite, Category = "Game Settings")
	int32 TotalScore = 0;
};