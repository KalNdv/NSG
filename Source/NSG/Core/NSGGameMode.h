#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Blueprint/UserWidget.h"
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

	// Enemy snake spawns
	UPROPERTY(EditDefaultsOnly, Category = "Arena Spawning")
	TSubclassOf<class ANSGSnakeBase> EnemySnakeClass;

	UPROPERTY(EditDefaultsOnly, Category = "Arena Spawning")
	TSubclassOf<class ANSGSnakeBase> OrangeSnakeClass;

	// Arena width
	UPROPERTY(EditAnywhere, Category = "Arena Spawning")
	float SpawnRadius = 2000.0f;

	// Pellet spawn per second
	UPROPERTY(EditAnywhere, Category = "Arena Spawning")
	float PelletSpawnRate = 0.5f;

	// How often a new enemy tries to spawn (in seconds)
	UPROPERTY(EditAnywhere, Category = "Arena Spawning")
	float EnemySpawnRate = 5.0f;

	// The maximum amount of enemies allowed alive at the same time
	UPROPERTY(EditAnywhere, Category = "Arena Spawning")
	int32 MaxEnemies = 5;

	// How many enemies to spawn at the start
	UPROPERTY(EditAnywhere, Category = "Arena Spawning")
	int32 StartingEnemies = 3;

	// Current starting difficulty
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Progression")
	float InitialDifficultyMultiplier = 1.0f;

	// Next level name to load
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Progression")
	FName NextLevelName;

	// Function for win widget (progress to next!)
	UFUNCTION(BlueprintCallable, Category = "Level Progression")
	void LoadNextLevel();

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

	// Timer to keep spawning stuff forever
	FTimerHandle PelletTimerHandle;
	FTimerHandle EnemyTimerHandle;

	// UI STUFF!
public:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UUserWidget> GameOverWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UUserWidget> WinWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UUserWidget> PauseWidgetClass;

	UPROPERTY()
	class UUserWidget* ActiveWidget;

	UPROPERTY(BlueprintReadWrite, Category = "Game State")
	bool bIsFreeplay = false;

	UFUNCTION(BlueprintCallable, Category = "Game State")
	void ShowGameOver();

	UFUNCTION(BlueprintCallable, Category = "Game State")
	void ShowWinScreen();

	UFUNCTION(BlueprintCallable, Category = "Game State")
	void TogglePause();

	UFUNCTION(BlueprintCallable, Category = "Game State")
	void StartFreeplay();
};