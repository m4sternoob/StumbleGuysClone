// Copyright (c) 2026. Built for portfolio. Free to study.

#include "GameMode/StumbleGameMode.h"
#include "GameMode/StumbleGameState.h"
#include "Character/StumbleCharacter.h"
#include "Arena/StumbleArena.h"
#include "PlayerController/StumblePlayerController.h"
#include "AI/StumbleBotController.h"
#include "Obstacles/StumbleObstacleMovingPlatform.h"
#include "Obstacles/StumbleObstacleSpinner.h"
#include "UI/StumbleWinWidget.h"
#include "UI/StumbleHUDWidget.h"
#include "Engine/World.h"
#include "GameFramework/PlayerStart.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
#include "Math/UnrealMathUtility.h"

AStumbleGameMode::AStumbleGameMode()
{
	// Default classes (can be overridden in Blueprint)
	static ConstructorHelpers::FClassFinder<AStumbleCharacter> CharacterBP(TEXT("/Game/Blueprints/BP_StumbleCharacter"));
	if (CharacterBP.Class) CharacterClass = CharacterBP.Class;

	static ConstructorHelpers::FClassFinder<AStumbleArena> ArenaBP(TEXT("/Game/Blueprints/BP_StumbleArena"));
	if (ArenaBP.Class) ArenaClass = ArenaBP.Class;

	static ConstructorHelpers::FClassFinder<UStumbleWinWidget> WinWidgetBP(TEXT("/Game/Blueprints/BP_StumbleWinWidget"));
	if (WinWidgetBP.Class) WinWidgetClass = WinWidgetBP.Class;

	static ConstructorHelpers::FClassFinder<UStumbleHUDWidget> HUDWidgetBP(TEXT("/Game/Blueprints/BP_StumbleHUDWidget"));
	if (HUDWidgetBP.Class) HUDWidgetClass = HUDWidgetBP.Class;

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

	// Generate obstacle spawn points
	GenerateObstacleSpawnPoints();

	// Start round after a brief delay for clients to connect
	GetWorldTimerManager().SetTimer(RoundTimerHandle, this, &AStumbleGameMode::StartRound, 3.0f, false);
}

void AStumbleGameMode::GenerateObstacleSpawnPoints()
{
	if (!Arena) return;

	const float HalfSize = Arena->ArenaSize / 2.0f;
	const float Margin = 500.0f; // Keep away from walls
	const float SafeSize = HalfSize - Margin;

	// Generate 6-8 spawn points around the arena
	const int32 NumPoints = 7;
	ObstacleSpawnPoints.Empty();
	ObstacleSpawnPoints.Reserve(NumPoints);

	// Center point
	ObstacleSpawnPoints.Add(FVector(0.0f, 0.0f, 150.0f));

	// Around the center in a circle
	for (int32 i = 0; i < NumPoints - 1; ++i)
	{
		float Angle = (float)i / (NumPoints - 1) * 2.0f * PI;
		float Radius = SafeSize * 0.6f;
		FVector Point = FVector(
			FMath::Cos(Angle) * Radius,
			FMath::Sin(Angle) * Radius,
			150.0f
		);
		ObstacleSpawnPoints.Add(Point);
	}
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

	// Spawn obstacles
	SpawnObstacles();

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

	// Clean up obstacles
	for (AActor* Obstacle : SpawnedObstacles)
	{
		if (Obstacle && !Obstacle->IsPendingKill())
		{
			Obstacle->Destroy();
		}
	}
	SpawnedObstacles.Empty();

	if (AStumbleGameState* GS = GetGameState<AStumbleGameState>())
	{
		GS->SetRoundState(EStumbleRoundState::Ended);
		if (Winner)
		{
			GS->SetWinner(Winner);
			
			// Award win to player state
			if (APlayerController* PC = Cast<APlayerController>(Winner->GetController()))
			{
				if (AStumblePlayerState* PS = PC->GetPlayerState<AStumblePlayerState>())
				{
					PS->RecordWin();
				}
			}
		}
	}

	// Show win screen for all players
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC && WinWidgetClass)
		{
			UStumbleWinWidget* WinWidget = Cast<UStumbleWinWidget>(PC->GetHUD());
			if (!WinWidget)
			{
				WinWidget = CreateWidget<UStumbleWinWidget>(PC, WinWidgetClass);
				if (WinWidget)
				{
					WinWidget->AddToViewport();
				}
			}
			
			if (WinWidget)
			{
				bool bLocalWon = (Winner && Winner->GetController() == WinWidget->GetOwningPlayer());
				WinWidget->SetWinnerInfo(
					Winner ? Winner->GetName() : TEXT("None"),
					bLocalWon,
					bLocalWon ? 100 : 0 // Score placeholder
				);
				
				WinWidget->SetOnPlayAgainClicked([this]()
				{
					RestartRound();
				});
			}
		}
	}

		UE_LOG(LogTemp, Log, TEXT("Round ended. Winner: %s"), Winner ? *Winner->GetName() : TEXT("None"));
	}

	void AStumbleGameMode::SpawnBots(int32 Count)
	{
		if (!HasAuthority() || !GetWorld() || !BotControllerClass) return;

		int32 CurrentPlayers = 0;
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			if (It->Get() && It->Get()->GetPawn())
			{
				CurrentPlayers++;
			}
		}

		int32 BotsToSpawn = FMath::Min(Count, MaxBots - SpawnedBots.Num());
		BotsToSpawn = FMath::Max(0, BotsToSpawn);

		for (int32 i = 0; i < BotsToSpawn; ++i)
		{
			// Find spawn location
			FVector SpawnLoc = FVector(0.0f, 0.0f, 150.0f);
			if (ObstacleSpawnPoints.Num() > 0)
			{
				int32 Index = FMath::RandRange(0, ObstacleSpawnPoints.Num() - 1);
				FVector SpawnPoint = ObstacleSpawnPoints[FMath::RandRange(0, ObstacleSpawnPoints.Num() - 1)];
				SpawnPoint.Z = 150.0f;
			}

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			// Spawn bot character first
			if (CharacterClass)
			{
				FVector BotSpawnLoc = FVector(
					FMath::RandRange(-1000.0f, 1000.0f),
					FMath::RandRange(-1000.0f, 1000.0f),
					150.0f
				);

				FActorSpawnParameters CharParams;
				CharParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				AStumbleCharacter* BotCharacter = GetWorld()->SpawnActor<AStumbleCharacter>(CharacterClass, FTransform(FRotator::ZeroRotator, FVector(
					FMath::RandRange(-1000.0f, 1000.0f),
					FMath::RandRange(-1000.0f, 1000.0f),
					150.0f
				)), CharParams);

				if (BotCharacter)
				{
					// Spawn AI controller
					AStumbleBotController* BotController = GetWorld()->SpawnActor<AStumbleBotController>(BotControllerClass);
					if (BotController)
					{
						BotController->Possess(BotCharacter);
						SpawnedBots.Add(BotController);

						// Assign bot color
						AssignPlayerColor(BotCharacter);
					}
				}
			}
		}
	}

	void AStumbleGameMode::RemoveBots(int32 Count)
	{
		if (!HasAuthority()) return;

		int32 BotsToRemove = FMath::Min(Count, SpawnedBots.Num());
		for (int32 i = 0; i < BotsToRemove; ++i)
		{
			if (SpawnedBots.Num() > 0)
			{
				AStumbleBotController* Bot = SpawnedBots.Pop();
				if (Bot && Bot->GetPawn())
				{
					Bot->GetPawn()->Destroy();
				}
				if (Bot)
				{
					Bot->Destroy();
				}
			}
		}
	}

	void AStumbleGameMode::UpdateBotCount()
	{
		if (!HasAuthority()) return;

		int32 HumanPlayers = 0;
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			if (It->Get() && It->Get()->IsPlayerController())
			{
				HumanPlayers++;
			}
		}

		int32 TargetBots = FMath::Max(0, MaxBots - HumanPlayers);
		int32 CurrentBots = SpawnedBots.Num();

		if (CurrentBots < TargetBots)
		{
			SpawnBots(TargetBots - CurrentBots);
		}
		else if (CurrentBots > TargetBots)
		{
			RemoveBots(CurrentBots - TargetBots);
		}
	}

void AStumbleGameMode::RestartRound()
{
	if (!HasAuthority()) return;
	
	// Clean up obstacles
	for (AActor* Obstacle : SpawnedObstacles)
	{
		if (Obstacle && !Obstacle->IsPendingKill())
		{
			Obstacle->Destroy();
		}
	}
	SpawnedObstacles.Empty();
	
	// Respawn all players
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC && PC->GetPawn())
		{
			AStumbleCharacter* Character = Cast<AStumbleCharacter>(PC->GetPawn());
			if (Character && Character->bIsRespawning)
			{
				// Already respawning, will respawn normally
			}
			else if (Character && Character->bIsEliminated)
			{
				Character->StartRespawn();
			}
		}
	}
	
	// Restart round timer
	GetWorldTimerManager().SetTimer(RoundTimerHandle, this, &AStumbleGameMode::StartRound, 3.0f, false);
}

void AStumbleGameMode::EliminatePlayer(AStumbleCharacter* Character)
{
	if (!HasAuthority() || !Character) return;

	// Start respawn process instead of permanent elimination
	Character->StartRespawn();
}

void AStumbleGameMode::SpawnObstacles()
{
	if (!HasAuthority() || !GetWorld()) return;
	if (!MovingPlatformClass || !SpinnerClass) return;

	// Clear previous obstacles
	for (AActor* Obstacle : SpawnedObstacles)
	{
		if (Obstacle && !Obstacle->IsPendingKill())
		{
			Obstacle->Destroy();
		}
	}
	SpawnedObstacles.Empty();

	if (ObstacleSpawnPoints.Num() == 0) return;

	// Shuffle spawn points
	TArray<FVector> ShuffledPoints = ObstacleSpawnPoints;
	for (int32 i = ShuffledPoints.Num() - 1; i > 0; --i)
	{
		int32 j = FMath::RandRange(0, i);
		ShuffledPoints.Swap(i, j);
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Spawn moving platforms (3-4)
	int32 NumPlatforms = FMath::RandRange(3, 4);
	for (int32 i = 0; i < NumPlatforms && i < ShuffledPoints.Num(); ++i)
	{
		if (MovingPlatformClass)
		{
			FVector SpawnLoc = ShuffledPoints[i];
			SpawnLoc.Z = 150.0f;

			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			AStumbleObstacleMovingPlatform* Platform = GetWorld()->SpawnActor<AStumbleObstacleMovingPlatform>(MovingPlatformClass, FTransform(FRotator::ZeroRotator, SpawnLoc), Params);
			if (Platform)
			{
				// Randomize platform properties
				Platform->MoveAxis = FVector(FMath::RandBool() ? 1.0f : 0.0f, FMath::RandBool() ? 1.0f : 0.0f, 0.0f);
				if (Platform->MoveAxis.IsNearlyZero())
				{
					Platform->MoveAxis = FVector(1.0f, 0.0f, 0.0f);
				}
				Platform->MoveDistance = FMath::RandRange(800.0f, 1500.0f);
				Platform->MoveSpeed = FMath::RandRange(300.0f, 500.0f);
				Platform->bPingPong = true;

				SpawnedObstacles.Add(Platform);
			}
		}
	}

	// Spawn spinners (2-3)
	int32 NumSpinners = FMath::RandRange(2, 3);
	int32 SpinnerStartIdx = FMath::Min(NumPlatforms, ShuffledPoints.Num() - 1);
	for (int32 i = 0; i < NumSpinners && (SpinnerStartIdx + i) < ShuffledPoints.Num(); ++i)
	{
		if (SpinnerClass)
		{
			FVector SpawnLoc = ShuffledPoints[SpinnerStartIdx + i];
			SpawnLoc.Z = 150.0f;

			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			AStumbleObstacleSpinner* Spinner = GetWorld()->SpawnActor<AStumbleObstacleSpinner>(SpinnerClass, FTransform(FRotator::ZeroRotator, SpawnLoc), Params);
			if (Spinner)
			{
				// Randomize spinner properties
				Spinner->RotationSpeed = FMath::RandRange(60.0f, 150.0f);
				Spinner->bReverseDirection = FMath::RandBool();
				Spinner->BarLength = FMath::RandRange(800.0f, 1400.0f);
				Spinner->BarThickness = FMath::RandRange(60.0f, 100.0f);

				SpawnedObstacles.Add(Spinner);
			}
		}
	}
}

void AStumbleGameMode::EndRound(AStumbleCharacter* Winner)
{
	if (!HasAuthority()) return;

	GetWorldTimerManager().ClearTimer(RoundTimerHandle);

	// Clean up obstacles
	for (AActor* Obstacle : SpawnedObstacles)
	{
		if (Obstacle && !Obstacle->IsPendingKill())
		{
			Obstacle->Destroy();
		}
	}
	SpawnedObstacles.Empty();

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

	void AStumbleGameMode::SpawnBots(int32 Count)
	{
		if (!HasAuthority() || !GetWorld() || !BotControllerClass) return;

		int32 CurrentPlayers = 0;
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			if (It->Get() && It->Get()->GetPawn())
			{
				CurrentPlayers++;
			}
		}

		int32 BotsToSpawn = FMath::Min(Count, MaxBots - SpawnedBots.Num());
		BotsToSpawn = FMath::Max(0, BotsToSpawn);

		for (int32 i = 0; i < BotsToSpawn; ++i)
		{
			// Find spawn location
			FVector SpawnLoc = FVector(0.0f, 0.0f, 150.0f);
			if (ObstacleSpawnPoints.Num() > 0)
			{
				int32 Index = FMath::RandRange(0, ObstacleSpawnPoints.Num() - 1);
				FVector SpawnPoint = ObstacleSpawnPoints[FMath::RandRange(0, ObstacleSpawnPoints.Num() - 1)];
				SpawnPoint.Z = 150.0f;
			}

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			// Spawn bot character first
			if (CharacterClass)
			{
				FVector BotSpawnLoc = FVector(
					FMath::RandRange(-1000.0f, 1000.0f),
					FMath::RandRange(-1000.0f, 1000.0f),
					150.0f
				);

				FActorSpawnParameters CharParams;
				CharParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				AStumbleCharacter* BotCharacter = GetWorld()->SpawnActor<AStumbleCharacter>(CharacterClass, FTransform(FRotator::ZeroRotator, FVector(
					FMath::RandRange(-1000.0f, 1000.0f),
					FMath::RandRange(-1000.0f, 1000.0f),
					150.0f
				)), CharParams);

				if (BotCharacter)
				{
					// Spawn AI controller
					AStumbleBotController* BotController = GetWorld()->SpawnActor<AStumbleBotController>(BotControllerClass);
					if (BotController)
					{
						BotController->Possess(BotCharacter);
						SpawnedBots.Add(BotController);

						// Assign bot color
						AssignPlayerColor(BotCharacter);
					}
				}
			}
		}
	}

	void AStumbleGameMode::RemoveBots(int32 Count)
	{
		if (!HasAuthority()) return;

		int32 BotsToRemove = FMath::Min(Count, SpawnedBots.Num());
		for (int32 i = 0; i < BotsToRemove; ++i)
		{
			if (SpawnedBots.Num() > 0)
			{
				AStumbleBotController* Bot = SpawnedBots.Pop();
				if (Bot && Bot->GetPawn())
				{
					Bot->GetPawn()->Destroy();
				}
				if (Bot)
				{
					Bot->Destroy();
				}
			}
		}
	}

	void AStumbleGameMode::UpdateBotCount()
	{
		if (!HasAuthority()) return;

		int32 HumanPlayers = 0;
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			if (It->Get() && It->Get()->IsPlayerController())
			{
				HumanPlayers++;
			}
		}

		int32 TargetBots = FMath::Max(0, MaxBots - HumanPlayers);
		int32 CurrentBots = SpawnedBots.Num();

		if (CurrentBots < TargetBots)
		{
			SpawnBots(TargetBots - CurrentBots);
		}
		else if (CurrentBots > TargetBots)
		{
			RemoveBots(CurrentBots - TargetBots);
		}
	}