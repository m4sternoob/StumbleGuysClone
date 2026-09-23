// Copyright (c) 2026. Built for portfolio. Free to study.

#include "Effects/StumbleSoundManager.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

void UStumbleSoundManager::PlayCueAtLocation(UObject* WorldContext, USoundBase* Cue, const FVector& Location, float Volume, float Pitch) const
{
	// Every cue is optional by design — a null entry simply produces silence so
	// the prototype stays playable before the audio pass lands.
	if (!Cue || !WorldContext) return;

	UGameplayStatics::PlaySoundAtLocation(
		WorldContext,
		Cue,
		Location,
		Volume,
		Pitch
	);
}