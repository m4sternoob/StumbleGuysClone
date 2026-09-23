// Copyright (c) 2026. Built for portfolio. Free to study.

#include "Obstacles/StumbleObstacleSpinner.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Character/StumbleCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Curves/CurveFloat.h"

AStumbleObstacleSpinner::AStumbleObstacleSpinner()
{
	// Spinner uses a long thin box as the rotating bar
	BarExtent = FVector(BarThickness / 2.0f, BarLength / 2.0f, BarThickness / 2.0f);

	if (ObstacleMesh)
	{
		ObstacleMesh->SetRelativeScale3D(FVector(
			BarThickness / 100.0f,
			BarLength / 100.0f,
			BarThickness / 100.0f
		));
	}

	if (CollisionBox)
	{
		CollisionBox->SetBoxExtent(BarExtent);
	}

	// Distinct color for spinner
	ObstacleColor = FLinearColor(0.9f, 0.3f, 0.1f, 1.0f);

	// Initialize tip offsets
	TipOffsets[0] = FVector(0.0f, BarLength / 2.0f, 0.0f);
	TipOffsets[1] = FVector(0.0f, -BarLength / 2.0f, 0.0f);

	// Defaults for new properties
	HitImpulseMultiplier = 1.5f;
	HitCooldown = 0.5f;
	LastHitTime = -1.0f;
}

void AStumbleObstacleSpinner::BeginPlay()
{
	Super::BeginPlay();

	// Spawn idle particles at tips
	SpawnTipParticles();

	// Start ambient sound
	if (AmbientSound)
	{
		AmbientAudioComponent = UGameplayStatics::SpawnSoundAttached(
			AmbientSound,
			GetRootComponent(),
			NAME_None,
			FVector::ZeroVector,
			EAttachLocation::KeepRelativeOffset,
			false,
			1.0f,
			1.0f,
			0.0f,
			nullptr,
			nullptr,
			true
		);
	}
}

void AStumbleObstacleSpinner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Clean up particle components
	for (int32 i = 0; i < 2; ++i)
	{
		if (IdleParticleComponents[i])
		{
			IdleParticleComponents[i]->DestroyComponent();
			IdleParticleComponents[i] = nullptr;
		}
	}

	if (AmbientAudioComponent)
	{
		AmbientAudioComponent->Stop();
		AmbientAudioComponent = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void AStumbleObstacleSpinner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!HasAuthority() || !bIsActive) return;

	// Optional speed variation via curve
	float CurrentSpeed = RotationSpeed;
	if (RotationSpeedCurve)
	{
		float CurveTime = FMath::Fmod(GetWorld()->GetTimeSeconds(), RotationSpeedCurve->GetFloatValue(RotationSpeedCurve->GetLastKey().Time));
		CurrentSpeed *= RotationSpeedCurve->GetFloatValue(CurveTime);
	}

	// Rotate around Z axis
	float CurrentYaw = GetActorRotation().Yaw;
	float DeltaYaw = CurrentSpeed * DeltaTime;
	if (bReverseDirection) DeltaYaw = -DeltaYaw;

	SetActorRotation(FRotator(0.0f, CurrentYaw + DeltaYaw, 0.0f));

	// Update tip particle positions
	UpdateTipPositions();
	UpdateIdleParticles(DeltaTime);
}

void AStumbleObstacleSpinner::UpdateTipPositions()
{
	if (!ObstacleMesh) return;

	FQuat Rotation = GetActorQuat();
	TipOffsets[0] = Rotation.RotateVector(FVector(0.0f, BarLength / 2.0f, 0.0f));
	TipOffsets[1] = Rotation.RotateVector(FVector(0.0f, -BarLength / 2.0f, 0.0f));

	// Update particle component positions
	for (int32 i = 0; i < 2; ++i)
	{
		if (IdleParticleComponents[i])
		{
			FVector WorldTipPos = GetActorLocation() + TipOffsets[i];
			IdleParticleComponents[i]->SetWorldLocation(WorldTipPos);
		}
	}
}

void AStumbleObstacleSpinner::SpawnTipParticles()
{
	if (!IdleParticles || !GetWorld()) return;

	for (int32 i = 0; i < 2; ++i)
	{
		FVector TipLocation = GetActorLocation() + TipOffsets[i];
		IdleParticleComponents[i] = UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			IdleParticles,
			TipLocation,
			FRotator::ZeroRotator,
			true,
			EPSCPoolMethod::AutoRelease
		);
	}
}

void AStumbleObstacleSpinner::UpdateIdleParticles(float DeltaTime)
{
	// Particles automatically follow if attached, but we update position manually
	UpdateTipPositions();
}

void AStumbleObstacleSpinner::PlayHitEffects(AStumbleCharacter* Character, const FHitResult& HitResult)
{
	if (!Character || !GetWorld()) return;

	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastHitTime < HitCooldown) return;
	LastHitTime = CurrentTime;

	// Spawn hit particles at impact point
	if (HitParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			HitParticles,
			HitResult.Location,
			HitResult.Normal.Rotation(),
			true,
			EPSCPoolMethod::AutoRelease
		);
	}

	// Play hit sound
	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			HitSound,
			HitResult.Location,
			1.0f,
			1.0f,
			0.0f
		);
	}

	// Screen shake for local player
	if (Character->IsLocallyControlled() && Character->GetController())
	{
		if (APlayerController* PC = Cast<APlayerController>(Character->GetController()))
		{
			PC->ClientStartCameraShake(nullptr, 1.0f); // Would need a camera shake asset
		}
	}
}

void AStumbleObstacleSpinner::ApplyObstacleEffect(AStumbleCharacter* Character, const FHitResult& HitResult)
{
	if (!Character)
	{
		return;
	}

	// Knock the player outward plus a tangential nudge, which reads as the bar
	// sweeping them off rather than simply pushing them straight back.
	FVector ImpulseDir = (Character->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	if (ImpulseDir.IsNearlyZero())
	{
		ImpulseDir = FVector::UpVector;
	}

	const FVector TangentDir = FVector(-ImpulseDir.Y, ImpulseDir.X, 0.0f).GetSafeNormal();
	const FVector FinalImpulse = (ImpulseDir + TangentDir * 0.3f) * DamageImpulse * HitImpulseMultiplier;

	Character->GetCharacterMovement()->AddImpulse(FinalImpulse, true);

	// Overlap events carry no hit location, so synthesise one for the effects.
	FHitResult EffectHit = HitResult;
	if (!EffectHit.bBlockingHit)
	{
		EffectHit.Location = Character->GetActorLocation();
		EffectHit.Normal = -ImpulseDir;
		EffectHit.bBlockingHit = true;
	}

	// Spinner-specific burst, then the character's shared hit feedback (which
	// owns the camera shake and the audio cue).
	PlayHitEffects(Character, EffectHit);
	Character->PlayHitEffects(EffectHit);
}

void AStumbleObstacleSpinner::PlayHitEffects(AStumbleCharacter* Character, const FHitResult& HitResult)
{
	if (!Character || !GetWorld())
	{
		return;
	}

	// Rate-limit so a single sweep cannot spam effects across several frames.
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastHitTime < HitCooldown)
	{
		return;
	}
	LastHitTime = CurrentTime;

	if (HitParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(), HitParticles, HitResult.Location, HitResult.Normal.Rotation(),
			true, EPSCPoolMethod::AutoRelease);
	}

	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitSound, HitResult.Location);
	}
}