#include "NSGGameMode.h"
#include "../Pawns/NSGSnakeBase.h"
#include "Math/UnrealMathUtility.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

ANSGGameMode::ANSGGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ANSGGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Spawn start enemies
	for (int32 i = 0; i < StartingEnemies; i++)
	{
		SpawnEnemy();
	}

	// The pellet timer
	GetWorldTimerManager().SetTimer(PelletTimerHandle, this, &ANSGGameMode::SpawnPellet, PelletSpawnRate, true);
}

void ANSGGameMode::SpawnPellet()
{
	if (!PelletClass) return;

	// Random spawn point
	FVector2D RandomPoint = FMath::RandPointInCircle(SpawnRadius);

	// Conversion from 2d to 3d
	FVector SpawnLocation(RandomPoint.X, RandomPoint.Y, 50.0f); // Magic number, but it's fine.. Why is Z up.

	GetWorld()->SpawnActor<AActor>(PelletClass, SpawnLocation, FRotator::ZeroRotator);
}

void ANSGGameMode::SpawnEnemy()
{
	if (!EnemySnakeClass) return;

	FVector2D RandomPoint = FMath::RandPointInCircle(SpawnRadius);
	FVector SpawnLocation(RandomPoint.X, RandomPoint.Y, 50.0f);

	// Face enemy randomly on spawn
	FRotator RandomRotation(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);

	GetWorld()->SpawnActor<ANSGSnakeBase>(EnemySnakeClass, SpawnLocation, RandomRotation);
}

void ANSGGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	GlobalSwapTimer -= DeltaTime;

	if (GlobalSwapTimer <= 0.0f)
	{
		GlobalSwapTimer = CurrentMaxSwapTime;

		// Every snake is now coupled to the timer. May the chaos begin
		TArray<AActor*> AllSnakes;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANSGSnakeBase::StaticClass(), AllSnakes);
		for (AActor* Actor : AllSnakes)
		{
			if (ANSGSnakeBase* Snake = Cast<ANSGSnakeBase>(Actor))
			{
				Snake->SwapHeadAndTail();
			}
		}
	}
}

void ANSGGameMode::AddScoreAndScaleDifficulty()
{
	GlobalScore += 10;
	DifficultyMultiplier += 0.05f; // Increase global speed by 5%

	// Shrink the swap timer, but clamp it so it never drops below 5 seconds
	CurrentMaxSwapTime = FMath::Clamp(30.0f - (DifficultyMultiplier * 2.0f), 5.0f, 30.0f);

	// Apply the speed buff to all snakes currently alive
	TArray<AActor*> AllSnakes;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANSGSnakeBase::StaticClass(), AllSnakes);
	for (AActor* Actor : AllSnakes)
	{
		if (ANSGSnakeBase* Snake = Cast<ANSGSnakeBase>(Actor))
		{
			Snake->MoveSpeed = FMath::Clamp(400.0f * DifficultyMultiplier, 400.0f, 2000.0f);
			Snake->MaxTurnSpeed = FMath::Clamp(80.0f * DifficultyMultiplier, 80.0f, 400.0f);
		}
	}

	// Speed up the pellet rain!
	PelletSpawnRate = FMath::Clamp(0.5f / DifficultyMultiplier, 0.1f, 0.5f);
	GetWorldTimerManager().ClearTimer(PelletTimerHandle);
	GetWorldTimerManager().SetTimer(PelletTimerHandle, this, &ANSGGameMode::SpawnPellet, PelletSpawnRate, true);
}