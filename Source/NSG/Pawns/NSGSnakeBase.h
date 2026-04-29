#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "NSGSnakeBase.generated.h"

UCLASS()
class NSG_API ANSGSnakeBase : public APawn
{
	GENERATED_BODY()

public:
	// Sets defaults
	ANSGSnakeBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Snake head mesh in blueprint
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* HeadMesh;

	// The Spring arm
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class USpringArmComponent* SpringArm;

	// The Camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class UCameraComponent* Camera;

	// The 3D Text
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UTextRenderComponent* TimerText;

	// Camera duration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mechanics")
	float MaxSwapTime = 30.0f;

	// The countdown value (gets set to MaxSwapTime)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mechanics")
	float CurrentSwapTimer;
	float InvincibilityTimer;

	// Coop toggler
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Rules")
	bool bIsCoopMode = false;

	// Track who is driving the snake
	bool bIsPlayer1Driving = true;

public:
	// Snake movement speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MoveSpeed;

	// Snake max turn speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MaxTurnSpeed = 80.0f;

	// Ticking
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
	void OnHeadOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:
	// Tail segment class
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tail")
	TArray<class AActor*> TailSegments;

	// Segment class
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tail")
	TSubclassOf<class AActor> SegmentClass;

	// Distance between body parts
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tail")
	float SegmentSpacing = 100.0f;

	// Snake growth function
	void AddTailSegment();

	void RefreshTailVisuals();

	// Core mechanic!! Swap heads and tails, good for co-op too!
	UFUNCTION(BlueprintCallable, Category = "Mechanics")
	void SwapHeadAndTail();

public:
	// Shrink and growth duration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mechanics")
	float SwapTransitionTime = 0.3f; // 0.3 seconds down and 0.3 seconds up - so basically SwapTransitionTime * 2 = current swap duration

private:
	// Animation tracker
	bool bIsSwapping = false;
	bool bMidSwapTeleportDone = false;
	float CurrentSwapTime = 0.0f;

	// Temporary pointer to tail segment
	class AActor* SwappingSegment = nullptr;

	// Helper teleport function
	void PerformSwapTeleport();
};