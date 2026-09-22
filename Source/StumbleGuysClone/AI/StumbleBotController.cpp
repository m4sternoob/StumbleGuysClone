// Copyright (c) 2026. Built for portfolio. Free to study.

#include "AI/StumbleBotController.h"
#include "Character/StumbleCharacter.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

AStumbleBotController::AStumbleBotController()
{
	// Bot AI config
	WanderRadius = 2000.0f;
	MinWanderTime = 2.0f;
	MaxWanderTime = 5.0f;
	JumpChance = 0.1f;
	ObstacleAvoidanceRadius = 300.0f;
	ObstacleAvoidanceWeight = 0.5f;

	bWantsPlayerState = true;
}

void AStumbleBotController::BeginPlay()
{
	Super::BeginPlay();

	NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	
	if (GetPawn())
	{
		OnPossess(GetPawn());
	}
}

void AStumbleBotController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	BotCharacter = Cast<AStumbleCharacter>(InPawn);
	if (BotCharacter)
	{
		// Configure bot movement
		if (UCharacterMovementComponent* MoveComp = BotCharacter->GetCharacterMovement())
		{
			MoveComp->bUseControllerDesiredRotation = true;
			MoveComp->bOrientRotationToMovement = true;
			MoveComp->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
		}

		// Start wandering
		FindNewWanderTarget();
	}
}

void AStumbleBotController::OnUnPossess()
{
	Super::OnUnPossess();

	GetWorldTimerManager().ClearTimer(WanderTimerHandle);
	BotCharacter = nullptr;
	bHasValidTarget = false;
}

void AStumbleBotController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!BotCharacter || !BotCharacter->IsValidLowLevel() || BotCharacter->bIsEliminated || BotCharacter->bIsRespawning)
	{
		return;
	}

	// Update wander timer
	if (bHasValidTarget)
	{
		WanderTimer += DeltaTime;
		if (WanderTimer >= CurrentWanderDuration)
		{
			FindNewWanderTarget();
		}
	}
	else
	{
		FindNewWanderTarget();
	}

	// Check and avoid obstacles
	CheckAndAvoidObstacles(DeltaTime);

	// Random jump
	MaybeJump();
}

void AStumbleBotController::FindNewWanderTarget()
{
	if (!BotCharacter || !NavSystem) return;

	FVector BotLocation = BotCharacter->GetActorLocation();
	FVector RandomPoint = BotLocation;

	// Try to find a random reachable point
	for (int32 Attempts = 0; Attempts < 10; ++Attempts)
	{
		float Angle = FMath::FRand() * 2.0f * PI;
		float Distance = FMath::FRandRange(WanderRadius * 0.3f, WanderRadius);
		
		FVector Candidate = BotLocation + FVector(
			FMath::Cos(Angle) * Distance,
			FMath::Sin(Angle) * Distance,
			0.0f
		);

		// Check if point is on navmesh
		FNavLocation NavLocation;
		if (NavSystem->ProjectPointToNavigation(Candidate, NavLocation, FVector(200.0f, 200.0f, 100.0f)))
		{
			// Check if path is valid
			FPathFindingQuery Query(this, *NavSystem, BotLocation, NavLocation.Location);
			FPathFindingResult Result = NavSystem->FindPathSync(Query);
			
			if (Result.IsSuccessful() && Result.Path.IsValid())
			{
				CurrentWanderTarget = NavLocation.Location;
				bHasValidTarget = true;
				CurrentWanderDuration = FMath::FRandRange(MinWanderTime, MaxWanderTime);
				WanderTimer = 0.0f;
				
				MoveToWanderTarget();
				return;
			}
		}
	}

	// Fallback: just pick a random direction
	CurrentWanderTarget = BotLocation + FVector(
		FMath::FRandRange(-WanderRadius, WanderRadius),
		FMath::FRandRange(-WanderRadius, WanderRadius),
		0.0f
	);
	bHasValidTarget = true;
	CurrentWanderDuration = FMath::FRandRange(MinWanderTime, MaxWanderTime);
	WanderTimer = 0.0f;
	MoveToWanderTarget();
}

void AStumbleBotController::MoveToWanderTarget()
{
	if (!BotCharacter || !bHasValidTarget) return;

	// Use MoveToLocation for pathfinding
	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalLocation(CurrentWanderTarget);
	MoveRequest.SetAcceptanceRadius(100.0f);
	MoveRequest.SetUsePathfinding(true);
	MoveRequest.SetUsePartialPaths(true);
	MoveRequest.SetCanStrafe(false);

	FAIRequestID RequestID;
	MoveTo(MoveRequest, &RequestID);
	
	// Bind completion delegate
	if (RequestID.IsValid())
	{
		FAIRequestID RequestIDCopy = RequestID;
		FOnRequestFinishedDelegate Delegate;
		Delegate.BindUObject(this, &AStumbleBotController::OnMoveCompleted);
		AddOnRequestFinishedDelegate(RequestIDCopy, Delegate);
	}
}

void AStumbleBotController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	// Target reached or failed - will pick new target next tick
	bHasValidTarget = false;
}

void AStumbleBotController::CheckAndAvoidObstacles(float DeltaTime)
{
	if (!BotCharacter) return;

	FVector BotLocation = BotCharacter->GetActorLocation();
	FVector Forward = BotCharacter->GetActorForwardVector();
	
	// Sphere trace ahead for obstacles
	TArray<FHitResult> HitResults;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(BotCharacter);
	
	bool bHit = GetWorld()->SweepMultiByChannel(
		HitResults,
		BotLocation,
		BotLocation + Forward * ObstacleAvoidanceRadius,
		FQuat::Identity,
		ECC_WorldDynamic,
		FCollisionShape::MakeSphere(150.0f)
	);

	if (bHit)
	{
		FVector AvoidanceForce = FVector::ZeroVector;
		int32 HitCount = 0;

		for (const FHitResult& Hit : HitResults)
		{
			if (Hit.GetActor() && Hit.GetActor() != BotCharacter)
			{
				FVector AwayDir = (BotLocation - Hit.Location).GetSafeNormal();
				AvoidanceForce += AwayDir;
				HitCount++;
			}
		}

		if (HitCount > 0)
		{
			AvoidanceForce = AvoidanceForce.GetSafeNormal() * ObstacleAvoidanceWeight;
			
			// Apply avoidance as movement input
			if (BotCharacter->GetCharacterMovement())
			{
				BotCharacter->GetCharacterMovement()->AddInputVector(AvoidanceForce, true);
			}
		}
	}
}

void AStumbleBotController::MaybeJump()
{
	if (!BotCharacter || !BotCharacter->GetCharacterMovement()) return;
	
	// Only jump if on ground
	if (BotCharacter->GetCharacterMovement()->IsMovingOnGround())
	{
		if (FMath::FRand() < JumpChance * 0.016f) // Per-frame chance (approx 60fps)
		{
			BotCharacter->Jump();
		}
	}
}