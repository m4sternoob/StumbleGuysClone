// Copyright (c) 2026. Built for portfolio. Free to study.

#include "GameMode/StumbleGameMode.h"
#include "GameMode/StumbleGameState.h"
#include "Character/StumbleCharacter.h"
#include "Arena/StumbleArena.h"
#include "PlayerController/StumblePlayerController.h"
#include "Engine/World.h"
#include "GameFramework/PlayerStart.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"

AStumbleGameMode::AStumbleGameMode()
{
	// Default classes (can be overridden in Blueprint)
	static ConstructorHelpers::FClassFinder<AStumbleCharacter> CharacterBP(TEXT("/Game/Blueprints/BP_StumbleCharacter"));
	if (CharacterBP.Class) CharacterClass = CharacterBP.Class;

	static ConstructorHelpers::FClassFinder<AStumbleArena> ArenaBP(TEXT("/Game/Blueprints/BP_StumbleArena"));
	if (ArenaBP.Class) ArenaClass = ArenaBP.Class;

	// Player colors for up to 4 players
	PlayerColors = {
		FLinearColor(1.0f, 0.2f, 0.2f, 1.0f),  // Red
		FLinearColor(0.2f, 0.4f, 1.0f, 1.0f),  // Blue
		FLinearColor(0.2f, 1.0f, 0.3f, 1.0f),  // Green
		FLinearColor(1.0f, 0.9f, 0.2f, 1.0f)   // Yellow
	};

	bReplicates = true;
}

void AStumbleGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Spawn arena
	if (ArenaClass && HasAuthority())
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		Arena = GetWorld()->SpawnActor<AStumbleArena>(ArenaClass, FTransform::Identity, SpawnParams);
	}

	// Start round after a brief delay for clients to connect
	GetWorldTimerManager().SetTimer(RoundTimerHandle, this, &AStumbleGameMode::StartRound, 3.0f, false);
}

void AStumbleGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (HasAuthority())
	{
		// Spawn character for new player
		AStumbleCharacter* Character = SpawnPlayerForController(NewPlayer);
		if (Character)
		{
			AssignPlayerColor(Character);
		}
	}
}

void AStumbleGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	// Character destruction handled by engine
}

void AStumbleGameMode::StartRound()
{
	if (!HasAuthority()) return;

	if (AStumbleGameState* GS = GetGameState<AStumbleGameState>())
	{
		GS->SetRoundState(EStumbleRoundState::InProgress);
		GS->SetRoundEndTime(GetWorld()->GetTimeSeconds() + RoundDuration);
	}

	// Set timer for round end
	GetWorldTimerManager().SetTimer(RoundTimerHandle, this, &AStumbleGameMode::EndRound, RoundDuration, false);
}

void AStumbleGameMode::EndRound(AStumbleCharacter* Winner)
{
	if (!HasAuthority()) return;

	GetWorldTimerManager().ClearTimer(RoundTimerHandle);

	if (AStumbleGameState* GS = GetGameState<AStumbleGameState>())
	{
		GS->SetRoundState(EStumbleRoundState::Ended);
		if (Winner)
		{
			GS->SetWinner(Winner);
		}
	}

	// TODO: Show win screen, handle restart
	UE_LOG(LogTemp, Log, TEXT("Round ended. Winner: %s"), Winner ? *Winner->GetName() : TEXT("None"));
}

void AStumbleGameMode::EliminatePlayer(AStumbleCharacter* Character)
{
	if (!HasAuthority() || !Character) return;

	Character->SetActorEnableCollision(false);
	Character->GetMesh()->SetVisibility(false);
	Character->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	// Disable input
	if (APlayerController* PC = Cast<APlayerController>(Character->GetController()))
	{
		PC->SetIgnoreMoveInput(true);
		PC->SetIgnoreLookInput(true);
	}

	// Check remaining players
	int32 AliveCount = 0;
	AStumbleCharacter* LastAlive = nullptr;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC && PC->GetPawn())
		{
			AStumbleCharacter* C = Cast<AStumbleCharacter>(PC->GetPawn());
			if (C && C->GetCapsuleComponent()->IsCollisionEnabled())
			{
				AliveCount++;
				LastAlive = C;
			}
		}
	}

	if (AliveCount <= 1)
	{
		EndRound(LastAlive);
	}
}

AStumbleCharacter* AStumbleGameMode::SpawnPlayerForController(APlayerController* Controller)
{
	if (!Controller || !CharacterClass || !HasAuthority()) return nullptr;

	// Find player start
	AActor* StartSpot = FindPlayerStart(Controller);
	FTransform SpawnTransform = StartSpot ? StartSpot->GetActorTransform() : FTransform(FRotator::ZeroRotator, FVector(0, 0, 150));

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	SpawnParams.Instigator = Controller;

	AStumbleCharacter* Character = GetWorld()->SpawnActor<AStumbleCharacter>(CharacterClass, SpawnTransform, SpawnParams);
	if (Character)
	{
		Controller->Possess(Character);
	}

	return Character;
}

void AStumbleGameMode::AssignPlayerColor(AStumbleCharacter* Character)
{
	if (!Character) return;

	FLinearColor Color = GetNextColor();
	Character->PlayerColor = Color;
	// OnRep_PlayerColor will be called on clients automatically
}

FLinearColor AStumbleGameMode::GetNextColor()
{
	if (PlayerColors.Num() == 0) return FLinearColor::White;
	
	FLinearColor Color = PlayerColors[NextColorIndex % PlayerColors.Num()];
	NextColorIndex++;
	return Color;
}

void AStumbleGameMode::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AStumbleGameMode, StumbleGameState);
}