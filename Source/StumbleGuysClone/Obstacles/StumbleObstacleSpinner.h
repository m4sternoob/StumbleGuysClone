// Copyright (c) 2026. Built for portfolio. Free to study.

#pragma once

#include "CoreMinimal.h"
#include "Obstacles/StumbleObstacleBase.h"
#include "StumbleObstacleSpinner.generated.h"

UCLASS()
class STUMBLEGUYSCLONE_API AStumbleObstacleSpinner : public AStumbleObstacleBase
{
	GENERATED_BODY()

public:
	AStumbleObstacleSpinner();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spinner", meta = (AllowPrivateAccess = "true", ClampMin = "10.0"))
	float BarLength = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spinner", meta = (AllowPrivateAccess = "true", ClampMin = "10.0"))
	float BarThickness = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spinner", meta = (AllowPrivateAccess = "true", ClampMin = "0.1"))
	float RotationSpeed = 90.0f; // Degrees per second

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spinner", meta = (AllowPrivateAccess = "true"))
	bool bReverseDirection = false;

private:
	FVector BarExtent;
};