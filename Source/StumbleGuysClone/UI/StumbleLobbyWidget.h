// Copyright (c) 2026. Built for portfolio. Free to study.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StumbleLobbyWidget.generated.h"

class UButton;
class UTextBlock;
class UVerticalBox;
class AStumblePlayerController;

UCLASS()
class STUMBLEGUYSCLONE_API UStumbleLobbyWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual bool Initialize() override;

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void SetPlayerList(const TArray<FString>& PlayerNames);

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void SetHostStatus(bool bIsHost);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> HostButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> JoinButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> StartButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> RefreshButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> StatusText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> PlayerListBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PlayerCountText;

	UFUNCTION()
	void OnHostClicked();

	UFUNCTION()
	void OnJoinClicked();

	UFUNCTION()
	void OnStartClicked();

	UFUNCTION()
	void OnRefreshClicked();

private:
	TWeakObjectPtr<AStumblePlayerController> OwningPlayerController;

	void UpdatePlayerCount(int32 Count);
};