// Copyright (c) 2026. Built for portfolio. Free to study.

#include "Character/StumbleCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Controller.h"
#include "PhysicalMaterial.h"

AStumbleCharacter::AStumbleCharacter()
{
	// Disable controller rotation — camera follows, character turns to face input
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Capsule (body) — replicated by default in ACharacter
	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));
	GetCapsuleComponent()->SetCanEverAffectNavigation(false);

	// Create and assign physics material for character (bouncy, low friction)
	UPhysicalMaterial* CharacterPhysMat = CreateDefaultSubobject<UPhysicalMaterial>(TEXT("CharacterPhysMat"));
	CharacterPhysMat->Friction = 0.1f;
	CharacterPhysMat->Restitution = 0.6f;
	CharacterPhysMat->RestitutionCombineMode = EPhysicalMaterialCombineMode::Average;
	CharacterPhysMat->FrictionCombineMode = EPhysicalMaterialCombineMode::Min;
	GetCapsuleComponent()->SetPhysMaterialOverride(CharacterPhysMat);
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));
	GetCapsuleComponent()->SetCanEverAffectNavigation(false);

	// Replication - CharacterMovementComponent handles movement replication
	bReplicates = true;
	SetReplicateMovement(true);

	// Head sphere (visual only, no collision)
	HeadSphere = CreateDefaultSubobject<USphereComponent>(TEXT("HeadSphere"));
	HeadSphere->SetupAttachment(GetCapsuleComponent());
	HeadSphere->SetRelativeLocation(FVector(0.0f, 0.0f, 96.0f));
	HeadSphere->SetSphereRadius(18.0f);
	HeadSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HeadSphere->SetHiddenInGame(false);

	// Character movement defaults
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->MaxWalkSpeed = MaxWalkSpeed;
	GetCharacterMovement()->JumpZVelocity = JumpZVelocity;
	GetCharacterMovement()->AirControl = AirControl;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = false;
	GetCharacterMovement()->NavAgentProps.bCanJump = true;
	GetCharacterMovement()->bCanWalkOffLedgesWhenCrouching = false;

	// Network settings
	NetUpdateFrequency = 60.0f;
	MinNetUpdateFrequency = 30.0f;

	// Camera boom (spring arm) — follows character, doesn't rotate with controller
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = false;
	CameraBoom->bDoCollisionTest = true;
	CameraBoom->bInheritPitch = false;
	CameraBoom->bInheritYaw = false;
	CameraBoom->bInheritRoll = false;
	CameraBoom->ProbeSize = 12.0f;
	CameraBoom->ProbeChannel = ECC_Camera;

	// Enable camera lag for smooth following
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->bEnableCameraRotationLag = true;
	CameraBoom->CameraLagSpeed = CameraLagSpeed;
	CameraBoom->CameraLagMaxDistance = CameraLagMaxDistance;

	// Follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	FollowCamera->FieldOfView = 90.0f;

	// Default values for input assets
	DefaultMappingContext = nullptr;
	MoveAction = nullptr;
	JumpAction = nullptr;
}

void AStumbleCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AStumbleCharacter, PlayerColor);
	DOREPLIFETIME(AStumbleCharacter, bIsEliminated);
}

void AStumbleCharacter::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);
	// Track position/velocity changes for bandwidth optimization
	// CharacterMovementComponent handles most of this automatically
}

void AStumbleCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Add Input Mapping Context (only on locally controlled pawn)
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (IsLocallyControlled() && DefaultMappingContext)
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}

	ApplyPlayerColor();
}

void AStumbleCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AStumbleCharacter::Move);
		}
		if (JumpAction)
		{
			EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &AStumbleCharacter::OnJumpStarted);
			EnhancedInput->BindAction(JumpAction, ETriggerEvent::Completed, this, &AStumbleCharacter::OnJumpStopped);
		}
	}
}

void AStumbleCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller && !MovementVector.IsNearlyZero() && !bIsEliminated)
	{
		// Camera-relative movement (uses follow camera yaw)
		const FRotator CameraRotation = FollowCamera ? FollowCamera->GetComponentRotation() : GetControlRotation();
		const FRotator YawRotation(0.0f, CameraRotation.Yaw, 0.0f);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AStumbleCharacter::OnJumpStarted()
{
	if (!bIsEliminated)
	{
		Jump();
	}
}

void AStumbleCharacter::OnJumpStopped()
{
	StopJumping();
}

void AStumbleCharacter::OnRep_PlayerColor()
{
	ApplyPlayerColor();
}

void AStumbleCharacter::OnRep_Eliminated()
{
	if (bIsEliminated)
	{
		// Visual feedback on elimination
		GetMesh()->SetVisibility(false);
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if (HeadSphere)
		{
			HeadSphere->SetVisibility(false);
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		if (HeadSphere)
		{
			HeadSphere->SetVisibility(true);
		}
	}
}

void AStumbleCharacter::ApplyPlayerColor()
{
	if (HeadSphere)
	{
		UMaterialInstanceDynamic* DynMat = HeadSphere->CreateAndSetMaterialInstanceDynamic(0);
		if (DynMat)
		{
			DynMat->SetVectorParameterValue(TEXT("EmissiveColor"), PlayerColor);
			DynMat->SetScalarParameterValue(TEXT("EmissiveStrength"), 2.0f);
		}
	}
	
	// Also tint capsule
	if (GetCapsuleComponent())
	{
		UMaterialInstanceDynamic* DynMat = GetCapsuleComponent()->CreateAndSetMaterialInstanceDynamic(0);
		if (DynMat)
		{
			DynMat->SetVectorParameterValue(TEXT("BaseColor"), PlayerColor);
		}
	}
}

void AStumbleCharacter::SetEliminated(bool bEliminated)
{
	if (HasAuthority())
	{
		bIsEliminated = bEliminated;
	}
}

void AStumbleCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Camera smoothing - only for locally controlled pawn
	if (IsLocallyControlled() && FollowCamera && CameraBoom)
	{
		UpdateCameraLag(DeltaTime);
	}
}

void AStumbleCharacter::UpdateCameraLag(float DeltaTime)
{
	SmoothCameraRotation(DeltaTime);
	UpdateCameraLagPosition(DeltaTime);
}

void AStumbleCharacter::SmoothCameraRotation(float DeltaTime)
{
	if (!CameraBoom || !FollowCamera) return;

	// Smooth camera rotation using interpolation
	const float RotationLagSpeed = FMath::Clamp(CameraLagSpeed * 0.5f, 1.0f, 20.0f);
	
	FRotator TargetRotation = CameraBoom->GetTargetRotation();
	FRotator CurrentRotation = CameraBoom->GetComponentRotation();
	
	FRotator SmoothedRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, RotationLagSpeed);
	CameraBoom->SetWorldRotation(SmoothedRotation);
}

void AStumbleCharacter::UpdateCameraLagPosition(float DeltaTime)
{
	if (!CameraBoom || !FollowCamera) return;

	if (!bCameraInitialized)
	{
		LastCameraLocation = CameraBoom->GetComponentLocation();
		LastCameraRotation = CameraBoom->GetComponentRotation();
		bCameraInitialized = true;
		return;
	}

	// Smooth camera position with configurable lag
	const float PositionLagSpeed = FMath::Clamp(CameraLagSpeed, 2.0f, 30.0f);
	
	FVector TargetLocation = CameraBoom->GetComponentLocation();
	FVector SmoothedLocation = FMath::VInterpTo(LastCameraLocation, TargetLocation, DeltaTime, PositionLagSpeed);
	
	// Clamp max distance to prevent camera from lagging too far behind
	FVector Delta = SmoothedLocation - LastCameraLocation;
	float Distance = Delta.Size();
	if (Distance > CameraLagMaxDistance)
	{
		Delta = Delta.GetSafeNormal() * CameraLagMaxDistance;
		SmoothedLocation = LastCameraLocation + Delta;
	}

		LastCameraLocation = SmoothedLocation;
	}