// Copyright (c) 2026. Built for portfolio. Free to study.

#pragma once

#include "CoreMinimal.h"
#include "Obstacles/StumbleObstacleBase.h"
#include "StumbleObstacleSpinner.generated.h"

class UAudioComponent;
class USoundBase;
class UParticleSystem;
class UCurveFloat;

UCLASS()
class STUMBLEGUYSCLONE_API AStumbleObstacleSpinner : public AStumbleObstacleBase
{
	GENERATED_BODY()

public:
	AStumbleObstacleSpinner();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Spinner configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spinner", meta = (AllowPrivateAccess = "true", ClampMin = "10.0"))
	float BarLength = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spinner", meta = (AllowPrivateAccess = "true", ClampMin = "10.0"))
	float BarThickness = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spinner", meta = (AllowPrivateAccess = "true", ClampMin = "0.1"))
	float RotationSpeed = 90.0f; // Degrees per second

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spinner", meta = (AllowPrivateAccess = "true"))
	bool bReverseDirection = false;

	// Visual polish
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCurveFloat> RotationSpeedCurve; // Optional speed variation over time

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UParticleSystem> HitParticles; // Spawn on impact

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UParticleSystem> IdleParticles; // Continuous particles on bar tips

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UParticleSystemComponent> IdleParticleComponents[2]; // Tip particles

	// Audio
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundBase> HitSound; // Play on character hit

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundBase> AmbientSound; // Continuous low hum

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAudioComponent> AmbientAudioComponent;

	// Gameplay tuning
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay", meta = (AllowPrivateAccess = "true", ClampMin = "100.0"))
	float HitImpulseMultiplier = 1.5f; // Extra impulse on hit

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0"))
	float HitCooldown = 0.5f; // Prevent multi-hits per frame

private:
	FVector BarExtent;
	FVector TipOffsets[2]; // End positions for tip particles
	float LastHitTime = -1.0f;

	void UpdateTipPositions();
	void SpawnTipParticles();
	void PlayHitEffects(AStumbleCharacter* Character, const FHitResult& HitResult);
	void UpdateIdleParticles(float DeltaTime);
};