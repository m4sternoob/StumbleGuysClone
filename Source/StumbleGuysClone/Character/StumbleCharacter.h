// Copyright (c) 2026. Built for portfolio. Free to study.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "StumbleCharacter.generated.h"

class UInputAction;
class UInputMappingContext;
class USpringArmComponent;
class UCameraComponent;
class USphereComponent;

/**
 * AStumbleCharacter — Replicated capsule character with sphere head.
 * Multiplayer-ready: server-authoritative movement, client prediction.
 */
UCLASS()
class STUMBLEGUYSCLONE_API AStumbleCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AStumbleCharacter();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// Input assets (set in Blueprint or defaults)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> JumpAction;

	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> HeadSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;

	// Movement tuning (replicated via CharacterMovementComponent)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float MaxWalkSpeed = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float JumpZVelocity = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0"))
	float AirControl = 0.35f;

	// Network
	UPROPERTY(ReplicatedUsing = OnRep_PlayerColor)
	FLinearColor PlayerColor = FLinearColor::White;

	UFUNCTION()
	void OnRep_PlayerColor();

private:
	void Move(const FInputActionValue& Value);
	void OnJumpStarted();
	void OnJumpStopped();

	void ApplyPlayerColor();
};