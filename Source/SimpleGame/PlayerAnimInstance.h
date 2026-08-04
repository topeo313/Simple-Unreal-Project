// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerAnimInstance.generated.h"

// Forward declarations
class ASimpleGameCharacter;
class UCharacterMovementComponent;

/**
 * 
 */
UCLASS()
class SIMPLEGAME_API UPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	virtual void NativeInitializeAnimation() override;
	
	UFUNCTION(BlueprintCallable)
	void UpdateAnimationProperties(float deltaTime);
	
// Members
private:
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Player", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ASimpleGameCharacter> GameCharacter = nullptr;
	
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Player", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCharacterMovementComponent> GameCharacterMovementComponent = nullptr;
	
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Player", meta = (AllowPrivateAccess = "true"))
	float speed;
	
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Player", meta = (AllowPrivateAccess = "true"))
	bool isPlayerMovementInputEnabled;	
	
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Player", meta = (AllowPrivateAccess = "true"))
	bool isInAir;
};
