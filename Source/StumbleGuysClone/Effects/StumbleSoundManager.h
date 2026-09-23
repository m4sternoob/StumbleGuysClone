// Copyright (c) 2026. Built for portfolio. Free to study.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "StumbleSoundManager.generated.h"

class USoundBase;

/**
 * UStumbleSoundManager — the game's single audio configuration asset.
 *
 * Holds every gameplay cue in one designer-editable asset instead of scattering
 * USoundBase pointers across actors. Assign one instance on the GameMode and
 * every character resolves its cues through it, so swapping the audio pass is a
 * one-asset change rather than a per-actor edit.
 *
 * Every cue is optional: a null entry is silently skipped, which keeps the
 * prototype playable before audio exists.
 */
UCLASS(BlueprintType)
class STUMBLEGUYSCLONE_API UStumbleSoundManager : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Obstacle impact — also the default fallback for any missing hit cue. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
	TObjectPtr<USoundBase> HitSound;

	/** Player eliminated / fell into the kill zone. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
	TObjectPtr<USoundBase> EliminationSound;

	/** Player respawned after the respawn delay. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
	TObjectPtr<USoundBase> RespawnSound;

	/** Jump. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
	TObjectPtr<USoundBase> JumpSound;

	/** Collectible pickup. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
	TObjectPtr<USoundBase> CollectibleSound;

	/** Round begins. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Round")
	TObjectPtr<USoundBase> RoundStartSound;

	/** Round ends with no winner (time out). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Round")
	TObjectPtr<USoundBase> RoundEndSound;

	/** Local player won. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Round")
	TObjectPtr<USoundBase> WinSound;

	/** Local player lost. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Round")
	TObjectPtr<USoundBase> LoseSound;

	/** Play a cue at a world location. No-op when the cue is unset. */
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlayCueAtLocation(UObject* WorldContext, USoundBase* Cue, const FVector& Location, float Volume = 1.0f, float Pitch = 1.0f) const;
};