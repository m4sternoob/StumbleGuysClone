// Copyright (c) 2026. Built for portfolio. Free to study.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "StumblePlayerController.generated.h"

class UUserWidget;
class UInputMappingContext;
class UInputAction;

UCLASS()
class STUMBLEGUYSCLONE_API AStumblePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AStumblePlayerController();

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	// UI
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowLobbyWidget();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowHUDWidget();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowWinWidget(AStumbleCharacter* Winner);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> LobbyWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> HUDWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> WinWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> CurrentWidget;

	// Input for UI navigation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> UIInputContext;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> UIAcceptAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> UIBackAction;

private:
	void OnUIAccept();
	void OnUIBack();
	void SwitchWidget(TSubclassOf<UUserWidget> NewWidgetClass);
};