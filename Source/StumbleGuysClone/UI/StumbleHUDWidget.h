// Copyright (c) 2026. Built for portfolio. Free to study.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StumbleHUDWidget.generated.h"

class UTextBlock;
class UProgressBar;
class AStumbleGameState;

UCLASS()
class STUMBLEGUYSCLONE_API UStumbleHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetRoundTime(float RemainingTime);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetPlayerCount(int32 AliveCount, int32 TotalCount);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetScore(int32 Score);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetCollectibles(int32 Count);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TimerText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> TimerBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PlayerCountText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ScoreText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CollectiblesText;

private:
	float CurrentRoundTime = 0.0f;
};