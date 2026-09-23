// Copyright (c) 2026. Built for portfolio. Free to study.

#include "GameMode/StumbleGameMode.h"

#include "GameMode/StumbleGameState.h"
#include "GameMode/StumblePlayerState.h"
#include "Character/StumbleCharacter.h"
#include "Arena/StumbleArena.h"
#include "AI/StumbleBotController.h"
#include "Obstacles/StumbleObstacleMovingPlatform.h"
#include "Obstacles/StumbleObstacleSpinner.h"
#include "Effects/StumbleSoundManager.h"
#include "UI/StumbleWinWidget.h"

#include "Engine/World.h"
#include "GameFramework/PlayerStart.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	/** Vertical offset so spawned pawns clear the arena floor. */
	constexpr float GSpawnZ = 150.0f;

	/** Obstacle layout is randomised per round. */
	constexpr int32 GMinPlatformsPerRound = 3;
	constexpr int32 GMaxPlatformsPerRound = 4;
	constexpr int32 GMinSpinnersPerRound = 2;
	constexpr int32 GMaxSpinnersPerRound = 3;

	/** Number of outer spawn points, excluding the arena centre. */
	constexpr int32 GOuterSpawnPoints = 6;

	/** Keep spawn points this far inside the walls. */
	constexpr float GSpawnMargin = 500.0f;
}

AStumbleGameMode::AStumbleGameMode()
{
	// Blueprint subclasses are optional; the C++ classes are the fallback so the
	// project is playable straight from a fresh clone with no content assets.
	static ConstructorHelpers::FClassFinder<AStumbleCharacter> CharacterBP(TEXT("/Game/Blueprints/BP_StumbleCharacter"));
	if (CharacterBP.Class) CharacterClass = CharacterBP.Class;

	static ConstructorHelpers::FClassFinder<AStumbleArena> ArenaBP(TEXT("/Game/Blueprints/BP_StumbleArena"));
	if (ArenaBP.Class) ArenaClass = ArenaBP.Class;

	static ConstructorHelpers::FClassFinder<UStumbleWinWidget> WinWidgetBP(TEXT("/Game/Blueprints/BP_StumbleWinWidget"));
	if (WinWidgetBP.Class) WinWidgetClass = WinWidgetBP.Class;

	// Distinct, high-contrast colours so players can tell each other apart in a
	// four-way scramble. Ordered to maximise separation between adjacent slots.
	PlayerColors =
	{
		FLinearColor(1.00f, 0.20f, 0.20f, 1.0f), // Red
		FLinearColor(0.20f, 0.45f, 1.00f, 1.0f), // Blue
		FLinearColor(0.25f, 1.00f, 0.35f, 1.0f), // Green
		FLinearColor(1.00f, 0.90f, 0.20f, 1.0f)  // Yellow
	};

	bReplicates = true;
	GameStateClass = AStumbleGameState::StaticClass();
	PlayerStateClass = AStumblePlayerState::StaticClass();
}

void AStumbleGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		return;
	}

	// The arena must exist before spawn points can be derived from its size.
	if (ArenaClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		Arena = GetWorld()->SpawnActor<AStumbleArena>(ArenaClass, FTransform::Identity, SpawnParams);
	}

	GenerateObstacleSpawnPoints();

	// Fill empty slots with bots so a solo host still has opponents.
	UpdateBotCount();

	// Brief delay lets joining clients finish loading before the first round.
	GetWorldTimerManager().SetTimer(
		RoundTimerHandle, this, &AStumbleGameMode::StartRound, RoundRestartDelay, false);
}

void AStumbleGameMode::GenerateObstacleSpawnPoints()
{
	ObstacleSpawnPoints.Reset();
	if (!Arena)
	{
		return;
	}

	// One centre point plus a fan of points inside the walls. Reused for both
	// obstacle placement and player respawns, so it must stay walkable.
	const float SafeRadius = FMath::Max(Arena->ArenaSize * 0.5f - GSpawnMargin, 0.0f);

	ObstacleSpawnPoints.Reserve(GOuterSpawnPoints + 1);
	ObstacleSpawnPoints.Add(FVector(0.0f, 0.0f, GSpawnZ));

	for (int32 i = 0; i < GOuterSpawnPoints; ++i)
	{
		const float Angle = (static_cast<float>(i) / GOuterSpawnPoints) * 2.0f * PI;
		ObstacleSpawnPoints.Add(FVector(
			FMath::Cos(Angle) * SafeRadius,
			FMath::Sin(Angle) * SafeRadius,
			GSpawnZ));
	}
}

void AStumbleGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (!HasAuthority() || !NewPlayer)
	{
		return;
	}

	// AGameModeBase would spawn a default pawn; we spawn our own so the colour
	// assignment and audio config travel with it.
	if (AStumbleCharacter* Character = SpawnPlayerForController(NewPlayer))
	{
		AssignPlayerColor(Character);
	}

	// A human took a slot, so retire one bot to keep the match size stable.
	UpdateBotCount();
}

void AStumbleGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	if (!HasAuthority())
	{
		return;
	}

	// A human freed a slot — backfill it with a bot.
	UpdateBotCount();
}

void AStumbleGameMode::StartRound()
{
	if (!HasAuthority())
	{
		return;
	}

	SpawnObstacles();

	if (AStumbleGameState* GS = GetGameState<AStumbleGameState>())
	{
		GS->SetRoundState(EStumbleRoundState::InProgress);
		GS->SetRoundEndTime(GetWorld()->GetTimeSeconds() + RoundDuration);
	}

	if (SoundSet)
	{
		SoundSet->PlayCueAtLocation(GetWorld(), SoundSet->RoundStartSound, FVector::ZeroVector);
	}

	GetWorldTimerManager().SetTimer(
		RoundTimerHandle, this, &AStumbleGameMode::EndRound, RoundDuration, false);
}

void AStumbleGameMode::EndRound(AStumbleCharacter* Winner)
{
	if (!HasAuthority())
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(RoundTimerHandle);
	ClearObstacles();

	AStumbleGameState* GS = GetGameState<AStumbleGameState>();
	if (GS)
	{
		GS->SetRoundState(EStumbleRoundState::Ended);
		GS->SetWinner(Winner);
	}

	// Credit the win before the widget reads the score back out.
	if (Winner)
	{
		if (APlayerController* WinnerPC = Cast<APlayerController>(Winner->GetController()))
		{
			if (AStumblePlayerState* WinnerPS = WinnerPC->GetPlayerState<AStumblePlayerState>())
			{
				WinnerPS->RecordWin();
			}
		}
	}

	// Show a personalised result screen to every connected player.
	if (WinWidgetClass)
	{
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			APlayerController* PC = It->Get();
			if (!PC)
			{
				continue;
			}

			UStumbleWinWidget* WinWidget = CreateWidget<UStumbleWinWidget>(PC, WinWidgetClass);
			if (!WinWidget)
			{
				continue;
			}

			const bool bLocalWon = (Winner != nullptr && Winner->GetController() == PC);

			int32 FinalScore = 0;
			if (const AStumblePlayerState* PS = PC->GetPlayerState<AStumblePlayerState>())
			{
				FinalScore = PS->Score;
			}

			WinWidget->AddToViewport();
			WinWidget->SetWinnerInfo(
				Winner ? Winner->GetName() : TEXT("Nobody"),
				bLocalWon,
				FinalScore);

			// Play Again is authoritative: the request goes back to the server
			// rather than restarting anything locally.
			WinWidget->SetOnPlayAgainClicked([this]()
			{
				if (HasAuthority())
				{
					RestartRound();
				}
			});
		}
	}

	if (SoundSet)
	{
		SoundSet->PlayCueAtLocation(GetWorld(), SoundSet->RoundEndSound, FVector::ZeroVector);
	}

	UE_LOG(LogTemp, Log, TEXT("Stumble round ended. Winner: %s"),
		Winner ? *Winner->GetName() : TEXT("none"));
}

void AStumbleGameMode::RestartRound()
{
	if (!HasAuthority())
	{
		return;
	}

	ClearObstacles();

	// Return every pawn to a clean spawn state. Characters own their own respawn
	// state machine, so the director only has to nudge them.
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		AStumbleCharacter* Character = PC ? Cast<AStumbleCharacter>(PC->GetPawn()) : nullptr;
		if (Character && !Character->IsRespawning())
		{
			Character->StartRespawn();
		}
	}

	// Bots have no controller iterator entry of their own beyond their AIController,
	// so reconcile the roster as well.
	UpdateBotCount();

	GetWorldTimerManager().SetTimer(
		RoundTimerHandle, this, &AStumbleGameMode::StartRound, RoundRestartDelay, false);
}

void AStumbleGameMode::EliminatePlayer(AStumbleCharacter* Character)
{
	if (!HasAuthority() || !Character)
	{
		return;
	}

	// Elimination is soft in this prototype: the character respawns after a
	// delay rather than leaving the match.
	Character->StartRespawn();
}

void AStumbleGameMode::ClearObstacles()
{
	for (AActor* Obstacle : SpawnedObstacles)
	{
		if (IsValid(Obstacle))
		{
			Obstacle->Destroy();
		}
	}
	SpawnedObstacles.Reset();
}

AStumbleCharacter* AStumbleGameMode::SpawnPlayerForController(APlayerController* Controller)
{
	if (!HasAuthority() || !Controller || !CharacterClass)
	{
		return nullptr;
	}

	const AActor* StartSpot = FindPlayerStart(Controller);
	const FTransform SpawnTransform = StartSpot
		? StartSpot->GetActorTransform()
		: FTransform(FRotator::ZeroRotator, FVector(0.0f, 0.0f, GSpawnZ));

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	SpawnParams.Instigator = Controller;

	AStumbleCharacter* Character = GetWorld()->SpawnActor<AStumbleCharacter>(
		CharacterClass, SpawnTransform, SpawnParams);
	if (!Character)
	{
		return nullptr;
	}

	Controller->Possess(Character);
	ApplySoundSet(Character);
	return Character;
}

void AStumbleGameMode::SpawnObstacles()
{
	if (!HasAuthority())
	{
		return;
	}

	ClearObstacles();

	if (ObstacleSpawnPoints.Num() == 0 || (!MovingPlatformClass && !SpinnerClass))
	{
		return;
	}

	// Shuffle so consecutive rounds do not read as the same layout.
	TArray<FVector> Points = ObstacleSpawnPoints;
	for (int32 i = Points.Num() - 1; i > 0; --i)
	{
		Points.Swap(i, FMath::RandRange(0, i));
	}

	const int32 NumPlatforms = MovingPlatformClass
		? FMath::RandRange(GMinPlatformsPerRound, GMaxPlatformsPerRound) : 0;
	const int32 NumSpinners = SpinnerClass
		? FMath::RandRange(GMinSpinnersPerRound, GMaxSpinnersPerRound) : 0;
	const int32 Budget = FMath::Min(Points.Num(), NumPlatforms + NumSpinners);

	int32 NextPoint = 0;

	for (int32 i = 0; i < NumPlatforms && NextPoint < Budget; ++i, ++NextPoint)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AStumbleObstacleMovingPlatform* Platform =
			GetWorld()->SpawnActor<AStumbleObstacleMovingPlatform>(
				MovingPlatformClass, FTransform(FRotator::ZeroRotator, Points[NextPoint]), Params);
		if (!Platform)
		{
			continue;
		}

		// Randomise the motion so each round feels different. A pure-diagonal
		// axis is allowed but never a zero vector.
		FVector Axis(FMath::RandBool() ? 1.0f : 0.0f, FMath::RandBool() ? 1.0f : 0.0f, 0.0f);
		if (Axis.IsNearlyZero())
		{
			Axis = FVector(1.0f, 0.0f, 0.0f);
		}
		Platform->MoveAxis = Axis;
		Platform->MoveDistance = FMath::RandRange(800.0f, 1500.0f);
		Platform->MoveSpeed = FMath::RandRange(300.0f, 500.0f);
		Platform->bPingPong = true;

		SpawnedObstacles.Add(Platform);
	}

	for (int32 i = 0; i < NumSpinners && NextPoint < Budget; ++i, ++NextPoint)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AStumbleObstacleSpinner* Spinner = GetWorld()->SpawnActor<AStumbleObstacleSpinner>(
			SpinnerClass, FTransform(FRotator::ZeroRotator, Points[NextPoint]), Params);
		if (!Spinner)
		{
			continue;
		}

		Spinner->RotationSpeed = FMath::RandRange(60.0f, 150.0f);
		Spinner->bReverseDirection = FMath::RandBool();
		Spinner->BarLength = FMath::RandRange(800.0f, 1400.0f);
		Spinner->BarThickness = FMath::RandRange(60.0f, 100.0f);

		SpawnedObstacles.Add(Spinner);
	}
}

void AStumbleGameMode::SpawnBots(int32 Count)
{
	if (!HasAuthority() || !BotControllerClass || !CharacterClass || Count <= 0)
	{
		return;
	}

	const int32 BotsToSpawn = FMath::Clamp(Count, 0, MaxBots - SpawnedBots.Num());
	if (BotsToSpawn <= 0)
	{
		return;
	}

	// Bots drop in at spawn points when available, otherwise anywhere in the
	// arena interior — the AI will path itself back onto solid ground.
	const float ScatterExtent = Arena ? Arena->ArenaSize * 0.4f : 1000.0f;

	for (int32 i = 0; i < BotsToSpawn; ++i)
	{
		FVector SpawnLocation;
		if (ObstacleSpawnPoints.Num() > 0)
		{
			SpawnLocation = ObstacleSpawnPoints[FMath::RandRange(0, ObstacleSpawnPoints.Num() - 1)];
		}
		else
		{
			SpawnLocation = FVector(
				FMath::FRandRange(-ScatterExtent, ScatterExtent),
				FMath::FRandRange(-ScatterExtent, ScatterExtent),
				GSpawnZ);
		}

		FActorSpawnParameters CharParams;
		CharParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		AStumbleCharacter* BotCharacter = GetWorld()->SpawnActor<AStumbleCharacter>(
			CharacterClass, FTransform(FRotator::ZeroRotator, SpawnLocation), CharParams);
		if (!BotCharacter)
		{
			continue;
		}

		FActorSpawnParameters AIParams;
		AIParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AStumbleBotController* BotController =
			GetWorld()->SpawnActor<AStumbleBotController>(BotControllerClass, AIParams);
		if (!BotController)
		{
			BotCharacter->Destroy();
			continue;
		}

		BotController->Possess(BotCharacter);
		SpawnedBots.Add(BotController);

		AssignPlayerColor(BotCharacter);
		ApplySoundSet(BotCharacter);
	}
}

void AStumbleGameMode::RemoveBots(int32 Count)
{
	if (!HasAuthority())
	{
		return;
	}

	const int32 BotsToRemove = FMath::Clamp(Count, 0, SpawnedBots.Num());
	for (int32 i = 0; i < BotsToRemove; ++i)
	{
		AStumbleBotController* Bot = SpawnedBots.Pop();
		if (!IsValid(Bot))
		{
			continue;
		}

		if (APawn* BotPawn = Bot->GetPawn())
		{
			BotPawn->Destroy();
		}
		Bot->Destroy();
	}
}

void AStumbleGameMode::UpdateBotCount()
{
	if (!HasAuthority())
	{
		return;
	}

	int32 HumanPlayers = 0;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (It->Get() && It->Get()->IsPlayerController())
		{
			++HumanPlayers;
		}
	}

	const int32 TargetBots = FMath::Max(0, MaxBots - HumanPlayers);
	const int32 CurrentBots = SpawnedBots.Num();

	if (CurrentBots < TargetBots)
	{
		SpawnBots(TargetBots - CurrentBots);
	}
	else if (CurrentBots > TargetBots)
	{
		RemoveBots(CurrentBots - TargetBots);
	}
}

void AStumbleGameMode::AssignPlayerColor(AStumbleCharacter* Character)
{
	if (!Character)
	{
		return;
	}

	// Written on the server; AStumbleCharacter replicates PlayerColor out to
	// clients, which re-apply it via OnRep_PlayerColor.
	Character->SetPlayerColor(GetNextColor());
}

FLinearColor AStumbleGameMode::GetNextColor()
{
	if (PlayerColors.Num() == 0)
	{
		return FLinearColor::White;
	}

	// Round-robin: colours are reused once all slots are taken, which is fine
	// for a four-slot party game.
	const FLinearColor Color = PlayerColors[NextColorIndex % PlayerColors.Num()];
	++NextColorIndex;
	return Color;
}

void AStumbleGameMode::ApplySoundSet(AStumbleCharacter* Character) const
{
	if (Character && SoundSet)
	{
		Character->SetSoundSet(SoundSet);
	}
}