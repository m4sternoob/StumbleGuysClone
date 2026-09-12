// Copyright (c) 2026. Built for portfolio. Free to study.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StumbleArena.generated.h"

/**
 * AStumbleArena — Floor, 4 walls, kill zone.
 * Spawned by GameMode, replicated to all clients.
 */
UCLASS()
class STUMBLEGUYSCLONE_API AStumbleArena : public AActor
{
	GENERATED_BODY()

public:
	AStumbleArena();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> FloorMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> WallMeshes[4];

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> KillZoneVolume;

	// Arena dimensions
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena", meta = (AllowPrivateAccess = "true", ClampMin = "100.0"))
	float ArenaSize = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena", meta = (AllowPrivateAccess = "true", ClampMin = "10.0"))
	float WallHeight = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena", meta = (AllowPrivateAccess = "true", ClampMin = "10.0"))
	float WallThickness = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena", meta = (AllowPrivateAccess = "true"))
	float KillZoneDepth = -1000.0f;

private:
	void CreateFloor();
	void CreateWalls();
	void CreateKillZone();

	UFUNCTION()
	void OnKillZoneOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};