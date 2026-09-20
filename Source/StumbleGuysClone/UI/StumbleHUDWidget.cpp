// Copyright (c) 2026. Built for portfolio. Free to study.

#include "UI/StumbleHUDWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "GameMode/StumbleGameState.h"

void UStumbleHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Update timer bar
	if (TimerBar && CurrentRoundTime > 0.0f)
	{
		// TimerBar progress goes from 1.0 to 0.0
		TimerBar->SetPercent(FMath::Clamp(CurrentRoundTime / 60.0f, 0.0f, 1.0f));
	}
}

void UStumbleHUDWidget::SetRoundTime(float RemainingTime)
{
	CurrentRoundTime = RemainingTime;
	
	if (TimerText)
	{
		int32 Minutes = FMath::FloorToInt(RemainingTime / 60.0f);
		int32 Seconds = FMath::FloorToInt(RemainingTime) % 60;
		TimerText->SetText(FText::FromString(FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds)));
	}
}

void UStumbleHUDWidget::SetPlayerCount(int32 AliveCount, int32 TotalCount)
{
	if (PlayerCountText)
	{
		PlayerCountText->SetText(FText::FromString(FString::Printf(TEXT("Alive: %d / %d"), AliveCount, TotalCount)));
	}
}

void UStumbleHUDWidget::SetScore(int32 Score)
{
	if (ScoreText)
	{
		ScoreText->SetText(FText::FromString(FString::Printf(TEXT("Score: %d"), Score)));
	}
}

void UStumbleHUDWidget::SetCollectibles(int32 Count)
{
	if (CollectiblesText)
	{
		CollectiblesText->SetText(FText::FromString(FString::Printf(TEXT("Stars: %d"), Count)));
	}
}