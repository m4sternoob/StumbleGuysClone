// Copyright (c) 2026. Built for portfolio. Free to study.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "StumblePlayerState.generated.h"

UCLASS()
class STUMBLEGUYSCLONE_API AStumblePlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AStumblePlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Score / collectibles
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Gameplay")
	int32 Score = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Gameplay")
	int32 CollectiblesCollected = 0;

	// Round stats
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Gameplay")
	int32 RoundsWon = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Gameplay")
	int32 TimesEliminated = 0;

	UFUNCTION(BlueprintCallable, Category = "Gameplay")
	void AddScore(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Gameplay")
	void AddCollectible();

	UFUNCTION(BlueprintCallable, Category = "Gameplay")
	void RecordWin();

	UFUNCTION(BlueprintCallable, Category = "Gameplay")
	void RecordElimination();
};