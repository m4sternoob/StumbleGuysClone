// Copyright (c) 2026. Built for portfolio. Free to study.

#include "UI/StumbleLobbyWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "PlayerController/StumblePlayerController.h"

bool UStumbleLobbyWidget::Initialize()
{
	bool bSuccess = Super::Initialize();
	if (!bSuccess) return false;

	if (HostButton) HostButton->OnClicked.AddDynamic(this, &UStumbleLobbyWidget::OnHostClicked);
	if (JoinButton) JoinButton->OnClicked.AddDynamic(this, &UStumbleLobbyWidget::OnJoinClicked);
	if (StartButton) StartButton->OnClicked.AddDynamic(this, &UStumbleLobbyWidget::OnStartClicked);
	if (RefreshButton) RefreshButton->OnClicked.AddDynamic(this, &UStumbleLobbyWidget::OnRefreshClicked);

	OwningPlayerController = Cast<AStumblePlayerController>(GetOwningPlayer());

	return true;
}

void UStumbleLobbyWidget::SetPlayerList(const TArray<FString>& PlayerNames)
{
	if (!PlayerListBox) return;

	PlayerListBox->ClearChildren();

	for (const FString& Name : PlayerNames)
	{
		UTextBlock* PlayerText = NewObject<UTextBlock>(this);
		PlayerText->SetText(FText::FromString(Name));
		PlayerText->SetFont(FSlateFontInfo(FCoreStyle::GetDefaultFontStyle("Regular", 16)));
		PlayerText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		PlayerListBox->AddChild(PlayerText);
	}

	UpdatePlayerCount(PlayerNames.Num());
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
}

void UStumbleLobbyWidget::UpdatePlayerCount(int32 Count)
{
	if (PlayerCountText)
	{
		PlayerCountText->SetText(FText::FromString(FString::Printf(TEXT("Players: %d / 4"), Count)));
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