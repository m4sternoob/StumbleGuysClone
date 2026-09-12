// Copyright (c) 2026. Built for portfolio. Free to study.

#include "Collectibles/StumbleCollectible.h"
#include "Character/StumbleCharacter.h"
#include "GameMode/StumblePlayerState.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"
#include "UObject/ConstructorHelpers.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

AStumbleCollectible::AStumbleCollectible()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(false);

	// Root
	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	SetRootComponent(RootScene);

	// Collectible mesh
	CollectibleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CollectibleMesh"));
	CollectibleMesh->SetupAttachment(RootScene);
	CollectibleMesh->SetCollisionProfileName(TEXT("NoCollision"));
	CollectibleMesh->SetMobility(EComponentMobility::Movable);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		CollectibleMesh->SetStaticMesh(CubeMesh.Object);
		CollectibleMesh->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.5f));
	}

	// Overlap sphere
	OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
	OverlapSphere->SetupAttachment(RootScene);
	OverlapSphere->SetSphereRadius(80.0f);
	OverlapSphere->SetCollisionProfileName(TEXT("OverlapAll"));
	OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OverlapSphere->SetGenerateOverlapEvents(true);
	OverlapSphere->SetMobility(EComponentMobility::Movable);

	// Net
	NetUpdateFrequency = 10.0f;
}

void AStumbleCollectible::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AStumbleCollectible, bCollected);
}

void AStumbleCollectible::BeginPlay()
{
	Super::BeginPlay();

	InitialLocation = GetActorLocation();

	if (OverlapSphere)
	{
		OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &AStumbleCollectible::OnOverlapBegin);
	}

	ApplyVisualMaterial();
}

void AStumbleCollectible::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bCollected) return;

	// Floating animation
	FloatTime += DeltaTime * FloatSpeed;
	float FloatOffset = FMath::Sin(FloatTime) * FloatAmplitude;
	FVector NewLocation = InitialLocation;
	NewLocation.Z += FloatOffset;
	SetActorLocation(NewLocation);

	// Rotation
	AddActorLocalRotation(FRotator(0.0f, RotationSpeed * DeltaTime, 0.0f));
}

void AStumbleCollectible::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bCollected) return;

	AStumbleCharacter* Character = Cast<AStumbleCharacter>(OtherActor);
	if (Character && !Character->IsPendingKill())
	{
		Server_Collect(Character);
	}
}

void AStumbleCollectible::Server_Collect_Implementation(AActor* Collector)
{
	if (bCollected || !HasAuthority()) return;

	AStumbleCharacter* Character = Cast<AStumbleCharacter>(Collector);
	if (!Character) return;

	// Award score to player
	if (APlayerController* PC = Cast<APlayerController>(Character->GetController()))
	{
		if (AStumblePlayerState* PS = PC->GetPlayerState<AStumblePlayerState>())
		{
			PS->AddCollectible();
		}
	}

	// Mark collected
	bCollected = true;
	CollectibleMesh->SetVisibility(false);
	OverlapSphere->SetGenerateOverlapEvents(false);

	// Respawn if enabled
	if (bRespawn)
	{
		GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &AStumbleCollectible::RespawnCollectible, RespawnTime, false);
	}
	else
	{
		// Destroy after short delay for effect
		SetLifeSpan(2.0f);
	}
}

bool AStumbleCollectible::Server_Collect_Validate(AActor* Collector)
{
	return true;
}

void AStumbleCollectible::ApplyVisualMaterial()
{
	if (CollectibleMesh)
	{
		static ConstructorHelpers::FObjectFinder<UMaterial> BaseMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
		if (BaseMaterial.Succeeded())
		{
			UMaterialInstanceDynamic* DynMat = CollectibleMesh->CreateAndSetMaterialInstanceDynamic(0);
			if (DynMat)
			{
				DynMat->SetVectorParameterValue(TEXT("BaseColor"), CollectibleColor);
				DynMat->SetVectorParameterValue(TEXT("EmissiveColor"), CollectibleColor * 2.0f);
				DynMat->SetScalarParameterValue(TEXT("Metallic"), 0.5f);
				DynMat->SetScalarParameterValue(TEXT("Roughness"), 0.3f);
			}
		}
	}
}

void AStumbleCollectible::RespawnCollectible()
{
	if (!HasAuthority()) return;

	bCollected = false;
	CollectibleMesh->SetVisibility(true);
	OverlapSphere->SetGenerateOverlapEvents(true);
	SetActorLocation(InitialLocation);
	FloatTime = 0.0f;
}