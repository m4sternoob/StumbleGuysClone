// Copyright (c) 2026. Built for portfolio. Free to study.

#include "GameMode/StumbleGameState.h"
#include "Character/StumbleCharacter.h"
#include "Net/UnrealNetwork.h"

AStumbleGameState::AStumbleGameState()
{
	bReplicates = true;
}

void AStumbleGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AStumbleGameState, RoundState);
	DOREPLIFETIME(AStumbleGameState, RoundEndTime);
	DOREPLIFETIME(AStumbleGameState, Winner);
}

void AStumbleGameState::SetRoundState(EStumbleRoundState NewState)
{
	if (HasAuthority())
	{
		RoundState = NewState;
	}
}

void AStumbleGameState::SetRoundEndTime(float NewEndTime)
{
	if (HasAuthority())
	{
		RoundEndTime = NewEndTime;
	}
}

void AStumbleGameState::SetWinner(AStumbleCharacter* NewWinner)
{
	if (HasAuthority())
	{
		Winner = NewWinner;
	}
}

float AStumbleGameState::GetRemainingTime() const
{
	if (RoundState == EStumbleRoundState::InProgress)
	{
		return FMath::Max(0.0f, RoundEndTime - GetWorld()->GetTimeSeconds());
	}
	return 0.0f;
}

void AStumbleGameState::OnRep_RoundState()
{
	// Client-side round state change - update UI
}

void AStumbleGameState::OnRep_Winner()
{
	// Client-side winner announced - show win screen
}