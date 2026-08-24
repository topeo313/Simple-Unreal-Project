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

// Initialization
public:
	// Sets default values for this character's properties
	ASimpleGameCharacter();

// Operations
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	void SetTimer(
		TDelegate<void(), FDefaultTSDelegateUserPolicy>::TMethodPtr<ASimpleGameCharacter> TimerDelegate,
		float DurationSeconds);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	void ResetAttackParameters();	
	
private:
	void OnJumpCooldownTimerElapsed();	
	void OnAttackCooldownTimerElapsed();
	
// Properties
public:
	FORCEINLINE USpringArmComponent* GetCameraArm() const { return this->CameraArm;}	
	FORCEINLINE UAnimInstance* GetAnimInstance() const { return this->GetMesh()->GetAnimInstance(); }
	
	virtual void Jump() override;	
	virtual void Landed(const FHitResult& Hit) override;
	
	bool IsPlayerMovementInputEnabled() const { return this->GetCharacterMovement()->GetCurrentAcceleration().SizeSquared() > KINDA_SMALL_NUMBER; }
	bool IsAttackStarted() const { return this->AttackStarted; }
	bool IsJumpAttackStarted() const { return this->JumpAttackStarted; }
	bool IsAttacking();
	
protected:
	// Handles movement in all directions
	void Move(const FInputActionValue& Value);
	
	// Rotate based on mouse X movement or right gamepad X movement
	// Rotate based on mouse Y movement or right gamepad Y movement
	void LookAround(const FInputActionValue& Value);
	
	void Attack_A_Started();
	
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
	
	// Action to carry execute "Attack A" action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> AttackA_Action = nullptr;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (AllowPrivateAccess = true))
	float AttackA_PlayRate = 0.8f;
			
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
	
	FVector2D SmoothedMovementVector;
};
