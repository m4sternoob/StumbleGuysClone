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

	// Replication
	bReplicates = true;
	SetReplicateMovement(true); // CharacterMovementComponent handles movement replication

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

	// Follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	FollowCamera->FieldOfView = 90.0f;

	// Net update frequency
	NetUpdateFrequency = 60.0f;
	MinNetUpdateFrequency = 30.0f;
}

void AStumbleCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AStumbleCharacter, PlayerColor);
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

	if (Controller && !MovementVector.IsNearlyZero())
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
	Jump();
}

void AStumbleCharacter::OnJumpStopped()
{
	StopJumping();
}

void AStumbleCharacter::OnRep_PlayerColor()
{
	ApplyPlayerColor();
}

void AStumbleCharacter::ApplyPlayerColor()
{
	if (HeadSphere)
	{
		UMaterialInstanceDynamic* DynMat = HeadSphere->CreateAndSetMaterialInstanceDynamic(0);
		if (DynMat)
		{
			DynMat->SetVectorParameterValue(TEXT("EmissiveColor"), PlayerColor);
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