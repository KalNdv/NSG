#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NSGSnakeSegment.generated.h"

UCLASS()
class NSG_API ANSGSnakeSegment : public AActor
{
	GENERATED_BODY()

public:
	ANSGSnakeSegment();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* SegmentMesh;
};