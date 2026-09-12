// Copyright (c) 2026. Built for portfolio. Free to study.

#include "Arena/StumbleArena.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"
#include "UObject/ConstructorHelpers.h"
#include "Net/UnrealNetwork.h"
#include "Character/StumbleCharacter.h"

AStumbleArena::AStumbleArena()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(false); // Static arena, no movement replication needed

	// Root component
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	CreateFloor();
	CreateWalls();
	CreateKillZone();
}

void AStumbleArena::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// Arena is static after spawn - no runtime property replication needed
}

void AStumbleArena::BeginPlay()
{
	Super::BeginPlay();

	if (KillZoneVolume)
	{
		KillZoneVolume->OnComponentBeginOverlap.AddDynamic(this, &AStumbleArena::OnKillZoneOverlap);
	}
}

void AStumbleArena::CreateFloor()
{
	FloorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FloorMesh"));
	FloorMesh->SetupAttachment(RootComponent);
	FloorMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -50.0f));
	FloorMesh->SetCollisionProfileName(TEXT("BlockAll"));
	FloorMesh->SetMobility(EComponentMobility::Static);

	// Use engine default cube scaled to floor dimensions
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		FloorMesh->SetStaticMesh(CubeMesh.Object);
		FloorMesh->SetWorldScale3D(FVector(
			(ArenaSize + 2 * WallThickness) / 100.0f,
			(ArenaSize + 2 * WallThickness) / 100.0f,
			1.0f
		));
	}

	// Simple material - dark ground
	static ConstructorHelpers::FObjectFinder<UMaterial> FloorMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (FloorMaterial.Succeeded())
	{
		UMaterialInstanceDynamic* DynMat = FloorMesh->CreateAndSetMaterialInstanceDynamic(0);
		if (DynMat)
		{
			DynMat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.15f, 0.12f, 0.1f, 1.0f));
			DynMat->SetScalarParameterValue(TEXT("Metallic"), 0.0f);
			DynMat->SetScalarParameterValue(TEXT("Roughness"), 0.9f);
		}
	}
}

void AStumbleArena::CreateWalls()
{
	const float HalfSize = ArenaSize / 2.0f;
	const float HalfThick = WallThickness / 2.0f;
	const float HalfHeight = WallHeight / 2.0f;

	struct FWallSpec
	{
		FVector Location;
		FVector Scale;
	};

	FWallSpec Walls[4] = {
		// Left wall (-X)
		{ FVector(-HalfSize - HalfThick, 0.0f, HalfHeight), FVector(WallThickness / 100.0f, (ArenaSize + 2 * WallThickness) / 100.0f, WallHeight / 100.0f) },
		// Right wall (+X)
		{ FVector(HalfSize + HalfThick, 0.0f, HalfHeight), FVector(WallThickness / 100.0f, (ArenaSize + 2 * WallThickness) / 100.0f, WallHeight / 100.0f) },
		// Back wall (-Y)
		{ FVector(0.0f, -HalfSize - HalfThick, HalfHeight), FVector((ArenaSize + 2 * WallThickness) / 100.0f, WallThickness / 100.0f, WallHeight / 100.0f) },
		// Front wall (+Y)
		{ FVector(0.0f, HalfSize + HalfThick, HalfHeight), FVector((ArenaSize + 2 * WallThickness) / 100.0f, WallThickness / 100.0f, WallHeight / 100.0f) },
	};

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UMaterial> WallMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));

	for (int32 i = 0; i < 4; ++i)
	{
		WallMeshes[i] = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("WallMesh_%d"), i));
		WallMeshes[i]->SetupAttachment(RootComponent);
		WallMeshes[i]->SetRelativeLocation(Walls[i].Location);
		WallMeshes[i]->SetWorldScale3D(Walls[i].Scale);
		WallMeshes[i]->SetCollisionProfileName(TEXT("BlockAll"));
		WallMeshes[i]->SetMobility(EComponentMobility::Static);

		if (CubeMesh.Succeeded())
		{
			WallMeshes[i]->SetStaticMesh(CubeMesh.Object);
		}

		if (WallMaterial.Succeeded())
		{
			UMaterialInstanceDynamic* DynMat = WallMeshes[i]->CreateAndSetMaterialInstanceDynamic(0);
			if (DynMat)
			{
				DynMat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.1f, 0.1f, 0.12f, 1.0f));
				DynMat->SetScalarParameterValue(TEXT("Metallic"), 0.0f);
				DynMat->SetScalarParameterValue(TEXT("Roughness"), 0.8f);
			}
		}
	}
}

void AStumbleArena::CreateKillZone()
{
	KillZoneVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("KillZoneVolume"));
	KillZoneVolume->SetupAttachment(RootComponent);
	KillZoneVolume->SetRelativeLocation(FVector(0.0f, 0.0f, KillZoneDepth));
	KillZoneVolume->SetBoxExtent(FVector(ArenaSize * 1.5f, ArenaSize * 1.5f, 100.0f));
	KillZoneVolume->SetCollisionProfileName(TEXT("OverlapAll"));
	KillZoneVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	KillZoneVolume->SetGenerateOverlapEvents(true);
	KillZoneVolume->SetHiddenInGame(true);
	KillZoneVolume->SetMobility(EComponentMobility::Static);
}

void AStumbleArena::OnKillZoneOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority()) return; // Only server handles elimination

	AStumbleCharacter* Character = Cast<AStumbleCharacter>(OtherActor);
	if (Character && !Character->IsPendingKill())
	{
		// Notify GameMode to handle elimination
		if (AGameModeBase* GM = GetWorld()->GetAuthGameMode())
		{
			if (AStumbleGameMode* StumbleGM = Cast<AStumbleGameMode>(GM))
			{
				StumbleGM->EliminatePlayer(Character);
			}
		}
	}
}