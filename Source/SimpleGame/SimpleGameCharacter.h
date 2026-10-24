// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Camera/CameraComponent.h"
#include "CoreMinimal.h"
#include "DodgeDirection.h"
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
	static constexpr auto HeavyAttackCooldownTimeSeconds = 3.0f;
	static constexpr auto BlockCooldownTimeSeconds = 0.225f;
	static constexpr auto DodgeCooldownTimeSeconds = 0.5f;
	
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
	
	UFUNCTION(BlueprintCallable, meta = (Category = "Functions"))
	void ResetAttackParameters();
	
	UFUNCTION(BlueprintCallable, meta = (Category = "Functions"))
	void ResetHeavyAttackParameters();	
	
	void ResetBlockParameters();
	void ResetDodgeParameters();
	
private:
	void OnJumpCooldownTimerElapsed();	
	void OnAttackCooldownTimerElapsed();
	void OnHeavyAttackCooldownTimerElapsed();
	void OnBlockCooldownTimerElapsed();
	void OnDodgeCooldownTimerElapsed();
	
// Properties
public:
	FORCEINLINE USpringArmComponent* GetCameraArm() const { return this->CameraArm;}	
	FORCEINLINE UAnimInstance* GetAnimInstance() const { return this->GetMesh()->GetAnimInstance(); }
	
	virtual void Landed(const FHitResult& Hit) override;
	
	FVector2D GetCurrentInputMovementVector() const { return this->SmoothedMovementVector; };
	bool IsPlayerMovementInputEnabled() const { return this->GetCharacterMovement()->GetCurrentAcceleration().SizeSquared() > KINDA_SMALL_NUMBER; }
	
	UFUNCTION(BlueprintCallable, meta = (Category = "Functions"))
	bool IsAttackStarted() const { return this->AttackStarted; }
	
	UFUNCTION(BlueprintCallable, meta = (Category = "Functions"))
	void SetIsAttackStarted(bool IsStarted) { this->AttackStarted = IsStarted; }
	
	UFUNCTION(BlueprintCallable, meta = (Category = "Functions"))	
	bool IsHeavyAttackStarted() const { return this->HeavyAttackStarted; }
	
	UFUNCTION(BlueprintCallable, meta = (Category = "Functions"))	
	void SetIsHeavyAttackStarted(bool IsStarted) { this->HeavyAttackStarted = IsStarted; }	
	
	UFUNCTION(BlueprintCallable, meta = (Category = "Functions"))
	bool IsAttacking() const;
	
	UFUNCTION(BlueprintCallable, meta = (Category = "Functions"))	
	bool IsInAttackCooldown() const { return this->InAttackCooldown; }
	
	UFUNCTION(BlueprintCallable, meta = (Category = "Functions"))	
	bool IsInHeavyAttackCooldown() const { return this->InHeavyAttackCooldown; }	
	
	UFUNCTION(BlueprintCallable, meta = (Category = "Functions"))	
	bool IsMovingAndAttackingInCooldown() const { return this->InAttackCooldown && this->IsPlayerMovementInputEnabled(); }
	
	bool IsInRotationForBlock() const { return this->IsRotatingForBlock; }
	
	UFUNCTION(BlueprintCallable, meta = (Category = "Functions"))
	bool IsInBlockMode() const { return this->InBlockMode; }
		
	bool IsBlockAnimationEnded() const { return this->BlockAnimationEnded; }
	
	bool IsInDodgeMode() const { return this->InDodgeMode; }
	
protected:
	// Handles movement in all directions
	void Move(const FInputActionValue& Value);
	
	// Rotate based on mouse X movement or right gamepad X movement
	// Rotate based on mouse Y movement or right gamepad Y movement
	void LookAround(const FInputActionValue& Value);
	
	void BottomGamepadStarted();
	void BottomGamepadCompleted();
		
	void RightGamepadStarted();
	
	void BlockStarted();
	void BlockCompleted();
	void StartBlockAnimation();
	void StopBlockAnimation();
	
	void CheckForDodge(DodgeDirection dodgeDirection);
	
// Members
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> PlayerMappingContext = nullptr; 
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction = nullptr;
	
	// Action to turn (yaw) and look up/down
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAroundAction = nullptr;
	
	// Execute actions associated with the Bottom Gamepad button
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> BottomGamepadAction = nullptr;
	
	// Execute actions associated with the Right Gamepad button
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> RightGamepadAction = nullptr;
	
	// Action to go into Block mode
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> BlockAction = nullptr;	
			
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = true))
	TObjectPtr<USpringArmComponent> CameraArm = nullptr;		
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = true))
	TObjectPtr<UCameraComponent> Camera = nullptr;
	
	const FAnimNode_StateMachine* SM_GroundMovement = nullptr;
	
	const FAnimNode_StateMachine* SM_GroundAttackMovement = nullptr;
	
	const FAnimNode_StateMachine* SM_GroundAttackAirMovement = nullptr;
	
	bool InJumpCooldown;
			
	bool AttackStarted;
	
	bool InAttackCooldown;
	
	bool HeavyAttackStarted;
	
	bool InHeavyAttackCooldown;
	
	bool IsRotatingForBlock;
	
	bool IsBlockCameraSet;
	
	bool InBlockMode;
	
	bool BlockAnimationEnded;
	
	bool InBlockCooldown;
	
	bool IsBlockManuallyStarted;
	
	// Use to track the moment that the Character fully comes out of the Block blend animation
	FAlphaBlend BlockEndBlendTracker;
	
	bool InDodgeMode;
	
	bool InDodgeCooldown;
	
	FVector2D SmoothedMovementVector;
	
	double PrevControlYaw;
};
