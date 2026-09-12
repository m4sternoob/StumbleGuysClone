// Copyright (c) 2026. Built for portfolio. Free to study.

#include "Obstacles/StumbleObstacleSpinner.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

AStumbleObstacleSpinner::AStumbleObstacleSpinner()
{
	// Spinner uses a long thin box as the rotating bar
	BarExtent = FVector(BarThickness / 2.0f, BarLength / 2.0f, BarThickness / 2.0f);

	if (ObstacleMesh)
	{
		ObstacleMesh->SetRelativeScale3D(FVector(
			BarThickness / 100.0f,
			BarLength / 100.0f,
			BarThickness / 100.0f
		));
	}

	if (CollisionBox)
	{
		CollisionBox->SetBoxExtent(BarExtent);
	}

	// Distinct color for spinner
	ObstacleColor = FLinearColor(0.9f, 0.3f, 0.1f, 1.0f);
}

void AStumbleObstacleSpinner::BeginPlay()
{
	Super::BeginPlay();
}

void AStumbleObstacleSpinner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!HasAuthority() || !bIsActive) return;

	// Rotate around Z axis
	float CurrentYaw = GetActorRotation().Yaw;
	float DeltaYaw = RotationSpeed * DeltaTime;
	if (bReverseDirection) DeltaYaw = -DeltaYaw;

	SetActorRotation(FRotator(0.0f, CurrentYaw + DeltaYaw, 0.0f));
}