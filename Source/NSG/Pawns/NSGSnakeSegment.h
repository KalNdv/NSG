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

public:
	// Body mesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	class UStaticMesh* BodyMesh;

	// Tail mesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	class UStaticMesh* TailMesh;

	// Function to swap the active mesh
	void UpdateVisuals(bool bIsLastSegment);
};