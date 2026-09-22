// Copyright (c) 2026. Built for portfolio. Free to study.

#include "UI/StumbleLobbyWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "Components/VerticalBox.h"
#include "PlayerController/StumblePlayerController.h"
#include "Engine/World.h"
#include "TimerManager.h"

bool UStumbleLobbyWidget::Initialize()
{
	bool bSuccess = Super::Initialize();
	if (!bSuccess) return false;

	if (HostButton) HostButton->OnClicked.AddDynamic(this, &UStumbleLobbyWidget::OnHostClicked);
	if (JoinButton) JoinButton->OnClicked.AddDynamic(this, &UStumbleLobbyWidget::OnJoinClicked);
	if (StartButton) StartButton->OnClicked.AddDynamic(this, &UStumbleLobbyWidget::OnStartClicked);
	if (RefreshButton) RefreshButton->OnClicked.AddDynamic(this, &UStumbleLobbyWidget::OnRefreshClicked);
	if (ReadyButton) ReadyButton->OnClicked.AddDynamic(this, &UStumbleLobbyWidget::OnReadyClicked);
	if (ColorPrevButton) ColorPrevButton->OnClicked.AddDynamic(this, &UStumbleLobbyWidget::OnColorPrevClicked);
	if (ColorNextButton) ColorNextButton->OnClicked.AddDynamic(this, &UStumbleLobbyWidget::OnColorNextClicked);

	OwningPlayerController = Cast<AStumblePlayerController>(GetOwningPlayer());

	// Initialize slot visuals
	for (int32 i = 0; i < 4; ++i)
	{
		UpdateSlotVisuals(i);
	}

	return true;
}

void UStumbleLobbyWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	// Could add animations here
}

void UStumbleLobbyWidget::SetPlayerSlots(const TArray<FPlayerSlotInfo>& Slots)
{
	if (!SlotsContainer) return;

	for (int32 i = 0; i < FMath::Min(Slots.Num(), 4); ++i)
	{
		const FPlayerSlotInfo& Slot = Slots[i];
		
		if (SlotNameTexts[i])
		{
			SlotNameTexts[i]->SetText(FText::FromString(Slot.PlayerName.IsEmpty() ? TEXT("Empty") : Slot.PlayerName));
		}
		
		if (SlotColorImages[i])
		{
			SlotColorImages[i]->SetColorAndOpacity(Slot.PlayerColor);
		}
		
		if (SlotReadyImages[i] && SlotReadyTexts[i])
		{
			SlotReadyImages[i]->SetVisibility(Slot.bIsReady ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
			SlotReadyTexts[i]->SetText(FText::FromString(Slot.bIsReady ? TEXT("READY") : TEXT("NOT READY")));
			SlotReadyTexts[i]->SetColorAndOpacity(Slot.bIsReady ? FLinearColor::Green : FLinearColor::Red);
		}
		
		// Highlight local player
		if (Slot.bIsLocalPlayer)
		{
			LocalSlotIndex = i;
			if (LocalPlayerNameText)
			{
				LocalPlayerNameText->SetText(FText::FromString(FString::Printf(TEXT("You: %s"), *Slot.PlayerName)));
			}
		}
	}
	
	UpdatePlayerCount(Slots.Num());
}

void UStumbleLobbyWidget::SetHostStatus(bool bIsHost)
{
	if (StartButton)
	{
		StartButton->SetVisibility(bIsHost ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
	if (HostButton)
	{
		HostButton->SetIsEnabled(!bIsHost);
	}
	if (JoinButton)
	{
		JoinButton->SetIsEnabled(!bIsHost);
	}
	if (ReadyButton)
	{
		ReadyButton->SetIsEnabled(!bIsHost); // Only clients ready up
	}
}

void UStumbleLobbyWidget::SetConnectionStatus(const FString& Status)
{
	if (StatusText)
	{
		StatusText->SetText(FText::FromString(Status));
	}
}

void UStumbleLobbyWidget::SetSlotReady(int32 SlotIndex, bool bReady)
{
	if (SlotIndex < 0 || SlotIndex >= 4) return;
	
	if (SlotReadyImages[SlotIndex] && SlotReadyTexts[SlotIndex])
	{
		SlotReadyImages[SlotIndex]->SetVisibility(bReady ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
		SlotReadyTexts[SlotIndex]->SetText(FText::FromString(bReady ? TEXT("READY") : TEXT("NOT READY")));
		SlotReadyTexts[SlotIndex]->SetColorAndOpacity(bReady ? FLinearColor::Green : FLinearColor::Red);
	}
	
	if (SlotIndex == LocalSlotIndex)
	{
		UpdateReadyButtonText();
	}
}

void UStumbleLobbyWidget::SetSlotColor(int32 SlotIndex, FLinearColor Color)
{
	if (SlotIndex < 0 || SlotIndex >= 4) return;
	
	if (SlotColorImages[SlotIndex])
	{
		SlotColorImages[SlotIndex]->SetColorAndOpacity(Color);
	}
}

void UStumbleLobbyWidget::UpdateSlotVisuals(int32 Index)
{
	if (Index < 0 || Index >= 4) return;
	
	FLinearColor Color = AvailableColors[CurrentColorIndex % AvailableColors.Num()];
	
	if (SlotColorImages[Index])
	{
		SlotColorImages[Index]->SetColorAndOpacity(Color);
	}
}

void UStumbleLobbyWidget::UpdateReadyButtonText()
{
	if (ReadyButton)
	{
		// Would need to check local player's ready state from PlayerController
		ReadyButton->SetIsEnabled(true);
	}
}

void UStumbleLobbyWidget::RequestSlotColorChange(int32 Delta)
{
	CurrentColorIndex = (CurrentColorIndex + Delta + AvailableColors.Num()) % AvailableColors.Num();
	SendColorChange(CurrentColorIndex);
}

void UStumbleLobbyWidget::SendReadyState(bool bReady)
{
	if (OwningPlayerController.IsValid())
	{
		OwningPlayerController->SetReadyState(bReady);
	}
}

void UStumbleLobbyWidget::SendColorChange(int32 NewColorIndex)
{
	if (OwningPlayerController.IsValid())
	{
		OwningPlayerController->SetSlotColor(NewColorIndex);
	}
}

void UStumbleLobbyWidget::OnHostClicked()
{
	if (OwningPlayerController.IsValid())
	{
		OwningPlayerController->HostSession();
	}
}

void UStumbleLobbyWidget::OnJoinClicked()
{
	if (OwningPlayerController.IsValid())
	{
		OwningPlayerController->FindSessions();
	}
}

void UStumbleLobbyWidget::OnStartClicked()
{
	if (OwningPlayerController.IsValid())
	{
		OwningPlayerController->StartGame();
	}
}

void UStumbleLobbyWidget::OnRefreshClicked()
{
	if (OwningPlayerController.IsValid())
	{
		OwningPlayerController->FindSessions();
	}
}

void UStumbleLobbyWidget::OnReadyClicked()
{
	// Toggle ready state
	// Would need to track current state
}

void UStumbleLobbyWidget::OnColorPrevClicked()
{
	RequestSlotColorChange(-1);
}

void UStumbleLobbyWidget::OnColorNextClicked()
{
	RequestSlotColorChange(1);
}