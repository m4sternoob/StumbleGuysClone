// Copyright (c) 2026. Built for portfolio. Free to study.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StumbleObstacleBase.generated.h"

UCLASS(Abstract)
class STUMBLEGUYSCLONE_API AStumbleObstacleBase : public AActor
{
	GENERATED_BODY()

public:
	AStumbleObstacleBase();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> ObstacleMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> CollisionBox;

	// Obstacle settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle", meta = (AllowPrivateAccess = "true"))
	bool bIsActive = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float DamageImpulse = 1500.0f;

	// Visual
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual", meta = (AllowPrivateAccess = "true"))
	FLinearColor ObstacleColor = FLinearColor(0.8f, 0.2f, 0.2f, 1.0f);

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	virtual void ApplyObstacleEffect(AStumbleCharacter* Character, const FHitResult& HitResult);
	void ApplyVisualMaterial();
};