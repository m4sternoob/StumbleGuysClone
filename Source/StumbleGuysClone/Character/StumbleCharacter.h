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
 * Industry-standard: Server-authoritative movement with client prediction.
 * Uses CharacterMovementComponent's built-in network prediction.
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
	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;

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

	// Camera smoothing (client-side only)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0"))
	float CameraLagSpeed = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float CameraLagMaxDistance = 200.0f;

	// Movement tuning
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float MaxWalkSpeed = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float JumpZVelocity = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0"))
	float AirControl = 0.35f;

	// Network optimization
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network", meta = (AllowPrivateAccess = "true", ClampMin = "1.0"))
	float NetUpdateFrequency = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network", meta = (AllowPrivateAccess = "true", ClampMin = "1.0"))
	float MinNetUpdateFrequency = 30.0f;

	// Visual - replicated
	UPROPERTY(ReplicatedUsing = OnRep_PlayerColor)
	FLinearColor PlayerColor = FLinearColor::White;

	UFUNCTION()
	void OnRep_PlayerColor();

	// Elimination state
	UPROPERTY(ReplicatedUsing = OnRep_Eliminated)
	bool bIsEliminated = false;

	UFUNCTION()
	void OnRep_Eliminated();

	// Respawn system
	UPROPERTY(ReplicatedUsing = OnRep_RespawnState)
	bool bIsRespawning = false;

	UPROPERTY(Replicated)
	float RespawnTimeRemaining = 0.0f;

	UFUNCTION()
	void OnRep_RespawnState();

	// Invincibility frames
	UPROPERTY(ReplicatedUsing = OnRep_Invincible)
	bool bIsInvincible = false;

	UPROPERTY(Replicated)
	float InvincibilityTimeRemaining = 0.0f;

	UFUNCTION()
	void OnRep_Invincible();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Respawn", meta = (AllowPrivateAccess = "true", ClampMin = "0.1"))
	float RespawnDelay = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Respawn", meta = (AllowPrivateAccess = "true", ClampMin = "0.1"))
	float InvincibilityDuration = 2.0f;

	// Juice effects (public so obstacles/collectibles can trigger them)
	UFUNCTION(BlueprintCallable, Category = "Juice")
	void PlayHitEffects(const FHitResult& HitResult);

	UFUNCTION(BlueprintCallable, Category = "Juice")
	void PlayCollectibleSound();

	/** Assign the shared audio configuration (called by the GameMode on spawn). */
	UFUNCTION(BlueprintCallable, Category = "Juice")
	void SetSoundSet(class UStumbleSoundManager* InSoundSet) { SoundSet = InSoundSet; }

	/** Server-authoritative colour assignment, replicated to clients. */
	UFUNCTION(BlueprintCallable, Category = "Character")
	void SetPlayerColor(const FLinearColor& NewColor) { PlayerColor = NewColor; }

	UFUNCTION(BlueprintPure, Category = "Character")
	bool IsRespawning() const { return bIsRespawning; }

	UFUNCTION(BlueprintPure, Category = "Character")
	bool IsEliminated() const { return bIsEliminated; }

	UFUNCTION(BlueprintPure, Category = "Character")
	FLinearColor GetPlayerColor() const { return PlayerColor; }

private:
	void Move(const FInputActionValue& Value);
	void OnJumpStarted();
	void OnJumpStopped();

	void ApplyPlayerColor();
	void SetEliminated(bool bEliminated);

	// Respawn system
	void StartRespawn();
	void UpdateRespawnTimer(float DeltaTime);
	void FinishRespawn();
	void SetInvincible(bool bInvincible);
	void UpdateInvincibilityTimer(float DeltaTime);
	void UpdateVisualInvincibility(float DeltaTime);

	// Juice effects (internal)
	void PlayEliminationEffects();
	void PlayJumpSound();
	void PlayRespawnSound();

	// Camera smoothing (client-side prediction correction)
	void UpdateCameraLag(float DeltaTime);
	void SmoothCameraRotation(float DeltaTime);
	void UpdateCameraLagPosition(float DeltaTime);

	// Camera state tracking for smoothing
	FVector LastCameraLocation;
	FRotator LastCameraRotation;
	bool bCameraInitialized = false;

	// Invincibility visual
	bool bInvincibilityVisible = true;
	float InvincibilityBlinkTimer = 0.0f;
	const float InvincibilityBlinkInterval = 0.1f;

	// Juice effect references
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Juice", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UParticleSystem> HitParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Juice", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UParticleSystem> EliminationParticles;

	/** Single audio configuration asset; every cue resolves through it. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Juice", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UStumbleSoundManager> SoundSet;

	/** Procedural camera shake, owned per-character and applied locally. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Juice", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UStumbleCameraShake> CameraShake;

	/** Fire a camera shake at the given strength multiplier. */
	UFUNCTION(BlueprintCallable, Category = "Juice")
	void TriggerCameraShake(float Strength = 1.0f);
};