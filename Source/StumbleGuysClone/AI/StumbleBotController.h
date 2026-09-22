// Copyright (c) 2026. Built for portfolio. Free to study.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "StumbleBotController.generated.h"

class AStumbleCharacter;
class UNavigationSystemV1;

UCLASS()
class STUMBLEGUYSCLONE_API AStumbleBotController : public AAIController
{
	GENERATED_BODY()

public:
	AStumbleBotController();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bot AI", meta = (AllowPrivateAccess = "true", ClampMin = "0.1"))
	float WanderRadius = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bot AI", meta = (AllowPrivateAccess = "true", ClampMin = "0.1"))
	float MinWanderTime = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bot AI", meta = (AllowPrivateAccess = "true", ClampMin = "0.1"))
	float MaxWanderTime = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bot AI", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0"))
	float JumpChance = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bot AI", meta = (AllowPrivateAccess = "true", ClampMin = "0.1"))
	float ObstacleAvoidanceRadius = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bot AI", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0"))
	float ObstacleAvoidanceWeight = 0.5f;

private:
	AStumbleCharacter* BotCharacter = nullptr;
	UNavigationSystemV1* NavSystem = nullptr;

	FVector CurrentWanderTarget = FVector::ZeroVector;
	float WanderTimer = 0.0f;
	float CurrentWanderDuration = 0.0f;
	bool bHasValidTarget = false;

	FTimerHandle WanderTimerHandle;

	void FindNewWanderTarget();
	void MoveToWanderTarget();
	void CheckAndAvoidObstacles(float DeltaTime);
	void MaybeJump();
	void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result);
};