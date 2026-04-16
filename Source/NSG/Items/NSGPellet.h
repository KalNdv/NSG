#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NSGPellet.generated.h"

UCLASS()
class NSG_API ANSGPellet : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ANSGPellet();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Collision sphere
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USphereComponent* CollisionComponent;

	// Pellet mesh
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* PelletMesh;

	// Overlap function
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};