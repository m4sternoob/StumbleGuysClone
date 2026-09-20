// Copyright (c) 2026. Built for portfolio. Free to study.

#include "UI/StumbleWinWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void UStumbleWinWidget::SetWinnerInfo(const FString& WinnerName, bool bLocalPlayerWon, int32 FinalScore)
{
	if (ResultText)
	{
		ResultText->SetText(FText::FromString(bLocalPlayerWon ? TEXT("VICTORY!") : TEXT("ELIMINATED")));
		ResultText->SetColorAndOpacity(bLocalPlayerWon ? FLinearColor(0.2f, 1.0f, 0.3f) : FLinearColor(1.0f, 0.2f, 0.2f));
	}

	if (WinnerNameText)
	{
		WinnerNameText->SetText(FText::FromString(FString::Printf(TEXT("Winner: %s"), *WinnerName)));
	}

	if (ScoreText)
	{
		ScoreText->SetText(FText::FromString(FString::Printf(TEXT("Final Score: %d"), FinalScore)));
	}
}

void UStumbleWinWidget::SetOnPlayAgainClicked(FOnClickedCallback Callback)
{
	PlayAgainCallback = Callback;
}

void UStumbleWinWidget::OnPlayAgainClicked()
{
	if (PlayAgainCallback.IsBound())
	{
		PlayAgainCallback.Execute();
	}
}

void UStumbleWinWidget::OnQuitClicked()
{
	// Return to main menu / lobby
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->ConsoleCommand(TEXT("OpenLevel /Game/Maps/LobbyLevel"));
	}
}