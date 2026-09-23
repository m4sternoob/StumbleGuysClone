// Copyright (c) 2026. Built for portfolio. Free to study.

#include "UI/StumbleWinWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"

namespace
{
	/** Headline styling for the two outcomes. */
	const FLinearColor WinColor(0.25f, 1.0f, 0.35f, 1.0f);
	const FLinearColor LoseColor(1.0f, 0.25f, 0.25f, 1.0f);
}

bool UStumbleWinWidget::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	if (PlayAgainButton)
	{
		PlayAgainButton->OnClicked.AddDynamic(this, &UStumbleWinWidget::HandlePlayAgainClicked);
	}
	if (QuitButton)
	{
		QuitButton->OnClicked.AddDynamic(this, &UStumbleWinWidget::HandleQuitClicked);
	}

	return true;
}

void UStumbleWinWidget::SetWinnerInfo(const FString& WinnerName, bool bLocalPlayerWon, int32 FinalScore)
{
	if (ResultText)
	{
		ResultText->SetText(FText::FromString(bLocalPlayerWon ? TEXT("VICTORY") : TEXT("ELIMINATED")));
		ResultText->SetColorAndOpacity(bLocalPlayerWon ? WinColor : LoseColor);
	}

	if (WinnerNameText)
	{
		// A null winner means the round timed out with everyone still alive.
		const FText WinnerLabel = WinnerName.IsEmpty()
			? FText::FromString(TEXT("Round over"))
			: FText::FromString(FString::Printf(TEXT("Winner: %s"), *WinnerName));
		WinnerNameText->SetText(WinnerLabel);
	}

	if (ScoreText)
	{
		ScoreText->SetText(FText::FromString(FString::Printf(TEXT("Final Score: %d"), FinalScore)));
	}
}

void UStumbleWinWidget::HandlePlayAgainClicked()
{
	// Restarting is authoritative: the game mode re-validates before acting.
	if (PlayAgainCallback.IsBound())
	{
		PlayAgainCallback.Execute();
	}

	RemoveFromParent();
}

void UStumbleWinWidget::HandleQuitClicked()
{
	// No dedicated menu level exists yet in the prototype, so dismiss the screen
	// and let the player quit through the platform UI.
	RemoveFromParent();
}