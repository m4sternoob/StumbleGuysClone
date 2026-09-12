// Copyright (c) 2026. Built for portfolio. Free to study.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "StumbleGameMode.generated.h"

class AStumbleCharacter;
class AStumbleArena;
class AStumbleGameState;

UCLASS()
class STUMBLEGUYSCLONE_API AStumbleGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AStumbleGameMode();

	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	// Round management
	UFUNCTION(BlueprintCallable, Category = "Round")
	void StartRound();

	UFUNCTION(BlueprintCallable, Category = "Round")
	void EndRound(AStumbleCharacter* Winner = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Round")
	void EliminatePlayer(AStumbleCharacter* Character);

	// Player spawning
	UFUNCTION(BlueprintCallable, Category = "Player")
	AStumbleCharacter* SpawnPlayerForController(APlayerController* Controller);

	// Arena
	UFUNCTION(BlueprintCallable, Category = "Arena")
	AStumbleArena* GetArena() const { return Arena; }

	// Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Round", meta = (AllowPrivateAccess = "true", ClampMin = "10.0"))
	float RoundDuration = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Round", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AStumbleCharacter> CharacterClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AStumbleArena> ArenaClass;

protected:
	UPROPERTY()
	TObjectPtr<AStumbleArena> Arena;

	UPROPERTY(Replicated)
	TObjectPtr<AStumbleGameState> StumbleGameState;

	FTimerHandle RoundTimerHandle;

	// Player colors for multiplayer
	UPROPERTY()
	TArray<FLinearColor> PlayerColors;

	int32 NextColorIndex = 0;

	void AssignPlayerColor(AStumbleCharacter* Character);
	FLinearColor GetNextColor();
};