#include "NSGSnakeBase.h"
#include "Components/StaticMeshComponent.h"
#include "Algo/Reverse.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/TextRenderComponent.h"

ANSGSnakeBase::ANSGSnakeBase()
{
	// Learning: This is for enabling ticking every frame, rember
	PrimaryActorTick.bCanEverTick = true;

	// Create mesh component of the snake head
	HeadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeadMesh"));
	RootComponent = HeadMesh;

	// Floating 3d text
	TimerText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TimerText"));
	TimerText->SetupAttachment(RootComponent);

	// Text offset
	TimerText->SetRelativeLocation(FVector(0.0f, 0.0f, 150.0f));

	// Why is it like this, and why do I like it
	TimerText->SetUsingAbsoluteRotation(true);
	// Matching camera
	TimerText->SetRelativeRotation(FRotator(60.0f, 180.0f, 0.0f));

	// Center text and colour
	TimerText->SetHorizontalAlignment(EHTA_Center);
	TimerText->SetTextRenderColor(FColor::Red);

	// Spring Arm setup here
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 1500.0f;
	SpringArm->SetUsingAbsoluteRotation(true);
	SpringArm->SetRelativeRotation(FRotator(-60.0f, 0.0f, 0.0f));
	SpringArm->bDoCollisionTest = false;

	// Camera setup and attachment, intuitive-ish
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);

	// Default speed
	MoveSpeed = 400.0f;
}

// Called when the game starts or when spawned
void ANSGSnakeBase::BeginPlay()
{
	Super::BeginPlay();

	// Set the starting timer
	CurrentSwapTimer = MaxSwapTime;

	// Spawn with 2 segments
	AddTailSegment();
	AddTailSegment();

	// Get the player controller and tell it to show the mouse, get and setting is nice
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->bShowMouseCursor = true;
	}
}

// Called every frame
void ANSGSnakeBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsSwapping)
	{
		CurrentSwapTime += DeltaTime;

		if (CurrentSwapTime <= SwapTransitionTime)
		{
			// Shrink first
			float Scale = FMath::Lerp(1.0f, 0.0f, CurrentSwapTime / SwapTransitionTime);

			// Scale the head mesh and the tail segment
			HeadMesh->SetWorldScale3D(FVector(Scale));
			if (SwappingSegment) SwappingSegment->SetActorScale3D(FVector(Scale));
		}
		else if (CurrentSwapTime <= SwapTransitionTime * 2.0f)
		{
			// Midpoint = teleport
			if (!bMidSwapTeleportDone)
			{
				PerformSwapTeleport();
				bMidSwapTeleportDone = true;
			}

			// Return growth
			float Scale = FMath::Lerp(0.0f, 1.0f, (CurrentSwapTime - SwapTransitionTime) / SwapTransitionTime);

			HeadMesh->SetWorldScale3D(FVector(Scale));

			// Because PerformSwapTeleport() reversed the array, our SwappingSegment is now at Index 0!
			if (TailSegments.Num() > 0) TailSegments[0]->SetActorScale3D(FVector(Scale));
		}
		else
		{
			// Finish swap
			bIsSwapping = false;
			HeadMesh->SetWorldScale3D(FVector(1.0f));
			if (TailSegments.Num() > 0) TailSegments[0]->SetActorScale3D(FVector(1.0f));
		}

		// Useful little return
		return;
	}

	// Only count down if we aren't currently in the middle of a swap animation
	if (!bIsSwapping)
	{
		CurrentSwapTimer -= DeltaTime;

		// Convert the float into a rounded-up int then to text
		int32 SecondsLeft = FMath::CeilToInt(CurrentSwapTimer);
		TimerText->SetText(FText::FromString(FString::FromInt(SecondsLeft)));

		// If we hit zero, force a swap!
		if (CurrentSwapTimer <= 0.0f)
		{
			SwapHeadAndTail();
		}
	}

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		FHitResult HitResult;

		// Ray casting from mouse!
		if (PC->GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
		{
			FVector TargetLocation = HitResult.ImpactPoint;

			// Lock so the snake only moves along its proper Z... Y axis... 
			TargetLocation.Z = GetActorLocation().Z;

			// Calculate the rotation needed to look at the mouse
			FRotator TargetRotation = (TargetLocation - GetActorLocation()).Rotation();

			// Smoothly turn the snake at a maximum speed
			FRotator ClampedRotation = FMath::RInterpConstantTo(GetActorRotation(), TargetRotation, DeltaTime, MaxTurnSpeed);
			SetActorRotation(ClampedRotation);
		}
	}

	// Move head with world space
	FVector ForwardMove = GetActorForwardVector() * MoveSpeed * DeltaTime;
	AddActorWorldOffset(ForwardMove, true);

	// Move tail
	AActor* Leader = this; // The first segment follows the Head

	for (int32 i = 0; i < TailSegments.Num(); i++)
	{
		AActor* CurrentSegment = TailSegments[i];
		if (!CurrentSegment) continue;

		FVector LeaderLocation = Leader->GetActorLocation();
		FVector SegmentLocation = CurrentSegment->GetActorLocation();

		// Check the distance between this segment and the one in front of it
		float Distance = FVector::Dist(LeaderLocation, SegmentLocation);

		if (Distance > SegmentSpacing)
		{
			// If it's too far away, pull it towards the leader
			FVector Direction = (LeaderLocation - SegmentLocation).GetSafeNormal();
			FVector NewLocation = SegmentLocation + (Direction * MoveSpeed * DeltaTime);

			// Rotate to face leader
			CurrentSegment->SetActorRotation(Direction.Rotation());
			CurrentSegment->SetActorLocation(NewLocation);
		}

		// NEXT segment in the loop follows THIS segment
		Leader = CurrentSegment;
	}
}

// Called to bind functionality to input
void ANSGSnakeBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ANSGSnakeBase::AddTailSegment()
{
	if (!SegmentClass)
	{
		UE_LOG(LogTemp, Error, TEXT("Snake cannot grow: SegmentClass is not set in Blueprint!"));
		return;
	}

	// Figure out where to spawn the new part (behind the last segment or behind the head)
	FVector SpawnLocation = GetActorLocation();
	if (TailSegments.Num() > 0)
	{
		SpawnLocation = TailSegments.Last()->GetActorLocation();
	}

	// Spawn it into the world
	FActorSpawnParameters SpawnParams;
	AActor* NewSegment = GetWorld()->SpawnActor<AActor>(SegmentClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);

	if (NewSegment)
	{
		// Add it to our list so the Tick function starts moving it
		TailSegments.Add(NewSegment);
	}
}

void ANSGSnakeBase::SwapHeadAndTail()
{
	if (TailSegments.Num() == 0 || bIsSwapping) return;

	bIsSwapping = true;
	bMidSwapTeleportDone = false;
	CurrentSwapTime = 0.0f;
	SwappingSegment = TailSegments.Last(); 

	CurrentSwapTimer = MaxSwapTime;
}

void ANSGSnakeBase::PerformSwapTeleport()
{
	FVector OldHeadLocation = GetActorLocation();
	FRotator InvertedRotation = GetActorRotation() + FRotator(0.0f, 180.0f, 0.0f);

	TArray<FVector> OldTailLocations;
	for (int32 i = 0; i < TailSegments.Num(); i++)
	{
		OldTailLocations.Add(TailSegments[i]->GetActorLocation());
	}

	SetActorLocationAndRotation(OldTailLocations.Last(), InvertedRotation);
	TailSegments[0]->SetActorLocation(OldHeadLocation);

	for (int32 i = 1; i < TailSegments.Num(); i++)
	{
		TailSegments[i]->SetActorLocation(OldTailLocations[i - 1]);
	}

	Algo::Reverse(TailSegments);
}