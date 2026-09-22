// Copyright (c) 2026. Built for portfolio. Free to study.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StumbleLobbyWidget.generated.h"

class UButton;
class UTextBlock;
class UVerticalBox;
class UHorizontalBox;
class UImage;
class AStumblePlayerController;

USTRUCT(BlueprintType)
struct FPlayerSlotInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString PlayerName;

	UPROPERTY(BlueprintReadWrite)
	FLinearColor PlayerColor;

	UPROPERTY(BlueprintReadWrite)
	bool bIsReady;

	UPROPERTY(BlueprintReadWrite)
	bool bIsLocalPlayer;

	UPROPERTY(BlueprintReadWrite)
	int32 SlotIndex;
};

UCLASS()
class STUMBLEGUYSCLONE_API UStumbleLobbyWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual bool Initialize() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void SetPlayerSlots(const TArray<FPlayerSlotInfo>& Slots);

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void SetHostStatus(bool bIsHost);

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void SetConnectionStatus(const FString& Status);

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void SetSlotReady(int32 SlotIndex, bool bReady);

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void SetSlotColor(int32 SlotIndex, FLinearColor Color);

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
	TObjectPtr<UButton> ReadyButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ColorPrevButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ColorNextButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> StatusText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PlayerCountText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> LocalPlayerNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> SlotsContainer;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SlotNameTexts[4];

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> SlotColorImages[4];

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> SlotReadyImages[4];

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SlotReadyTexts[4];

	UFUNCTION()
	void OnHostClicked();

	UFUNCTION()
	void OnJoinClicked();

	UFUNCTION()
	void OnStartClicked();

	UFUNCTION()
	void OnRefreshClicked();

	UFUNCTION()
	void OnReadyClicked();

	UFUNCTION()
	void OnColorPrevClicked();

	UFUNCTION()
	void OnColorNextClicked();

private:
	TWeakObjectPtr<AStumblePlayerController> OwningPlayerController;

	int32 LocalSlotIndex = -1;
	int32 CurrentColorIndex = 0;

	TArray<FLinearColor> AvailableColors = {
		FLinearColor(1.0f, 0.2f, 0.2f, 1.0f),  // Red
		FLinearColor(0.2f, 0.4f, 1.0f, 1.0f),  // Blue
		FLinearColor(0.2f, 1.0f, 0.3f, 1.0f),  // Green
		FLinearColor(1.0f, 0.9f, 0.2f, 1.0f),  // Yellow
		FLinearColor(1.0f, 0.5f, 0.0f, 1.0f),  // Orange
		FLinearColor(0.8f, 0.2f, 1.0f, 1.0f),  // Purple
		FLinearColor(0.0f, 1.0f, 1.0f, 1.0f),  // Cyan
		FLinearColor(1.0f, 1.0f, 1.0f, 1.0f),  // White
	};

	void UpdateSlotVisuals(int32 Index);
	void UpdateReadyButtonText();
	void RequestSlotColorChange(int32 Delta);
	void SendReadyState(bool bReady);
	void SendColorChange(int32 NewColorIndex);
};