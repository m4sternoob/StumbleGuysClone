// Copyright (c) 2026. Built for portfolio. Free to study.

#include "Obstacles/StumbleObstacleMovingPlatform.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

AStumbleObstacleMovingPlatform::AStumbleObstacleMovingPlatform()
{
	if (ObstacleMesh)
	{
		ObstacleMesh->SetRelativeScale3D(FVector(
			PlatformThickness / 100.0f,
			PlatformLength / 100.0f,
			PlatformWidth / 100.0f
		));
	}

	if (CollisionBox)
	{
		CollisionBox->SetBoxExtent(FVector(
			PlatformThickness / 2.0f,
			PlatformLength / 2.0f,
			PlatformWidth / 2.0f
		));
	}

	// Distinct color for moving platform
	ObstacleColor = FLinearColor(0.2f, 0.7f, 0.9f, 1.0f);
}

void AStumbleObstacleMovingPlatform::BeginPlay()
{
	Super::BeginPlay();

	StartLocation = GetActorLocation();
	MoveAxis.Normalize();
	TargetLocation = StartLocation + MoveAxis * MoveDistance;
}

void AStumbleObstacleMovingPlatform::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!HasAuthority() || !bIsActive) return;

	FVector CurrentLocation = GetActorLocation();
	FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();
	
	float DistanceThisFrame = MoveSpeed * DeltaTime * MoveDirection;
	FVector NewLocation = CurrentLocation + Direction * DistanceThisFrame;

	// Check if we've reached or passed target
	float DistanceToTarget = FVector::Dist(NewLocation, TargetLocation);
	float DistanceToStart = FVector::Dist(NewLocation, StartLocation);

	if (bPingPong)
	{
		if (DistanceToTarget < DistanceThisFrame)
		{
			// Reached target, reverse
			NewLocation = TargetLocation;
			MoveDirection *= -1;
			Swap(StartLocation, TargetLocation);
		}
	}
	else
	{
		// Loop mode - teleport back to start
		if (DistanceToTarget < DistanceThisFrame)
		{
			NewLocation = StartLocation;
		}
	}

	SetActorLocation(NewLocation);
}