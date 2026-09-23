// Copyright (c) 2026. Built for portfolio. Free to study.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StumbleWinWidget.generated.h"

class UTextBlock;
class UButton;

/** Raised when the local player asks for another round. */
DECLARE_DELEGATE(FOnStumblePlayAgain);

/**
 * UStumbleWinWidget — end-of-round result screen.
 *
 * Presentational only: it renders the outcome and forwards the "play again"
 * intent to the owning game mode. Restarting the round is authoritative, so the
 * widget never mutates game state itself.
 *
 * Widgets are built in Blueprint (WBP_StumbleWinWidget) and bind to the optional
 * named slots below; every reference is null-checked so a partial layout still
 * runs.
 */
UCLASS(Abstract)
class STUMBLEGUYSCLONE_API UStumbleWinWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual bool Initialize() override;

	/** Populate the result screen. bLocalPlayerWon drives the headline styling. */
	UFUNCTION(BlueprintCallable, Category = "Win")
	void SetWinnerInfo(const FString& WinnerName, bool bLocalPlayerWon, int32 FinalScore);

	/** Bind the play-again intent (typically AStumbleGameMode::RestartRound). */
	void SetOnPlayAgainClicked(FOnStumblePlayAgain InCallback) { PlayAgainCallback = MoveTemp(InCallback); }

protected:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ResultText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> WinnerNameText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ScoreText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> PlayAgainButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> QuitButton;

	UFUNCTION()
	void HandlePlayAgainClicked();

	UFUNCTION()
	void HandleQuitClicked();

private:
	FOnStumblePlayAgain PlayAgainCallback;
};