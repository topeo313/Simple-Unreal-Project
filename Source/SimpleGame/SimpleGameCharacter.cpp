// Fill out your copyright notice in the Description page of Project Settings.

#include "Animation/AnimNode_StateMachine.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputMappingContext.h"
#include "SimpleGameCharacter.h"

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
	this->bUseControllerRotationPitch = false;
	this->bUseControllerRotationYaw = false;
	this->bUseControllerRotationRoll = false;
	
	UCharacterMovementComponent* CharMovement = this->GetCharacterMovement();
	CharMovement->bOrientRotationToMovement = true; // Update pawn's rotation based on direction of acceleration (based on CharacterMovement RotationRate)
	CharMovement->RotationRate = FRotator(0.f, 540.f, 0.f);
	CharMovement->JumpZVelocity = 380.0f;
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

	UAnimInstance* AnimInstance = this->GetAnimInstance();
	if (AnimInstance != nullptr)
	{
		this->GroundMovementStateMachine = AnimInstance->GetStateMachineInstanceFromName(FName("Ground Movement"));
		this->AirMovementStateMachine = AnimInstance->GetStateMachineInstanceFromName(FName("Air Movement"));
	}
}

void ASimpleGameCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason) 
{
	Super::EndPlay(EndPlayReason);		
}

void ASimpleGameCharacter::SetTimer(
	TDelegate<void(), FDefaultTSDelegateUserPolicy>::TMethodPtr<ASimpleGameCharacter> TimerDelegate, 
	float DurationSeconds)
{
	TWeakObjectPtr<ASimpleGameCharacter> WeakThis(this);

	AsyncTask(ENamedThreads::GameThread, [WeakThis, TimerDelegate, DurationSeconds]() mutable
	{	
		if (WeakThis.IsValid())
		{			
			FTimerHandle TimerHandle;
				
			FTimerDelegate SafeDelegate;
			SafeDelegate.BindUObject(WeakThis.Get(), TimerDelegate);
	
			WeakThis->GetWorldTimerManager().SetTimer(
				TimerHandle,
				SafeDelegate,
				DurationSeconds,
				false
			);
		}			
	});
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

void ASimpleGameCharacter::ResetAttackParameters()
{
	this->AttackStarted = false;
	this->JumpAttackStarted = false;	
	this->SetTimer(&ASimpleGameCharacter::OnAttackCooldownTimerElapsed, ASimpleGameCharacter::AttackCooldownTimeSeconds);
	this->InAttackCooldown = true;	
}

void ASimpleGameCharacter::OnJumpCooldownTimerElapsed()
{
	this->InJumpCooldown = false;
}

void ASimpleGameCharacter::OnAttackCooldownTimerElapsed()
{
	this->InAttackCooldown = false;
}

void ASimpleGameCharacter::Jump()
{
	if (this->GetCharacterMovement()->IsFalling() || this->InJumpCooldown || this->IsAttackStarted())
	{
		return;
	}
	
	Super::Jump();
}

void ASimpleGameCharacter::Landed(const FHitResult& Hit)
{
	this->SetTimer(&ASimpleGameCharacter::OnJumpCooldownTimerElapsed, ASimpleGameCharacter::JumpCooldownTimeSeconds);
	this->InJumpCooldown = true;
}

bool ASimpleGameCharacter::IsAttacking()
{	 
	if (this->GroundMovementStateMachine == nullptr || this->AirMovementStateMachine == nullptr)
	{
		return false;
	}
	
	return this->GroundMovementStateMachine->GetCurrentStateName() == FName("Attack") ||
		   this->AirMovementStateMachine->GetCurrentStateName() == FName("Jump Attack");
}

void ASimpleGameCharacter::Move(const FInputActionValue& Value)
{
	if (this->IsAttackStarted() || this->IsAttacking())
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
	// To prevent the character animations from looking stuttery or jumpy, we avoid going into the attack state if the character
	// is both moving AND trying to attack within the attack cooldown period (i.e. rapidly pressing the attack button). We check for 
	// both movement and the cooldown because moving while mashing the attack button confuses the state machine, so we want to be
	// extra sure that we guard against the jittery/jumpy animations
	if (this->InAttackCooldown && this->IsPlayerMovementInputEnabled())
	{
		return;
	}
	else if (this->GetCharacterMovement()->IsFalling())
	{
		if (this->AirMovementStateMachine->GetCurrentStateName() == FName("Jump Apex"))
		{
			this->JumpAttackStarted = true;
		}
		
		return;
	}

	this->AttackStarted = true;
}
