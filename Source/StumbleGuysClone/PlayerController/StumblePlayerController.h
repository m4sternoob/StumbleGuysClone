// Copyright (c) 2026. Built for portfolio. Free to study.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "StumblePlayerController.generated.h"

class UUserWidget;
class UInputMappingContext;
class UInputAction;
class IOnlineSession;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionListUpdated, const TArray<FBlueprintSessionResult>&, SessionResults);

UCLASS()
class STUMBLEGUYSCLONE_API AStumblePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AStumblePlayerController();

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void OnRep_Pawn() override;

	// Session management
	UFUNCTION(BlueprintCallable, Category = "Session")
	void HostSession();

	UFUNCTION(BlueprintCallable, Category = "Session")
	void FindSessions();

	UFUNCTION(BlueprintCallable, Category = "Session")
	void JoinSession(const FBlueprintSessionResult& SessionResult);

	UFUNCTION(BlueprintCallable, Category = "Session")
	void StartGame();

	UFUNCTION(BlueprintCallable, Category = "Session")
	void LeaveSession();

	// UI
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowLobbyWidget();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowHUDWidget();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowWinWidget(const FString& WinnerName, bool bLocalPlayerWon, int32 FinalScore);

	// Session delegates
	UFUNCTION()
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

	UFUNCTION()
	void OnFindSessionsComplete(bool bWasSuccessful);

	UFUNCTION()
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	UFUNCTION()
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);

	// UI delegates
	FOnSessionListUpdated OnSessionListUpdated;

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

	// Session
	UPROPERTY()
	TObjectPtr<class IOnlineSession> SessionInterface;

	FName CurrentSessionName = TEXT("StumbleSession");

private:
	void SwitchWidget(TSubclassOf<UUserWidget> NewWidgetClass);
	void OnUIAccept();
	void OnUIBack();

	// Session search settings
	TSharedPtr<class FOnlineSessionSearch> SessionSearch;
};