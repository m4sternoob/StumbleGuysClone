// Copyright (c) 2026. Built for portfolio. Free to study.

#include "StumbleCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"

AStumbleCharacter::AStumbleCharacter()
{
	// Disable controller rotation — camera follows, character turns to face input
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Capsule (body) — default from ACharacter, but tune size
	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));

	// Head sphere (visual only, no collision)
	HeadSphere = CreateDefaultSubobject<USphereComponent>(TEXT("HeadSphere"));
	HeadSphere->SetupAttachment(GetCapsuleComponent());
	HeadSphere->SetRelativeLocation(FVector(0.0f, 0.0f, 96.0f)); // Top of capsule
	HeadSphere->SetSphereRadius(18.0f);
	HeadSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HeadSphere->SetHiddenInGame(false);

	// Character movement defaults
	GetCharacterMovement()->bOrientRotationToMovement = true; // Face movement direction
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->MaxWalkSpeed = MaxWalkSpeed;
	GetCharacterMovement()->JumpZVelocity = JumpZVelocity;
	GetCharacterMovement()->AirControl = AirControl;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;

	// Camera boom (spring arm)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // Distance from character
	CameraBoom->bUsePawnControlRotation = false; // Don't rotate with controller
	CameraBoom->bDoCollisionTest = true;
	CameraBoom->bInheritPitch = false;
	CameraBoom->bInheritYaw = false;
	CameraBoom->bInheritRoll = false;

	// Follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Default values for input assets (set in editor or via constructor helpers)
	DefaultMappingContext = nullptr;
	MoveAction = nullptr;
	JumpAction = nullptr;
}

void AStumbleCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}
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
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller && !MovementVector.IsNearlyZero())
	{
		// Find forward/right vectors relative to camera (not controller)
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