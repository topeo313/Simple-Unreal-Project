// Fill out your copyright notice in the Description page of Project Settings.

#include "Animation/AnimNode_StateMachine.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputMappingContext.h"
#include "LoggerHelper.h"
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
		
	// Set these properties to match the coresponding property values in the "Blend Poses by bool" node
	this->BlockEndBlendTracker.SetBlendTime(0.15f);
	this->BlockEndBlendTracker.SetBlendOption(EAlphaBlendOption::HermiteCubic);
	
	this->SmoothedMovementVector = FVector2D::ZeroVector;
	this->PrevControlYaw = this->GetControlRotation().Yaw;
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

// Called every frame
void ASimpleGameCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);	
	
	bool performingOtherAction = this->GetCharacterMovement()->IsFalling() || this->IsAttacking();	
	if (!performingOtherAction)
	{
		if (this->IsInBlockMode())
		{
			if (this->IsBlockCameraSet)
			{
				this->bUseControllerRotationYaw = true;
				
				// Trigger foot movement (i.e. set IsRotatingForBlock to true) only when we rotate
				double CurControlYaw = this->GetControlRotation().Yaw;
				this->IsRotatingForBlock = !FMath::IsNearlyEqual(CurControlYaw, this->PrevControlYaw);
				this->PrevControlYaw = CurControlYaw;	
			}
			else
			{
				this->GetCharacterMovement()->bOrientRotationToMovement = false;
				
				FRotator CurControlRotation = this->GetControlRotation();
				FRotator DestControlRotation = this->GetControlRotation();
				DestControlRotation.Pitch = 0;
				FRotator SmoothDestControlRotation = FMath::RInterpTo(CurControlRotation, DestControlRotation,
											   this->GetWorld()->GetDeltaSeconds(),
											   ASimpleGameCharacter::MoveInterpolationSpeed / 2);
				this->GetController()->SetControlRotation(SmoothDestControlRotation);			
				
				FRotator CurActorRotation = this->GetActorRotation();
				FRotator DestActorRotation = this->GetControlRotation();
				DestActorRotation.Pitch = 0;
				FRotator SmoothDestActorRotation = FMath::RInterpTo(CurActorRotation, DestActorRotation,
															   this->GetWorld()->GetDeltaSeconds(),
															   ASimpleGameCharacter::MoveInterpolationSpeed / 2);
				this->SetActorRotation(SmoothDestActorRotation);
								
				if (this->GetActorRotation().Equals(DestActorRotation, 1.0) &&
					this->GetControlRotation().Equals(DestControlRotation, 1.0))
				{
					this->IsRotatingForBlock = false;
					this->IsBlockCameraSet = true;
				}				
			}		
		}
	}
	
	// Use IsBlockAnimationEnded to determine when to reset Block parameters and begin Block cooldown
	if (this->IsBlockAnimationEnded())
	{
		this->BlockEndBlendTracker.Update(DeltaTime);		
		if (this->BlockEndBlendTracker.IsComplete())
		{
			this->ResetBlockParameters();
		}		
	}
}

// Called to bind functionality to input
void ASimpleGameCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	UEnhancedInputComponent* EnhancedInputComponent = ::CastChecked<UEnhancedInputComponent>(PlayerInputComponent);
	EnhancedInputComponent->BindAction(this->MoveAction, ETriggerEvent::Triggered, this, &ASimpleGameCharacter::Move);
	EnhancedInputComponent->BindAction(this->LookAroundAction, ETriggerEvent::Triggered, this, &ASimpleGameCharacter::LookAround);
	
	EnhancedInputComponent->BindAction(this->BottomGamepadAction, ETriggerEvent::Started, this, &ASimpleGameCharacter::BottomGamepadStarted);
	EnhancedInputComponent->BindAction(this->BottomGamepadAction, ETriggerEvent::Completed, this, &ASimpleGameCharacter::BottomGamepadCompleted);
	
	EnhancedInputComponent->BindAction(this->LeftGamepadAction, ETriggerEvent::Started, this, &ASimpleGameCharacter::LeftGamepadStarted);
	
	EnhancedInputComponent->BindAction(this->TopGamepadAction, ETriggerEvent::Started, this, &ASimpleGameCharacter::TopGamepadStarted);
	
	EnhancedInputComponent->BindAction(this->RightGamepadAction, ETriggerEvent::Started, this, &ASimpleGameCharacter::RightGamepadStarted);
	
	EnhancedInputComponent->BindAction(this->BlockAction, ETriggerEvent::Started, this, &ASimpleGameCharacter::BlockStarted);
	EnhancedInputComponent->BindAction(this->BlockAction, ETriggerEvent::Completed, this, &ASimpleGameCharacter::BlockCompleted);
}

void ASimpleGameCharacter::ResetAttackParameters()
{
	this->AttackStarted = false;
	this->JumpAttackStarted = false;	
	this->SetTimer(&ASimpleGameCharacter::OnAttackCooldownTimerElapsed, ASimpleGameCharacter::AttackCooldownTimeSeconds);
	this->InAttackCooldown = true;	
}

void ASimpleGameCharacter::ResetBlockParameters()
{
	this->BlockEndBlendTracker.Reset();
	this->InBlockMode = false;
	this->BlockAnimationEnded = false;
	this->IsBlockManuallyStarted = false;
	this->SetTimer(&ASimpleGameCharacter::OnBlockCooldownTimerElapsed, ASimpleGameCharacter::BlockCooldownTimeSeconds);
	this->InBlockCooldown = true;
}

void ASimpleGameCharacter::ResetDodgeParameters()
{
	if (!this->IsBlockManuallyStarted)
	{
		this->StopBlockAnimation();
	}

	this->InDodgeMode = false;	
	this->SetTimer(&ASimpleGameCharacter::OnDodgeCooldownTimerElapsed, ASimpleGameCharacter::DodgeCooldownTimeSeconds);
	this->InDodgeCooldown = true;	
}

void ASimpleGameCharacter::OnJumpCooldownTimerElapsed()
{
	this->InJumpCooldown = false;
}

void ASimpleGameCharacter::OnAttackCooldownTimerElapsed()
{
	this->InAttackCooldown = false;
}

void ASimpleGameCharacter::OnBlockCooldownTimerElapsed()
{
	this->InBlockCooldown = false;
}

void ASimpleGameCharacter::OnDodgeCooldownTimerElapsed()
{
	this->InDodgeCooldown = false;
}

void ASimpleGameCharacter::Landed(const FHitResult& Hit)
{
	this->SetTimer(&ASimpleGameCharacter::OnJumpCooldownTimerElapsed, ASimpleGameCharacter::JumpCooldownTimeSeconds);
	this->InJumpCooldown = true;
	
	// Need to check for Dodge cooldown to ensure that Dodge parameters don't get reset each time we land
	if (this->IsInDodgeMode() && !this->InDodgeCooldown)
	{
		this->ResetDodgeParameters();
	}
}

bool ASimpleGameCharacter::IsAttacking() const
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
	if (this->IsAttackStarted() || this->IsAttacking() || this->IsInDodgeMode())
	{
		return;
	}

	// Extract 2D axis data (x and y) 	
	FVector2D MovementVector =  Value.Get<FVector2D>();
		
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
		
	if (!this->IsBlockCameraSet)
	{
		this->AddControllerPitchInput(TurnVector.Y);
	}
}

void ASimpleGameCharacter::BottomGamepadStarted()
{
	if (this->GetCharacterMovement()->IsFalling() || this->InJumpCooldown || this->IsAttackStarted() || this->IsInDodgeMode() || this->InDodgeCooldown || this->IsInBlockMode())
	{
		return;
	}
	
	this->Jump();
}

void ASimpleGameCharacter::BottomGamepadCompleted()
{
	this->StopJumping();
}

void ASimpleGameCharacter::LeftGamepadStarted()
{
	// To prevent the character animations from looking stuttery or jumpy, we avoid going into the attack state if the character
	// is both moving AND trying to attack within the attack cooldown period (i.e. rapidly pressing the attack button). We check for 
	// both movement and the cooldown because moving while mashing the attack button confuses the state machine, so we want to be
	// extra sure that we guard against the jittery/jumpy animations
	bool movingAttackInCooldown = this->InAttackCooldown && this->IsPlayerMovementInputEnabled();
	if (this->IsInBlockMode() || movingAttackInCooldown)
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

void ASimpleGameCharacter::TopGamepadStarted()
{
}
	
void ASimpleGameCharacter::RightGamepadStarted()
{
	this->CheckForDodge(DodgeDirection::Back);
}

void ASimpleGameCharacter::BlockStarted()
{
	this->IsBlockManuallyStarted = true;
	this->IsRotatingForBlock = true;
	this->StartBlockAnimation();
}

void ASimpleGameCharacter::BlockCompleted()
{
	this->StopBlockAnimation();
}

void ASimpleGameCharacter::StartBlockAnimation()
{
	if (this->InBlockCooldown)
	{
		return;
	}

	this->InBlockMode = true;
}

void ASimpleGameCharacter::StopBlockAnimation()
{
	// Reset no matter what part of the Block state we're in
	this->GetCharacterMovement()->bOrientRotationToMovement = true;
	this->bUseControllerRotationYaw = false;	
	this->IsBlockCameraSet = false;
	this->IsRotatingForBlock = false;

	if (this->InBlockCooldown)
	{
		return;
	}

	this->BlockAnimationEnded = true;
	this->InBlockMode = false;
	
	// Set the tracker to count down to 0 to determine when the transition out of Block is complete
	this->BlockEndBlendTracker.SetValueRange(this->BlockEndBlendTracker.GetBlendedValue(), 0.0f);
}

void ASimpleGameCharacter::CheckForDodge(DodgeDirection dodgeDirection)
{
	bool isMovementInBlock = this->IsPlayerMovementInputEnabled() && this->IsInBlockMode();
	if (this->GetCharacterMovement()->IsFalling() || this->IsAttacking() || isMovementInBlock)
	{
		return;
	}

	if (!this->InDodgeCooldown)
	{			
		this->StartBlockAnimation();
	
		this->SmoothedMovementVector =
			dodgeDirection == DodgeDirection::Back ? FVector2D(0, -1) :
			dodgeDirection == DodgeDirection::Left ? FVector2D(-1, 0) :
			dodgeDirection == DodgeDirection::Front ? FVector2D(0, 1) :
			dodgeDirection == DodgeDirection::Right ? FVector2D(1, 0) :
			FVector2D(0, 0);		
		
		// Get dodge vector in local space, then apply rotation quaternion to transform it into world space
		FVector LocalForwardVector = this->GetActorForwardVector();
		LocalForwardVector.Normalize();
		
		FVector DodgeVector = -LocalForwardVector;
		double DodgeFactor = ASimpleGameCharacter::DodgeMoveFactor + this->GetVelocity().Size2D();
		DodgeVector.X *= DodgeFactor;
		DodgeVector.Y *= DodgeFactor;
		DodgeVector.Z = ASimpleGameCharacter::DodgeJumpFactor;
		
		this->LaunchCharacter(DodgeVector, false, false);
		this->InDodgeMode = true;
	}
}