// Copyright (c) 2026. Built for portfolio. Free to study.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StumbleWinWidget.generated.h"

class UTextBlock;
class UButton;
class AStumbleCharacter;

UCLASS()
class STUMBLEGUYSCLONE_API UStumbleWinWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Win")
	void SetWinnerInfo(const FString& WinnerName, bool bLocalPlayerWon, int32 FinalScore);

	UFUNCTION(BlueprintCallable, Category = "Win")
	void SetOnPlayAgainClicked(FOnClickedCallback Callback);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ResultText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> WinnerNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ScoreText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> PlayAgainButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> QuitButton;

	UFUNCTION()
	void OnPlayAgainClicked();

	UFUNCTION()
	void OnQuitClicked();

private:
	FOnClickedCallback PlayAgainCallback;
};