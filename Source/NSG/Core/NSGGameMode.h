#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NSGGameMode.generated.h"

UCLASS()
class NSG_API ANSGGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ANSGGameMode();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	// Pellet spawn (Pellet Blueprint here)
	UPROPERTY(EditDefaultsOnly, Category = "Arena Spawning")
	TSubclassOf<AActor> PelletClass;

	// Enemy snake spawn (BP_HostileSnake here)
	UPROPERTY(EditDefaultsOnly, Category = "Arena Spawning")
	TSubclassOf<class ANSGSnakeBase> EnemySnakeClass;

	// Arena width
	UPROPERTY(EditAnywhere, Category = "Arena Spawning")
	float SpawnRadius = 3000.0f;

	// Pellet spawn per second
	UPROPERTY(EditAnywhere, Category = "Arena Spawning")
	float PelletSpawnRate = 0.5f;

	// How many enemies to spawn at the start
	UPROPERTY(EditAnywhere, Category = "Arena Spawning")
	int32 StartingEnemies = 3;

	// More Global Variables
	UPROPERTY(BlueprintReadOnly, Category = "Arena")
	int32 GlobalScore = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Arena")
	float GlobalSwapTimer = 30.0f;

	float CurrentMaxSwapTime = 30.0f;
	float DifficultyMultiplier = 1.0f;

	// Scaling and score
	UFUNCTION(BlueprintCallable, Category = "Arena")
	void AddScoreAndScaleDifficulty();

private:
	// Functions for spawning
	void SpawnPellet();
	void SpawnEnemy();

	// Timer to keep spawning pellets forever
	FTimerHandle PelletTimerHandle;
};