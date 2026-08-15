// Fill out your copyright notice in the Description page of Project Settings.

#include "SimpleGameCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"

// Sets default values
ASimpleGameCharacter::ASimpleGameCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create CameraArm component
	this->CameraArm = this->CreateDefaultSubobject<USpringArmComponent>(TEXT("Camera Arm"));
	this->CameraArm->SetupAttachment(this->RootComponent);
	this->CameraArm->TargetArmLength = 425.0f;
	this->CameraArm->bUsePawnControlRotation = true; // Update camera arm with controller's rotation
	
	this->Camera = this->CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	this->Camera->SetupAttachment(this->CameraArm, USpringArmComponent::SocketName);
	this->Camera->bUsePawnControlRotation = false; // No need to update camera's rotation here (since camera arm gets updated)
	
	// Enable/disable to get this Pawn to match/ignore the controller's pitch, yaw, roll
	this->bUseControllerRotationPitch = true;
	this->bUseControllerRotationYaw = false;
	this->bUseControllerRotationRoll = false;
	
	UCharacterMovementComponent* CharMovement = this->GetCharacterMovement();
	CharMovement->bOrientRotationToMovement = true; // Update pawn's rotation based on direction of acceleration (based on CharacterMovement RotationRate)
	CharMovement->RotationRate = FRotator(0.f, 540.f, 0.f);
	CharMovement->JumpZVelocity = 500.0f;
	CharMovement->AirControl = 0.2f;
	
	// Cut top speed down (Default is usually 600.0f)
	CharMovement->MaxWalkSpeed = 375.0f;
	
	this->SmoothedMovementVector = FVector2D::ZeroVector;
}

// Called when the game starts or when spawned
void ASimpleGameCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	APlayerController* PlayerController = ::Cast<APlayerController>(this->Controller);
	if (PlayerController != nullptr)
	{
		UEnhancedInputLocalPlayerSubsystem* PcSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
		if (PcSubsystem != nullptr)
		{
			PcSubsystem->AddMappingContext(this->PlayerMappingContext, 0);
		}
	}
}

// Called every frame
void ASimpleGameCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ASimpleGameCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	UEnhancedInputComponent* EnhancedInputComponent = ::CastChecked<UEnhancedInputComponent>(PlayerInputComponent);
	EnhancedInputComponent->BindAction(this->MoveAction, ETriggerEvent::Triggered, this, &ASimpleGameCharacter::Move);
	EnhancedInputComponent->BindAction(this->LookAroundAction, ETriggerEvent::Triggered, this, &ASimpleGameCharacter::LookAround);
	
	EnhancedInputComponent->BindAction(this->JumpAction, ETriggerEvent::Started, this, &ASimpleGameCharacter::Jump);
	EnhancedInputComponent->BindAction(this->JumpAction, ETriggerEvent::Completed, this, &ASimpleGameCharacter::StopJumping);
	
	EnhancedInputComponent->BindAction(this->AttackA_Action, ETriggerEvent::Started, this, &ASimpleGameCharacter::Attack_A_Started);
}

bool ASimpleGameCharacter::IsAttacking() const
{
	UAnimInstance* AnimInstance = this->GetAnimInstance();
	if (AnimInstance == nullptr)
	{
		return false;
	}
	
	return AnimInstance->Montage_IsPlaying(this->AttackA_Montage);
}

void ASimpleGameCharacter::Move(const FInputActionValue& Value)
{
	if (this->IsAttacking())
	{
		return;
	}

	// Extract 2D axis data (x and y) 
	FVector2D MovementVector = Value.Get<FVector2D>();
	
	// Check to ensure that input vector hits a certain threshold before triggering (e.g. for detecting left joystick movement)
	if (MovementVector.Size() < ASimpleGameCharacter::MoveThreshold)
	{
		return;
	}		
		
	if (this->Controller != nullptr)
	{
		// Extract yaw rotation info
		FRotator Rotation = this->Controller->GetControlRotation();
		FRotator YawRotation(0, Rotation.Yaw, 0);
		
		// Create rotation matrix based on the yaw
		FRotationMatrix RotationMatrix = FRotationMatrix(YawRotation);
		FVector ForwardDirection = RotationMatrix.GetUnitAxis(EAxis::X); // In Unreal, X is forward/backward
		FVector RightDirection = RotationMatrix.GetUnitAxis(EAxis::Y); // In Unreal, Y is left/right
		
		// To keep movement smooth, interpolate between the current movement vector and the calculated movement vector
		this->SmoothedMovementVector = FMath::Vector2DInterpTo(this->SmoothedMovementVector, MovementVector, this->GetWorld()->GetDeltaSeconds(), ASimpleGameCharacter::MoveInterpolationSpeed);
		this->AddMovementInput(ForwardDirection, this->SmoothedMovementVector.Y); // In Unreal, Y is where we choose to store W/S movement (hence why we swizzle in the InputMappingContext) 
		this->AddMovementInput(RightDirection, this->SmoothedMovementVector.X); // In Unreal, X is where we choose to store A/D movement
	}
}

void ASimpleGameCharacter::LookAround(const FInputActionValue& Value)
{
	FVector2D TurnVector = Value.Get<FVector2D>();
	this->AddControllerYawInput(TurnVector.X);
	this->AddControllerPitchInput(TurnVector.Y);
}

void ASimpleGameCharacter::Attack_A_Started()
{
	UAnimInstance* AnimInstance = this->GetAnimInstance();
	if (AnimInstance != nullptr)
	{
		if (this->AttackA_Montage != nullptr)
		{
			if (!AnimInstance->Montage_IsPlaying(this->AttackA_Montage))
			{	
				AnimInstance->Montage_Play(this->AttackA_Montage, this->AttackA_PlayRate);				
			}
		}	
	}
}