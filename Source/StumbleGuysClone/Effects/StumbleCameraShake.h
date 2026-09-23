// Copyright (c) 2026. Built for portfolio. Free to study.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "StumbleCameraShake.generated.h"

/**
 * UStumbleCameraShake — lightweight, plugin-free camera shake.
 *
 * UE 5.x moved shake logic into UCameraShakePattern and the GameplayCameras
 * plugin. For a prototype we want zero plugin dependencies and full C++ control,
 * so this is a plain UObject holding shake parameters plus a pure offset solver.
 *
 * AStumbleCharacter owns one instance and applies ComputeOffset() to its camera
 * boom each tick while a shake is active. Deterministic, allocation-free, and
 * evaluated only on the locally controlled pawn.
 */
UCLASS(BlueprintType)
class STUMBLEGUYSCLONE_API UStumbleCameraShake : public UObject
{
	GENERATED_BODY()

public:
	UStumbleCameraShake();

	/** Peak positional displacement, in world units, at full strength. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake", meta = (ClampMin = "0.0"))
	float Amplitude = 12.0f;

	/** Oscillations per second. Higher = snappier, more violent. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake", meta = (ClampMin = "0.1"))
	float Frequency = 28.0f;

	/** Seconds for the shake to decay to zero. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake", meta = (ClampMin = "0.01"))
	float Duration = 0.35f;

	/** Fraction of the amplitude applied as roll, for a bit of torque. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake", meta = (ClampMin = "0.0"))
	float RollInfluence = 0.15f;

	/** Restart the shake at the given strength (never weakens an active shake). */
	void Trigger(float Strength = 1.0f);

	/** Advance the shake and return the camera offset for this frame. */
	FVector ComputeOffset(float DeltaTime);

	/** Current scalar intensity in [0, Strength], after decay. */
	float GetIntensity() const;

	/** True while the shake still contributes. */
	bool IsActive() const { return RemainingTime > 0.0f; }

	/** Cancel immediately (elimination, respawn, teardown). */
	void Reset();

private:
	float RemainingTime = 0.0f;
	float ElapsedTime = 0.0f;
	float CurrentStrength = 1.0f;
};