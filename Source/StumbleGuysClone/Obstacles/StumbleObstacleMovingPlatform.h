// Copyright (c) 2026. Built for portfolio. Free to study.

#pragma once

#include "CoreMinimal.h"
#include "Obstacles/StumbleObstacleBase.h"
#include "StumbleObstacleMovingPlatform.generated.h"

UCLASS()
class STUMBLEGUYSCLONE_API AStumbleObstacleMovingPlatform : public AStumbleObstacleBase
{
	GENERATED_BODY()

public:
	AStumbleObstacleMovingPlatform();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MovingPlatform", meta = (AllowPrivateAccess = "true"))
	float PlatformWidth = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MovingPlatform", meta = (AllowPrivateAccess = "true"))
	float PlatformLength = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MovingPlatform", meta = (AllowPrivateAccess = "true"))
	float PlatformThickness = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MovingPlatform", meta = (AllowPrivateAccess = "true"))
	FVector MoveAxis = FVector(1.0f, 0.0f, 0.0f); // X axis by default

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MovingPlatform", meta = (AllowPrivateAccess = "true", ClampMin = "100.0"))
	float MoveDistance = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MovingPlatform", meta = (AllowPrivateAccess = "true", ClampMin = "0.1"))
	float MoveSpeed = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MovingPlatform", meta = (AllowPrivateAccess = "true"))
	bool bPingPong = true; // true = back and forth, false = loop

private:
	FVector StartLocation;
	FVector TargetLocation;
	float MoveProgress = 0.0f;
	int32 MoveDirection = 1; // 1 or -1
};