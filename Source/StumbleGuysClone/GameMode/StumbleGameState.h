// Copyright (c) 2026. Built for portfolio. Free to study.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "StumbleGameState.generated.h"

class AStumbleCharacter;

UENUM(BlueprintType)
enum class EStumbleRoundState : uint8
{
	Waiting,
	InProgress,
	Ended
};

UCLASS()
class STUMBLEGUYSCLONE_API AStumbleGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AStumbleGameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Round")
	EStumbleRoundState GetRoundState() const { return RoundState; }

	UFUNCTION(BlueprintCallable, Category = "Round")
	void SetRoundState(EStumbleRoundState NewState);

	UFUNCTION(BlueprintCallable, Category = "Round")
	float GetRoundEndTime() const { return RoundEndTime; }

	UFUNCTION(BlueprintCallable, Category = "Round")
	void SetRoundEndTime(float NewEndTime);

	UFUNCTION(BlueprintCallable, Category = "Round")
	AStumbleCharacter* GetWinner() const { return Winner; }

	UFUNCTION(BlueprintCallable, Category = "Round")
	void SetWinner(AStumbleCharacter* NewWinner);

	UFUNCTION(BlueprintCallable, Category = "Round")
	float GetRemainingTime() const;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_RoundState)
	EStumbleRoundState RoundState = EStumbleRoundState::Waiting;

	UPROPERTY(Replicated)
	float RoundEndTime = 0.0f;

	UPROPERTY(Replicated)
	TObjectPtr<AStumbleCharacter> Winner = nullptr;

	UFUNCTION()
	void OnRep_RoundState();

	UFUNCTION()
	void OnRep_Winner();
};