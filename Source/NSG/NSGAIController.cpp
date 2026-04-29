#include "NSGAIController.h"
#include "Kismet/GameplayStatics.h"
#include "Pawns/NSGSnakeBase.h"

void ANSGAIController::BeginPlay()
{
	Super::BeginPlay();

	// Randomize their starting state so they don't all dive at the same time
	bIsSniping = FMath::RandBool();
	StateTimer = FMath::RandRange(1.0f, 6.0f);
	RoamRadius = FMath::RandRange(500.0f, 1500.0f);
	SnipeDistance = FMath::RandRange(300.0f, 800.0f);
}

void ANSGAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ANSGSnakeBase* MySnake = Cast<ANSGSnakeBase>(GetPawn());
	if (!MySnake) return;

	// States
	StateTimer -= DeltaTime;
	if (StateTimer <= 0.0f)
	{
		bIsSniping = !bIsSniping; // Toggle between roam and rush
		StateTimer = bIsSniping ? 3.0f : 8.0f; // 3 Second rush, 8 second roam
	}

	// Find players head
	FVector TargetPos = GetSteeringTarget();

	// Move to the player!
	FRotator TargetRot = (TargetPos - MySnake->GetActorLocation()).Rotation();
	FRotator NewRot = FMath::RInterpConstantTo(MySnake->GetActorRotation(), TargetRot, DeltaTime, MySnake->MaxTurnSpeed);
	MySnake->SetActorRotation(NewRot);
}

FVector ANSGAIController::GetSteeringTarget()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	ANSGSnakeBase* MySnake = Cast<ANSGSnakeBase>(GetPawn());
	if (!PlayerPawn || !MySnake) return FVector::ZeroVector;

	FVector FinalTarget = PlayerPawn->GetActorLocation();

	if (bIsSniping)
	{
		// Lead player
		FinalTarget += PlayerPawn->GetActorForwardVector() * SnipeDistance;
	}
	else
	{
		// Stay near the player
		FinalTarget += FVector(RoamRadius, 0, 0).RotateAngleAxis(GetWorld()->GetTimeSeconds() * 40.0f, FVector::UpVector);
	}

	TArray<AActor*> AllSnakes;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANSGSnakeBase::StaticClass(), AllSnakes);

	for (AActor* Actor : AllSnakes)
	{
		if (Actor == MySnake) continue; // Don't avoid our self

		float Distance = FVector::Dist(MySnake->GetActorLocation(), Actor->GetActorLocation());
		if (Distance < AvoidanceRadius)
		{
			// Calculate a way away from other snakes
			FVector PushDirection = (MySnake->GetActorLocation() - Actor->GetActorLocation()).GetSafeNormal();

			// The closer the more paranoid
			float PushMultiplier = (AvoidanceRadius - Distance) * 1.5f;
			FinalTarget += PushDirection * PushMultiplier;
		}
	}

	return FinalTarget;
}