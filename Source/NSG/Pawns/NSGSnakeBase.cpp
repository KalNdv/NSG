#include "NSGSnakeBase.h"
#include "Components/StaticMeshComponent.h"
#include "Algo/Reverse.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/TextRenderComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NSGSnakeSegment.h"

ANSGSnakeBase::ANSGSnakeBase()
{
	// Learning: This is for enabling ticking every frame, rember
	PrimaryActorTick.bCanEverTick = true;

	// Create mesh component of the snake head
	HeadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeadMesh"));
	RootComponent = HeadMesh;

	HeadMesh->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	HeadMesh->SetGenerateOverlapEvents(true);
	HeadMesh->OnComponentBeginOverlap.AddDynamic(this, &ANSGSnakeBase::OnHeadOverlap);

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

	// Invincibility timer, toggles that headmesh overlap check too
	if (InvincibilityTimer > 0.0f)
	{
		InvincibilityTimer -= DeltaTime;

		if (InvincibilityTimer <= 0.0f)
		{
			HeadMesh->SetGenerateOverlapEvents(true);
		}
	}

	if (bIsSwapping)
	{
		CurrentSwapTime += DeltaTime;

		if (CurrentSwapTime <= SwapTransitionTime)
		{

			// Shrink first
			// Reworked scaling, minimum of 0.01f to preserve collision if this is what is breaking it
			float Scale = FMath::Lerp(1.0f, 0.01f, CurrentSwapTime / SwapTransitionTime);

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
			// Reworked scaling, minimum of 0.01f to preserve collision if this is what is breaking it
			float Scale = FMath::Lerp(0.01f, 1.0f, (CurrentSwapTime - SwapTransitionTime) / SwapTransitionTime);

			HeadMesh->SetWorldScale3D(FVector(Scale));

			// Because PerformSwapTeleport() reversed the array, our SwappingSegment is now at Index 0!
			if (TailSegments.Num() > 0) TailSegments[0]->SetActorScale3D(FVector(Scale));
		}
		else
		{
			bIsSwapping = false;
			HeadMesh->SetWorldScale3D(FVector(1.0f));
			if (TailSegments.Num() > 0) TailSegments[0]->SetActorScale3D(FVector(1.0f));

			// Give the player 0.5 seconds of invincibility after the swap finishes
			InvincibilityTimer = 0.5f;

			// Refresh the tail visuals 
			RefreshTailVisuals();
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

	// Funny controller link
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		// Singleplayer or Co-op? If single player or player 1 in co-op:
		if (!bIsCoopMode || bIsPlayer1Driving)
		{
			FHitResult HitResult;
			if (PC->GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
			{
				FVector TargetLocation = HitResult.ImpactPoint;
				TargetLocation.Z = GetActorLocation().Z;
				FRotator TargetRotation = (TargetLocation - GetActorLocation()).Rotation();
				FRotator ClampedRotation = FMath::RInterpConstantTo(GetActorRotation(), TargetRotation, DeltaTime, MaxTurnSpeed);
				SetActorRotation(ClampedRotation);
			}
		}
		// If it is player 2s turn in co-op:
		else
		{
			float TurnInput = 0.0f;

			// Read the arrow keys directly
			if (PC->IsInputKeyDown(EKeys::Right)) TurnInput += 1.0f;
			if (PC->IsInputKeyDown(EKeys::Left)) TurnInput -= 1.0f;

			if (TurnInput != 0.0f)
			{
				// Spin the snake left or right based on the Arrow Keys
				FRotator NewRotation = GetActorRotation() + FRotator(0.0f, TurnInput * MaxTurnSpeed * DeltaTime, 0.0f);
				SetActorRotation(NewRotation);
			}
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

		// Refresh tail
		RefreshTailVisuals();
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

	bIsPlayer1Driving = !bIsPlayer1Driving;

	HeadMesh->SetGenerateOverlapEvents(false); // For safety, since for some reason I cannot collide with my own tail
}

void ANSGSnakeBase::PerformSwapTeleport()
{
	FVector OldHeadLocation = GetActorLocation();

	// Take last segment and make snake head face opposite of it
	FRotator TailRotation = TailSegments.Last()->GetActorRotation();
	FRotator InvertedRotation = TailRotation + FRotator(0.0f, 180.0f, 0.0f);

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

void ANSGSnakeBase::RefreshTailVisuals()
{
	for (int32 i = 0; i < TailSegments.Num(); i++)
	{
		// Try to cast the generic Actor into our specific Segment class
		if (ANSGSnakeSegment* Segment = Cast<ANSGSnakeSegment>(TailSegments[i]))
		{
			// It is the last piece ONLY if its index is Size - 1
			bool bIsLast = (i == TailSegments.Num() - 1);
			Segment->UpdateVisuals(bIsLast);
		}
	}
}

void ANSGSnakeBase::OnHeadOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// If overlapping something that isn't a tail then ignore
	if (bIsSwapping || InvincibilityTimer > 0.0f || !OtherActor || OtherActor == this) return;

	// Did we hit a snake segment?
	if (ANSGSnakeSegment* HitSegment = Cast<ANSGSnakeSegment>(OtherActor))
	{
		// Find which segment we hit
		int32 SegmentIndex = TailSegments.Find(HitSegment);

		// Ignore our own neck!
		if (SegmentIndex > 2)
		{
			UE_LOG(LogTemp, Error, TEXT("GAME OVER! You bit your tail!"));

			UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()), false);
		}
	}
}