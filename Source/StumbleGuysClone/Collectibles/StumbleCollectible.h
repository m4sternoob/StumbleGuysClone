// Copyright (c) 2026. Built for portfolio. Free to study.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StumbleCollectible.generated.h"

UCLASS()
class STUMBLEGUYSCLONE_API AStumbleCollectible : public AActor
{
	GENERATED_BODY()

public:
	AStumbleCollectible();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> CollectibleMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> OverlapSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> RootScene;

	// Collectible settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collectible", meta = (AllowPrivateAccess = "true"))
	int32 ScoreValue = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collectible", meta = (AllowPrivateAccess = "true"))
	FLinearColor CollectibleColor = FLinearColor(1.0f, 0.8f, 0.1f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collectible", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float FloatAmplitude = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collectible", meta = (AllowPrivateAccess = "true", ClampMin = "0.1"))
	float FloatSpeed = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collectible", meta = (AllowPrivateAccess = "true", ClampMin = "0.1"))
	float RotationSpeed = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collectible", meta = (AllowPrivateAccess = "true"))
	bool bRespawn = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collectible", meta = (AllowPrivateAccess = "true", ClampMin = "1.0"))
	float RespawnTime = 10.0f;

private:
	FVector InitialLocation;
	float FloatTime = 0.0f;
	bool bCollected = false;
	FTimerHandle RespawnTimerHandle;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Collect(AActor* Collector);

	void ApplyVisualMaterial();
	void RespawnCollectible();
};