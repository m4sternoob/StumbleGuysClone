// Copyright (c) 2026. Built for portfolio. Free to study.

#include "Obstacles/StumbleObstacleBase.h"
#include "Character/StumbleCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"
#include "UObject/ConstructorHelpers.h"
#include "Net/UnrealNetwork.h"

AStumbleObstacleBase::AStumbleObstacleBase()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);

	// Root
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	// Obstacle mesh
	ObstacleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ObstacleMesh"));
	ObstacleMesh->SetupAttachment(RootComponent);
	ObstacleMesh->SetCollisionProfileName(TEXT("BlockAll"));
	ObstacleMesh->SetMobility(EComponentMobility::Movable);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		ObstacleMesh->SetStaticMesh(CubeMesh.Object);
	}

	// Collision box for overlap detection
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(RootComponent);
	CollisionBox->SetCollisionProfileName(TEXT("OverlapAll"));
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionBox->SetGenerateOverlapEvents(true);
	CollisionBox->SetMobility(EComponentMobility::Movable);

	// Net
	NetUpdateFrequency = 30.0f;
}

void AStumbleObstacleBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AStumbleObstacleBase, bIsActive);
}

void AStumbleObstacleBase::BeginPlay()
{
	Super::BeginPlay();

	if (CollisionBox)
	{
		CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AStumbleObstacleBase::OnOverlapBegin);
	}

	ApplyVisualMaterial();
}

void AStumbleObstacleBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// Override in children for movement/rotation
}

void AStumbleObstacleBase::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority() || !bIsActive) return;

	AStumbleCharacter* Character = Cast<AStumbleCharacter>(OtherActor);
	if (Character && !Character->IsPendingKill())
	{
		ApplyObstacleEffect(Character, SweepResult);
	}
}

void AStumbleObstacleBase::ApplyObstacleEffect(AStumbleCharacter* Character, const FHitResult& HitResult)
{
	if (!Character) return;

	// Apply impulse away from obstacle
	FVector ImpulseDir = (Character->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	if (ImpulseDir.IsNearlyZero())
	{
		ImpulseDir = FVector::UpVector;
	}

	Character->GetCharacterMovement()->AddImpulse(ImpulseDir * DamageImpulse, true);
}

void AStumbleObstacleBase::ApplyVisualMaterial()
{
	if (ObstacleMesh)
	{
		static ConstructorHelpers::FObjectFinder<UMaterial> BaseMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
		if (BaseMaterial.Succeeded())
		{
			UMaterialInstanceDynamic* DynMat = ObstacleMesh->CreateAndSetMaterialInstanceDynamic(0);
			if (DynMat)
			{
				DynMat->SetVectorParameterValue(TEXT("BaseColor"), ObstacleColor);
				DynMat->SetScalarParameterValue(TEXT("Metallic"), 0.1f);
				DynMat->SetScalarParameterValue(TEXT("Roughness"), 0.7f);
				DynMat->SetVectorParameterValue(TEXT("EmissiveColor"), ObstacleColor * 0.3f);
			}
		}
	}
}