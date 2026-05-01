#include "NSGGameMode.h"
#include "Blueprint/UserWidget.h"
#include "../Pawns/NSGSnakeBase.h"
#include "Math/UnrealMathUtility.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "../Pawns/NSGSnakeBase.h"
#include "../NSGGameInstance.h"

ANSGGameMode::ANSGGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ANSGGameMode::BeginPlay()
{
	Super::BeginPlay();

	DifficultyMultiplier = InitialDifficultyMultiplier;

	// Spawn start enemies
	for (int32 i = 0; i < StartingEnemies; i++)
	{
		SpawnEnemy();
	}

	// The pellet timer
	GetWorldTimerManager().SetTimer(PelletTimerHandle, this, &ANSGGameMode::SpawnPellet, PelletSpawnRate, true);
	
	// The snake timer
	GetWorldTimerManager().SetTimer(EnemyTimerHandle, this, &ANSGGameMode::SpawnEnemy, EnemySpawnRate, true);
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
	// Population capper!
	TArray<AActor*> AllSnakes;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANSGSnakeBase::StaticClass(), AllSnakes);

	// Total snake pool
	int32 CurrentEnemyCount = AllSnakes.Num() - 1;

	// If we have reached the max allowed enemies, do not spawn
	if (CurrentEnemyCount >= MaxEnemies)
	{
		return;
	}

	// Safe little spawn zone so no snake eats us alive
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	FVector PlayerLocation = PlayerPawn ? PlayerPawn->GetActorLocation() : FVector::ZeroVector;

	FVector SpawnLocation;
	bool bFoundSafeSpot = false;

	// Loop a couple of times to find a good spot away from the player
	for (int32 i = 0; i < 10; i++)
	{
		FVector2D RandomPoint = FMath::RandPointInCircle(SpawnRadius);
		SpawnLocation = FVector(RandomPoint.X, RandomPoint.Y, 50.0f);

		// IF we are 500 units away, then great
		if (!PlayerPawn || FVector::Dist2D(SpawnLocation, PlayerLocation) >= 500.0f)
		{
			bFoundSafeSpot = true;
			break; // Found it! Out of loop.
		}
	}

	// Give up if no spot is found
	if (!bFoundSafeSpot) return;

	// Random rotation
	FRotator RandomRotation(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);

	// Spawning probability
	TSubclassOf<ANSGSnakeBase> ClassToSpawn = EnemySnakeClass;

	if (OrangeSnakeClass)
	{
		// Spawn random snakes, then 50/50 when at highest difficulty scaling
		float OrangeChance = FMath::Clamp((DifficultyMultiplier - 1.0f) * 0.2f, 0.0f, 0.5f);

		if (FMath::FRand() <= OrangeChance)
		{
			ClassToSpawn = OrangeSnakeClass;
		}
	}

	if (!ClassToSpawn) return;

	ANSGSnakeBase* NewEnemy = GetWorld()->SpawnActor<ANSGSnakeBase>(ClassToSpawn, SpawnLocation, RandomRotation);

	if (NewEnemy)
	{
		int32 TargetLength = FMath::RoundToInt(3.0f * DifficultyMultiplier);
		for (int32 i = 0; i < TargetLength; i++)
		{
			NewEnemy->AddTailSegment();
		}
	}
}

void ANSGGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Arena drawing
	DrawDebugCircle(GetWorld(), FVector(0.0f, 0.0f, 10.0f), SpawnRadius + 200.0f, 64, FColor::Red, false, -1.0f, 0, 20.0f, FVector(0, 1, 0), FVector(1, 0, 0), false);

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
	if (UNSGGameInstance* GI = Cast<UNSGGameInstance>(GetGameInstance()))
	{
		GI->TotalScore += 10;
		
	}

	DifficultyMultiplier += 0.03f; // Increase global speed by 3%

	// Shrink the swap timer, but clamp it so it never drops below 5 seconds
	CurrentMaxSwapTime = FMath::Clamp(30.0f - (DifficultyMultiplier * 2.0f), 5.0f, 30.0f);

	// Apply the speed buff to all snakes currently alive
	TArray<AActor*> AllSnakes;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANSGSnakeBase::StaticClass(), AllSnakes);
	for (AActor* Actor : AllSnakes)
	{
		if (ANSGSnakeBase* Snake = Cast<ANSGSnakeBase>(Actor))
		{
			Snake->MoveSpeed = FMath::Clamp(400.0f * DifficultyMultiplier / 2.0f, 400.0f, 2000.0f);
			Snake->MaxTurnSpeed = FMath::Clamp(80.0f * DifficultyMultiplier / 2.0f, 80.0f, 400.0f);
		}
	}

	// Speed up the pellet rain!
	PelletSpawnRate = FMath::Clamp(0.5f / DifficultyMultiplier, 0.1f, 0.5f);
	GetWorldTimerManager().ClearTimer(PelletTimerHandle);
	GetWorldTimerManager().SetTimer(PelletTimerHandle, this, &ANSGGameMode::SpawnPellet, PelletSpawnRate, true);
}

void ANSGGameMode::ShowGameOver()
{
	if (GameOverWidgetClass)
	{
		ActiveWidget = CreateWidget<UUserWidget>(GetWorld(), GameOverWidgetClass);
		if (ActiveWidget)
		{
			ActiveWidget->AddToViewport();
			if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
			{
				PC->SetShowMouseCursor(true);
				PC->SetPause(true); // Freeze game
			}
		}
	}
}

void ANSGGameMode::ShowWinScreen()
{
	if (WinWidgetClass && !bIsFreeplay)
	{
		ActiveWidget = CreateWidget<UUserWidget>(GetWorld(), WinWidgetClass);
		if (ActiveWidget)
		{
			ActiveWidget->AddToViewport();
			if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
			{
				PC->SetShowMouseCursor(true);
				PC->SetPause(true); // Freeze game yet again!
			}
		}
	}
}

void ANSGGameMode::TogglePause()
{
	// Don't pause if dead or won
	if (ActiveWidget && ActiveWidget->GetClass() != PauseWidgetClass) return;

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC) return;

	if (UGameplayStatics::IsGamePaused(GetWorld()))
	{
		// Unpause
		if (ActiveWidget)
		{
			ActiveWidget->RemoveFromParent();
			ActiveWidget = nullptr; // Kill pointer
		}
		PC->SetPause(false);

		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
	}
	else if (PauseWidgetClass)
	{
		// Pause
		ActiveWidget = CreateWidget<UUserWidget>(GetWorld(), PauseWidgetClass);
		if (ActiveWidget)
		{
			ActiveWidget->AddToViewport();
			PC->SetPause(true);

			FInputModeGameAndUI InputMode;
			PC->SetInputMode(InputMode);
			PC->SetShowMouseCursor(true);
		}
	}
}

void ANSGGameMode::StartFreeplay()
{
	bIsFreeplay = true;

	if (ActiveWidget)
	{
		ActiveWidget->RemoveFromParent();
		ActiveWidget = nullptr; 
	}

	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);

		// Unfreeze the game!
		PC->SetPause(false);

		if (ANSGSnakeBase* Snake = Cast<ANSGSnakeBase>(PC->GetPawn()))
		{
			Snake->InvincibilityTimer = 0.5f;

			FTimerHandle WakeupTimer;
			GetWorldTimerManager().SetTimer(WakeupTimer, [Snake]()
				{
					if (IsValid(Snake) && Snake->HeadMesh)
					{
						Snake->HeadMesh->SetGenerateOverlapEvents(false);
					}
				}, 0.05f, false);
		}
	}
}

void ANSGGameMode::LoadNextLevel()
{
	// Load next level, safety check as I'm a defensive programmer and scared of everything
	if (!NextLevelName.IsNone())
	{
		UGameplayStatics::OpenLevel(this, NextLevelName);
	}
}