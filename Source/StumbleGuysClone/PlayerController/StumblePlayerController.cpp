// Copyright (c) 2026. Built for portfolio. Free to study.

#include "PlayerController/StumblePlayerController.h"
#include "Character/StumbleCharacter.h"
#include "Blueprint/UserWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"

AStumblePlayerController::AStumblePlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

void AStumblePlayerController::BeginPlay()
{
	Super::BeginPlay();

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
			Subsystem->AddMappingContext(UIInputContext, 10); // Higher priority than gameplay
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

void AStumblePlayerController::ShowLobbyWidget()
{
	SwitchWidget(LobbyWidgetClass);
}

void AStumblePlayerController::ShowHUDWidget()
{
	SwitchWidget(HUDWidgetClass);
}

void AStumblePlayerController::ShowWinWidget(AStumbleCharacter* Winner)
{
	if (WinWidgetClass)
	{
		SwitchWidget(WinWidgetClass);
		// TODO: Pass winner info to widget
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
	// Handle widget button press
}

void AStumblePlayerController::OnUIBack()
{
	// Handle back button
}