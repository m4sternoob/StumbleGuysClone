// Copyright (c) 2026. Built for portfolio. Free to study.

#include "PlayerController/StumblePlayerController.h"
#include "Character/StumbleCharacter.h"
#include "UI/StumbleLobbyWidget.h"
#include "UI/StumbleHUDWidget.h"
#include "UI/StumbleWinWidget.h"
#include "Blueprint/UserWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"

AStumblePlayerController::AStumblePlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

void AStumblePlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Get session interface
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		SessionInterface = OnlineSub->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &AStumblePlayerController::OnCreateSessionComplete);
			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &AStumblePlayerController::OnFindSessionsComplete);
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &AStumblePlayerController::OnJoinSessionComplete);
			SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &AStumblePlayerController::OnDestroySessionComplete);
		}
	}

	// Show lobby widget on start
	if (LobbyWidgetClass)
	{
		ShowLobbyWidget();
	}

	// Add UI input mapping context
	if (UIInputContext)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(UIInputContext, 10);
		}
	}
}

void AStumblePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (UIAcceptAction)
		{
			EnhancedInput->BindAction(UIAcceptAction, ETriggerEvent::Started, this, &AStumblePlayerController::OnUIAccept);
		}
		if (UIBackAction)
		{
			EnhancedInput->BindAction(UIBackAction, ETriggerEvent::Started, this, &AStumblePlayerController::OnUIBack);
		}
	}
}

void AStumblePlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();

	// Switch to HUD when pawn is possessed
	if (GetPawn() && HUDWidgetClass)
	{
		ShowHUDWidget();
	}
}

void AStumblePlayerController::HostSession()
{
	if (!SessionInterface.IsValid()) return;

	FOnlineSessionSettings SessionSettings;
	SessionSettings.bIsLANMatch = true;
	SessionSettings.bUsesPresence = true;
	SessionSettings.NumPublicConnections = 4;
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.bAllowJoinInProgress = true;
	SessionSettings.Set(TEXT("GameMode"), FString(TEXT("Stumble")), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	SessionInterface->CreateSession(0, CurrentSessionName, SessionSettings);
}

void AStumblePlayerController::FindSessions()
{
	if (!SessionInterface.IsValid()) return;

	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->bIsLanQuery = true;
	SessionSearch->MaxSearchResults = 20;
	SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
}

void AStumblePlayerController::JoinSession(const FBlueprintSessionResult& SessionResult)
{
	if (!SessionInterface.IsValid()) return;

	SessionInterface->JoinSession(0, CurrentSessionName, SessionResult);
}

void AStumblePlayerController::StartGame()
{
	if (!SessionInterface.IsValid()) return;

	if (AStumbleGameMode* GM = GetWorld()->GetAuthGameMode<AStumbleGameMode>())
	{
		GM->StartRound();
	}
}

void AStumblePlayerController::LeaveSession()
{
	if (!SessionInterface.IsValid()) return;

	SessionInterface->DestroySession(CurrentSessionName);
}

void AStumblePlayerController::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		UE_LOG(LogTemp, Log, TEXT("Session created successfully: %s"), *SessionName.ToString());
		
		if (UStumbleLobbyWidget* LobbyWidget = Cast<UStumbleLobbyWidget>(CurrentWidget))
		{
			LobbyWidget->SetHostStatus(true);
		}
		
		// Travel to game level
		GetWorld()->ServerTravel(TEXT("/Game/Maps/GameLevel?listen"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to create session"));
	}
}

void AStumblePlayerController::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (bWasSuccessful && SessionSearch.IsValid())
	{
		TArray<FBlueprintSessionResult> Results;
		for (const FOnlineSessionSearchResult& Result : SessionSearch->SearchResults)
		{
			FBlueprintSessionResult BlueprintResult;
			BlueprintResult.OnlineResult = Result;
			Results.Add(BlueprintResult);
		}
		
		OnSessionListUpdated.Broadcast(Results);
		
		if (UStumbleLobbyWidget* LobbyWidget = Cast<UStumbleLobbyWidget>(CurrentWidget))
		{
			TArray<FString> SessionNames;
			for (const FBlueprintSessionResult& Result : Results)
			{
				FString SessionName;
				Result.OnlineResult.Session.SessionSettings.Get(TEXT("GameMode"), SessionName);
				SessionNames.Add(SessionName.IsEmpty() ? TEXT("Unknown Session") : SessionName);
			}
			LobbyWidget->SetPlayerList(SessionNames);
		}
	}
}

void AStumblePlayerController::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		UE_LOG(LogTemp, Log, TEXT("Joined session successfully"));
		
		FString ConnectString;
		if (SessionInterface->GetResolvedConnectString(SessionName, ConnectString))
		{
			ClientTravel(ConnectString, ETravelType::TRAVEL_Absolute);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to join session: %d"), (int32)Result);
	}
}

void AStumblePlayerController::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		UE_LOG(LogTemp, Log, TEXT("Session destroyed"));
		ShowLobbyWidget();
	}
}

void AStumblePlayerController::ShowLobbyWidget()
{
	SwitchWidget(LobbyWidgetClass);
}

void AStumblePlayerController::ShowHUDWidget()
{
	SwitchWidget(HUDWidgetClass);
}

void AStumblePlayerController::ShowWinWidget(const FString& WinnerName, bool bLocalPlayerWon, int32 FinalScore)
{
	if (WinWidgetClass)
	{
		SwitchWidget(WinWidgetClass);
		
		if (UStumbleWinWidget* WinWidget = Cast<UStumbleWinWidget>(CurrentWidget))
		{
			WinWidget->SetWinnerInfo(WinnerName, bLocalPlayerWon, FinalScore);
		}
	}
}

void AStumblePlayerController::SwitchWidget(TSubclassOf<UUserWidget> NewWidgetClass)
{
	if (CurrentWidget)
	{
		CurrentWidget->RemoveFromParent();
		CurrentWidget = nullptr;
	}

	if (NewWidgetClass)
	{
		CurrentWidget = CreateWidget<UUserWidget>(this, NewWidgetClass);
		if (CurrentWidget)
		{
			CurrentWidget->AddToViewport();
		}
	}
}

void AStumblePlayerController::OnUIAccept()
{
	// Handle widget button press - could forward to focused widget
}

void AStumblePlayerController::OnUIBack()
{
	// Handle back button
	if (CurrentWidget && CurrentWidget->IsA(UStumbleLobbyWidget::StaticClass()))
	{
		LeaveSession();
	}
}