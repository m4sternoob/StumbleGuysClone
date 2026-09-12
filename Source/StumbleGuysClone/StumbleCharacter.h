// Copyright (c) 2026. Built for portfolio. Free to study.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "StumbleCharacter.generated.h"

class UInputAction;
class UInputMappingContext;
class UCapsuleComponent;
class USphereComponent;
class USpringArmComponent;
class UCameraComponent;

/**
 * AStumbleCharacter — Day 2: Capsule body + sphere head, EnhancedInput movement + jump.
 * No Blueprints — pure C++ gameplay class.
 */
UCLASS()
class STUMBLEGUYSCLONE_API AStumbleCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AStumbleCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// Input actions
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USphereComponent* HeadSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	// Movement tuning
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float MaxWalkSpeed = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float JumpZVelocity = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float AirControl = 0.35f;

private:
	void Move(const FInputActionValue& Value);
	void OnJumpStarted();
	void OnJumpStopped();

	// Visual debug
	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDrawDebugCapsule = false;
};