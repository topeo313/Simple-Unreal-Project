// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Camera/CameraComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
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
	static constexpr auto MoveInterpolationSpeed = 14.0f;

public:
	// Sets default values for this character's properties
	ASimpleGameCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
// Properties
public:
	FORCEINLINE USpringArmComponent* GetCameraArm() const { return this->CameraArm;}	
	
protected:
	// Handles movement in all directions
	void Move(const FInputActionValue& Value);
	void MoveEnd(const FInputActionValue& Value);
	
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
	
	// Attack montage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (AllowPrivateAccess = true))
	TObjectPtr<UAnimMontage> AttackA_Montage = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (AllowPrivateAccess = true))
	float AttackA_PlayRate = 0.8f;
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = true))
	TObjectPtr<USpringArmComponent> CameraArm = nullptr;		
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = true))
	TObjectPtr<UCameraComponent> Camera = nullptr;
	
	FVector2D SmoothedMovementVector;
};
