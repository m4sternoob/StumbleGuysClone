// Copyright (c) 2026. Built for portfolio. Free to study.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "StumbleGameMode.generated.h"

class AStumbleCharacter;
class AStumbleArena;
class AStumbleGameState;
class AStumbleBotController;
class AStumbleObstacleMovingPlatform;
class AStumbleObstacleSpinner;
class UStumbleSoundManager;
class UStumbleWinWidget;

/**
 * AStumbleGameMode — authoritative round director (server only).
 *
 * Owns the round lifecycle (arena, obstacle layout, timer, elimination, win
 * conditions), player/bot spawning, and session-facing configuration. Every
 * mutating path is Authority-guarded; clients observe state through
 * AStumbleGameState and AStumblePlayerState.
 */
UCLASS()
class STUMBLEGUYSCLONE_API AStumbleGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AStumbleGameMode();

	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	// ---- Round lifecycle ----

	UFUNCTION(BlueprintCallable, Category = "Round")
	void StartRound();

	UFUNCTION(BlueprintCallable, Category = "Round")
	void EndRound(AStumbleCharacter* Winner = nullptr);

	/** Tear down the finished round and schedule a fresh one. */
	UFUNCTION(BlueprintCallable, Category = "Round")
	void RestartRound();

	UFUNCTION(BlueprintCallable, Category = "Round")
	void EliminatePlayer(AStumbleCharacter* Character);

	// ---- Spawning ----

	UFUNCTION(BlueprintCallable, Category = "Player")
	AStumbleCharacter* SpawnPlayerForController(APlayerController* Controller);

	UFUNCTION(BlueprintCallable, Category = "Obstacles")
	void SpawnObstacles();

	/** Spawn up to Count wandering bots, respecting MaxBots. */
	UFUNCTION(BlueprintCallable, Category = "Bots")
	void SpawnBots(int32 Count);

	/** Remove up to Count bots (LIFO). */
	UFUNCTION(BlueprintCallable, Category = "Bots")
	void RemoveBots(int32 Count);

	/** Reconcile live bot count against MaxBots minus human players. */
	UFUNCTION(BlueprintCallable, Category = "Bots")
	void UpdateBotCount();

	// ---- Queries ----

	UFUNCTION(BlueprintCallable, Category = "Arena")
	AStumbleArena* GetArena() const { return Arena; }

	/** Points used both for obstacle placement and player respawns. */
	const TArray<FVector>& GetObstacleSpawnPoints() const { return ObstacleSpawnPoints; }

	// ---- Configuration ----

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Round", meta = (ClampMin = "10.0"))
	float RoundDuration = 60.0f;

	/** Grace period after the round ends before the next one starts. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Round", meta = (ClampMin = "0.0"))
	float RoundRestartDelay = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	TSubclassOf<AStumbleCharacter> CharacterClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena")
	TSubclassOf<AStumbleArena> ArenaClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacles")
	TSubclassOf<AStumbleObstacleMovingPlatform> MovingPlatformClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacles")
	TSubclassOf<AStumbleObstacleSpinner> SpinnerClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bots", meta = (ClampMin = "0"))
	int32 MaxBots = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bots")
	TSubclassOf<AStumbleBotController> BotControllerClass;

	/** Win-screen widget class shown to every player when a round ends. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UStumbleWinWidget> WinWidgetClass;

	/** Single audio configuration asset, applied to every spawned character. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Juice")
	TObjectPtr<UStumbleSoundManager> SoundSet;

	// ---- Spawn-point configuration ----

	/** Vertical offset applied to every spawn point, matching capsule half-height. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena", meta = (ClampMin = "0.0"))
	float SpawnHeight = 150.0f;

protected:
	/** Generated spawn ring: one centre point plus a fan of outer points. */
	UPROPERTY()
	TArray<FVector> ObstacleSpawnPoints;

	UPROPERTY()
	TObjectPtr<AStumbleArena> Arena;

	UPROPERTY()
	TArray<TObjectPtr<AActor>> SpawnedObstacles;

	UPROPERTY()
	TArray<TObjectPtr<AStumbleBotController>> SpawnedBots;

	UPROPERTY()
	TArray<FLinearColor> PlayerColors;

	int32 NextColorIndex = 0;

	FTimerHandle RoundTimerHandle;

	void GenerateObstacleSpawnPoints();
	void AssignPlayerColor(AStumbleCharacter* Character);
	FLinearColor GetNextColor();

	/** Destroy every tracked obstacle. Safe to call repeatedly. */
	void ClearObstacles();

private:
	/** Push the shared audio config onto a freshly spawned character. */
	void ApplySoundSet(AStumbleCharacter* Character) const;
};