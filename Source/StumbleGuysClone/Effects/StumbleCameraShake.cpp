// Copyright (c) 2026. Built for portfolio. Free to study.

#include "Effects/StumbleCameraShake.h"

UStumbleCameraShake::UStumbleCameraShake()
{
	Amplitude = 12.0f;
	Frequency = 28.0f;
	Duration = 0.35f;
	RollInfluence = 0.15f;
}

void UStumbleCameraShake::Trigger(float Strength)
{
	// Keep the strongest active shake rather than restarting weaker ones,
	// so a big hit is not drowned out by a follow-up small one.
	CurrentStrength = FMath::Max(CurrentStrength, FMath::Max(Strength, 0.0f));
	RemainingTime = Duration;
	ElapsedTime = 0.0f;
}

float UStumbleCameraShake::GetIntensity() const
{
	if (Duration <= KINDA_SMALL_NUMBER) return 0.0f;

	// Ease-out decay: intensity falls off quadratically toward the end.
	const float Alpha = FMath::Clamp(RemainingTime / Duration, 0.0f, 1.0f);
	return CurrentStrength * Alpha * Alpha;
}

FVector UStumbleCameraShake::ComputeOffset(float DeltaTime)
{
	if (!IsActive())
	{
		Reset();
		return FVector::ZeroVector;
	}

	ElapsedTime += DeltaTime;
	RemainingTime = FMath::Max(0.0f, RemainingTime - DeltaTime);

	const float Intensity = GetIntensity();
	if (Intensity <= KINDA_SMALL_NUMBER)
	{
		Reset();
		return FVector::ZeroVector;
	}

	// Three incommensurate sine waves per axis: smooth, loop-free, and
	// deterministic, so the shake looks the same on every run and platform.
	const float T = ElapsedTime * Frequency * 2.0f * PI;
	const float Scaled = Intensity * Amplitude;

	return FVector(
		FMath::Sin(T * 1.00f + 0.00f) * Scaled * 0.6f,
		FMath::Sin(T * 1.37f + 2.10f) * Scaled * 0.6f,
		FMath::Sin(T * 1.73f + 4.20f) * Scaled * 0.35f
	);
}

void UStumbleCameraShake::Reset()
{
	RemainingTime = 0.0f;
	ElapsedTime = 0.0f;
	CurrentStrength = 1.0f;
}