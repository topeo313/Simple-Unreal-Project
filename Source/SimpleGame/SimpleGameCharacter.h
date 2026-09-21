// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Camera/CameraComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputActionValue.h"
#include "SimpleGameCharacter.generated.h"

// Forward declarations
class USpringArmComponent;
class UInputMappingContext;
class UInputAction;
class UAnimMontage;

UCLASS()
class SIMPLEGAME_API ASimpleGameCharacter : public ACharacter
{
	GENERATED_BODY()

// Constants
private:
	static constexpr auto MoveThreshold = 0.78f; // Ensures that left joystick movement only occurs above a certain threshold
	static constexpr auto MoveInterpolationSpeed = 15.0f;
	
	static constexpr auto JumpCooldownTimeSeconds = 0.175f;
	static constexpr auto AttackCooldownTimeSeconds = 0.175f;
	static constexpr auto BlockCooldownTimeSeconds = 0.225f;
	static constexpr auto DodgeCooldownTimeSeconds = 0.750f;
	
	static constexpr auto DodgeMoveFactor = 500;
	static constexpr auto DodgeJumpFactor = 150;

// Initialization
public:
	// Sets default values for this character's properties
	ASimpleGameCharacter();

// Operations
protected:
	void SetTimer(
		TDelegate<void(), FDefaultTSDelegateUserPolicy>::TMethodPtr<ASimpleGameCharacter> TimerDelegate,
		float DurationSeconds);

public:	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	void ResetAttackParameters();	
	void ResetBlockParameters();
	void ResetDodgeParameters();
	
private:
	void OnJumpCooldownTimerElapsed();	
	void OnAttackCooldownTimerElapsed();
	void OnBlockCooldownTimerElapsed();
	void OnDodgeCooldownTimerElapsed();
	
// Properties
public:
	FORCEINLINE USpringArmComponent* GetCameraArm() const { return this->CameraArm;}	
	FORCEINLINE UAnimInstance* GetAnimInstance() const { return this->GetMesh()->GetAnimInstance(); }
	
	virtual void Jump() override;	
	virtual void Landed(const FHitResult& Hit) override;
	
	FVector2D GetCurrentInputMovementVector() const { return this->SmoothedMovementVector; };
	bool IsPlayerMovementInputEnabled() const { return this->GetCharacterMovement()->GetCurrentAcceleration().SizeSquared() > KINDA_SMALL_NUMBER; }
	bool IsAttackStarted() const { return this->AttackStarted; }
	bool IsJumpAttackStarted() const { return this->JumpAttackStarted; }
	bool IsAttacking() const;
	
	bool IsInBlockMode() const { return this->InBlockMode; }
	bool IsBlockAnimationEnded() const { return this->BlockAnimationEnded; }
	
	bool IsInDodgeMode() const { return this->InDodgeMode; }
	
protected:
	// Handles movement in all directions
	void Move(const FInputActionValue& Value);
	
	// Rotate based on mouse X movement or right gamepad X movement
	// Rotate based on mouse Y movement or right gamepad Y movement
	void LookAround(const FInputActionValue& Value);
	
	void LeftGamepadStarted();
	
	void BlockStarted();
	void BlockCompleted();
	
// Members
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> PlayerMappingContext = nullptr; 
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction = nullptr;
	
	// Action to turn (yaw) and look up/down
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAroundAction = nullptr;
	
	// Action to turn (yaw) and look up/down
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction = nullptr;
	
	// Execute actions associated with the Left Gamepad button
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LeftGamepadAction = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (AllowPrivateAccess = true))
	float AttackA_PlayRate = 0.8f;
	
	// Action to go into Block mode
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> BlockAction = nullptr;	
			
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = true))
	TObjectPtr<USpringArmComponent> CameraArm = nullptr;		
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = true))
	TObjectPtr<UCameraComponent> Camera = nullptr;
	
	const FAnimNode_StateMachine* GroundMovementStateMachine = nullptr;
	
	const FAnimNode_StateMachine* AirMovementStateMachine = nullptr;
	
	bool InJumpCooldown;
			
	bool AttackStarted;
	
	bool InAttackCooldown;
	
	bool JumpAttackStarted;
	
	bool IsBlockCameraSet;
	
	bool InBlockMode;
	
	bool BlockAnimationEnded;
	
	bool InBlockCooldown;
	
	// Use to track the moment that the Character fully comes out of the Block blend animation
	FAlphaBlend BlockEndBlendTracker;
	
	bool InDodgeMode;
	
	bool InDodgeCooldown;
	
	FVector2D SmoothedMovementVector;
};
