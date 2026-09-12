// Copyright (c) 2026. Built for portfolio. Free to study.

#include "GameMode/StumblePlayerState.h"
#include "Net/UnrealNetwork.h"

AStumblePlayerState::AStumblePlayerState()
{
	bReplicates = true;
}

void AStumblePlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AStumblePlayerState, Score);
	DOREPLIFETIME(AStumblePlayerState, CollectiblesCollected);
	DOREPLIFETIME(AStumblePlayerState, RoundsWon);
	DOREPLIFETIME(AStumblePlayerState, TimesEliminated);
}

void AStumblePlayerState::AddScore(int32 Amount)
{
	if (HasAuthority())
	{
		Score += Amount;
	}
}

void AStumblePlayerState::AddCollectible()
{
	if (HasAuthority())
	{
		CollectiblesCollected++;
		Score += 10; // 10 points per collectible
	}
}

void AStumblePlayerState::RecordWin()
{
	if (HasAuthority())
	{
		RoundsWon++;
		Score += 100; // 100 points for winning
	}
}

void AStumblePlayerState::RecordElimination()
{
	if (HasAuthority())
	{
		TimesEliminated++;
	}
}